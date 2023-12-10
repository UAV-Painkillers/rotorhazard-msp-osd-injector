#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <functional>
#include <DNSServer.h>

#include "logging.hpp"
#include "const.h"
#include "eeprom.hpp"
#include "utils.hpp"

#define DNS_PORT 53

// callback type for wifi connection status changed
typedef std::function<void(wl_status_t, IPAddress)> WiFiConnectionStatusChangedCallback;

// callback for hotspot enabled or disabled
typedef std::function<void(bool, String, String, IPAddress)> WiFiHotspotEnabledCallback;

struct network_credentials_t {
    String ssid;
    String password;
};

class WiFiConnection {
    public:
        void setup();
        void loop();
        void onWiFiConnectionStatusChanged(WiFiConnectionStatusChangedCallback callback);
        void onHotspotEnabled(WiFiHotspotEnabledCallback callback);
        bool isHotspotEnabled();
        String getWifiSSID();
        void connectToWiFi(String ssid = "", String password = "", bool saveToEEPROM = false);
        void setWifiCredentials(String ssid, String password);
        bool enableHotspot();
        void storeHotspotCredentials(String ssid, String password);
        network_credentials_t getHotspotCredentials();

    private:
        wl_status_t currentStatus;
        WiFiConnectionStatusChangedCallback onWiFiConnectionStatusChangedCallback;
        WiFiHotspotEnabledCallback onHotspotEnabledCallback;

        String getHostname();
        String getWifiPassword();
        void checkWiFiConnectionAndFallbackToHotspot();

        IPAddress ipAddress;
        DNSServer dnsServer;
        unsigned long maxConnectionAttemptDurationMs = 0;
        unsigned long connectionCheckStartMs = 0;
        bool hotspotEnabled = false;

        static IPAddress wifi_hotspot_local_IP;
        static IPAddress wifi_hotspot_gateway;
        static IPAddress wifi_hotspot_subnet;
};
