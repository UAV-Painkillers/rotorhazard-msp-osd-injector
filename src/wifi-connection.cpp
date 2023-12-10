#include "wifi-connection.hpp"

const unsigned long WIFI_CALLBACK_INTERVAL_MS = 25;

IPAddress WiFiConnection::wifi_hotspot_local_IP = IPAddress(10, 0, 0, 1);
IPAddress WiFiConnection::wifi_hotspot_gateway = IPAddress(10, 0, 0, 1);
IPAddress WiFiConnection::wifi_hotspot_subnet = IPAddress(255, 255, 255, 0);

String WiFiConnection::getHostname()
{
    if (EEPROMManager::data.hostname[0] != '\0')
    {
        return String(EEPROMManager::data.hostname);
    }

    return String(DEFAULT_HOSTNAME);
}

void WiFiConnection::setup()
{
    logLine("WiFiConnection::setup");

    this->currentStatus = WL_IDLE_STATUS;

    this->maxConnectionAttemptDurationMs = WIFI_CONNECTION_ATTEMPT_DURATION_S * 1000;

    WiFi.hostname(this->getHostname());

    String eepromSSID = this->getWifiSSID();

    bool alreadyConnecting = false;
    if (!eepromSSID.isEmpty())
    {
        logLine("WiFiConnection::setup connecting to wifi from eeprom");
        alreadyConnecting = true;
        this->connectToWiFi();
    }

#ifdef WIFI_SSID
#ifdef WIFI_PASSWORD
    if (!alreadyConnecting)
    {
        logLine("WiFiConnection::setup connecting to wifi from secrets.h");
        alreadyConnecting = true;
        this->connectToWiFi(WIFI_SSID, WIFI_PASSWORD);
    }
#endif
#endif

    if (!alreadyConnecting)
    {
        logLine("WiFiConnection::setup no wifi credentials found, enabling hotspot");
        this->enableHotspot();
    }

    logLine("WiFiConnection::setup done");
}

void WiFiConnection::loop()
{
    EVERY_N_MILLISECONDS(WIFI_CALLBACK_INTERVAL_MS)
    {
        this->checkWiFiConnectionAndFallbackToHotspot();
    }
    this->dnsServer.processNextRequest();
}

void WiFiConnection::checkWiFiConnectionAndFallbackToHotspot()
{
    if (this->hotspotEnabled)
    {
        return;
    }

    wl_status_t newStatus = WiFi.status();

    unsigned long now = millis();
    bool maxConnectionTimeReached = this->connectionCheckStartMs + this->maxConnectionAttemptDurationMs < now;
    if ((newStatus != WL_CONNECTED) && maxConnectionTimeReached)
    {
        logLine("WiFiConnection::checkWiFiConnectionAndFallbackToHotspot max connection time reached, enabling hotspot");
        logLine(this->maxConnectionAttemptDurationMs);
        this->enableHotspot();
        return;
    }

    if (newStatus == this->currentStatus)
    {
        return;
    }
    this->currentStatus = newStatus;

    if (this->onWiFiConnectionStatusChangedCallback != NULL && this->onWiFiConnectionStatusChangedCallback != nullptr)
    {
        this->onWiFiConnectionStatusChangedCallback(newStatus, WiFi.localIP());
    }

    switch (newStatus)
    {
    case WL_CONNECTED:
        logLine("WiFiConnection::checkWiFiConnectionAndFallbackToHotspot wifi status: CONNECTED");
        return;
    case WL_NO_SSID_AVAIL:
        logLine("WiFiConnection::checkWiFiConnectionAndFallbackToHotspot wifi status: NO SSID AVAILABLE");
        break;
    case WL_CONNECT_FAILED:
        logLine("WiFiConnection::checkWiFiConnectionAndFallbackToHotspot wifi status: CONNECTION FAILED");
        break;
    case WL_IDLE_STATUS:
        logLine("WiFiConnection::checkWiFiConnectionAndFallbackToHotspot wifi status: IDLE");
        break;
    case WL_DISCONNECTED:
        logLine("WiFiConnection::checkWiFiConnectionAndFallbackToHotspot wifi status: DISCONNECTED");
        break;
    default:
        logLine("WiFiConnection::checkWiFiConnectionAndFallbackToHotspot wifi status: UNKNOWN");
        break;
    }
}

void WiFiConnection::onWiFiConnectionStatusChanged(WiFiConnectionStatusChangedCallback callback)
{
    this->onWiFiConnectionStatusChangedCallback = callback;
}

void WiFiConnection::setWifiCredentials(String ssid, String password)
{
    logLine("WiFiConnection::setWifiCredentials started");

    logLine("WiFiConnection::setWifiCredentials writing credentials to EEPROM");
    // copy ssid and password into eeprom manager data
    ssid.trim();
    ssid.toCharArray(EEPROMManager::data.wifi_ssid, ssid.length() + 1);

    password.trim();
    password.toCharArray(EEPROMManager::data.wifi_password, password.length() + 1);
    EEPROMManager::writeEEPROM();

    logLine("WiFiConnection::setWifiCredentials finished");
}

void WiFiConnection::connectToWiFi(String ssid, String password, bool saveToEEPROM)
{
    logLine("WiFiConnection::connectToWiFi started");

    if (saveToEEPROM)
    {
        this->setWifiCredentials(ssid, password);
    }

    String ssidToUse = this->getWifiSSID();;
    String passwordToUse = this->getWifiPassword();;

    if (ssidToUse.equals("") || ssidToUse.isEmpty())
    {
        logLine("WiFiConnection::connectToWiFi no ssid to connect to set, starting hotspot");
        this->hotspotEnabled = false;
        this->enableHotspot();
        return;
    }

    logLine("WiFiConnection::connectToWiFi stopping dns");
    this->dnsServer.stop();

    logLine("WiFiConnection::connectToWiFi disconnecting from AP");
    WiFi.softAPdisconnect();

    logLine("WiFiConnection::connectToWiFi set wifi mode to WIFI_STA");
    WiFi.mode(WIFI_STA);

    logLine("WiFiConnection::connectToWiFi disconnecting from current WiFi");
    WiFi.disconnect();

    logLine("WiFiConnection::connectToWiFi connecting to WiFi");
    logInline("SSID: ");
    logLine(ssidToUse);

    WiFi.setAutoReconnect(true);
    this->connectionCheckStartMs = millis();
    WiFi.begin(ssidToUse.c_str(), passwordToUse.c_str());

    this->currentStatus = WL_IDLE_STATUS;
    this->hotspotEnabled = false;

    logLine("WiFiConnection::connectToWiFi finished");
}

bool WiFiConnection::enableHotspot()
{
    if (this->hotspotEnabled)
    {
        return true;
    }

    logLine("WiFiConnection::enableHotspot begin");

    WiFi.disconnect();
    WiFi.softAPdisconnect();

    WiFi.mode(WIFI_AP);
    logLine("WiFiConnection::enableHotspot setting up hotspot");

    WiFi.softAPConfig(this->wifi_hotspot_local_IP, this->wifi_hotspot_gateway, this->wifi_hotspot_subnet);

    network_credentials_t credentials = this->getHotspotCredentials();
    bool success = WiFi.softAP(credentials.ssid.c_str(), credentials.password.c_str());

    // retry 5 times with a random suffix if the hotspot could not be enabled
    if (!success)
    {
        logLine("WiFiConnection::enableHotspot failed, retrying with random 4 digit suffix");
        for (int i = 0; i < 5; i++)
        {
            String randomSuffix = String(random(1000, 9999));
            String ssidWithSuffix = credentials.ssid + "-" + randomSuffix;
            success = WiFi.softAP(ssidWithSuffix.c_str(), credentials.password.c_str());

            if (success)
            {
                credentials.ssid = ssidWithSuffix;
                break;
            }
        }
    }

    if (!success)
    {
        logLine("WiFiConnection::enableHotspot failed!");
        return false;
    }

    logInline("SSID: ");
    logLine(credentials.ssid);
    logInline("Password: ");
    logLine(credentials.password);

    IPAddress softAPIP = WiFi.softAPIP();

    this->dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    this->dnsServer.start(DNS_PORT, "*", softAPIP);

    if (this->onHotspotEnabledCallback != NULL && this->onHotspotEnabledCallback != nullptr)
    {
        logLine("WiFiConnection::enableHotspot calling onHotspotEnabledCallback");
        this->onHotspotEnabledCallback(true, credentials.ssid, credentials.password, softAPIP);
    }

    this->hotspotEnabled = true;

    logLine("WiFiConnection::enableHotspot done");

    return true;
}

void WiFiConnection::onHotspotEnabled(WiFiHotspotEnabledCallback callback)
{
    this->onHotspotEnabledCallback = callback;
}

bool WiFiConnection::isHotspotEnabled()
{
    return this->hotspotEnabled;
}

String WiFiConnection::getWifiSSID()
{
    if (EEPROMManager::data.wifi_ssid[0] != '\0')
    {
        return String(EEPROMManager::data.wifi_ssid);
    }

#ifdef WIFI_SSID
    return String(WIFI_SSID);
#endif

    return "";
}

String WiFiConnection::getWifiPassword()
{
    if (EEPROMManager::data.wifi_password[0] != '\0')
    {
        return String(EEPROMManager::data.wifi_password);
    }

#ifdef WIFI_PASSWORD
    return String(WIFI_PASSWORD);
#endif

    return "";
}

void WiFiConnection::storeHotspotCredentials(String ssid, String password)
{
    logLine("WiFiConnection::storeHotspotCredentials started");

    logLine("WiFiConnection::storeHotspotCredentials writing credentials to EEPROM");
    // copy ssid and password into eeprom manager data
    ssid.trim();
    ssid.toCharArray(EEPROMManager::data.hotspot_ssid, ssid.length() + 1);

    password.trim();
    password.toCharArray(EEPROMManager::data.hotspot_password, password.length() + 1);

    EEPROMManager::writeEEPROM();

    logLine("WiFiConnection::storeHotspotCredentials finished");
}

network_credentials_t WiFiConnection::getHotspotCredentials()
{
    network_credentials_t credentials;
    credentials.ssid = String(EEPROMManager::data.hotspot_ssid);

    if (credentials.ssid.isEmpty() || credentials.ssid.equals("0"))
    {
        credentials.ssid = DEFAULT_HOTSPOT_SSID;
    }

    credentials.password = String(EEPROMManager::data.hotspot_password);

    if (credentials.password.isEmpty() || credentials.password.equals("0"))
    {
        credentials.password = DEFAULT_HOTSPOT_PASSWORD;
    }

    return credentials;
}
