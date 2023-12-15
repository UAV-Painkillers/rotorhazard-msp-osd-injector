#include "app.hpp"

void App::setupSerial(bool endOldSerial) {
    uint32_t fc_msp_baud = EEPROMManager::data.fc_msp_baud == 0 ? DEFAULT_FC_SERIAL_BAUD : EEPROMManager::data.fc_msp_baud;
    uint32_t logging_baud = EEPROMManager::data.logging_baud == 0 ? DEFAULT_LOGGING_BAUD : EEPROMManager::data.logging_baud;
    
    if (EEPROMManager::data.fc_serial_uses_main_serial) {
        if (endOldSerial) {
            Serial.end();
        }
        Serial.begin(fc_msp_baud);

        this->mspSerial = &Serial;
        this->loggingSerial = NULL;

        logInline("App::setupSerial done, no logging");
        return;
    }

    Serial.end();
    Serial.begin(logging_baud);

    this->loggingSerial = &Serial;

    if (this->mspSoftSerial == NULL || this->mspSoftSerial == nullptr) {
        this->mspSoftSerial = new EspSoftwareSerial::UART();
    }

    if (endOldSerial) {
        this->mspSoftSerial->end();
    }

    if (EEPROMManager::data.fc_soft_serial_tx_pin != -1 && EEPROMManager::data.fc_soft_serial_rx_pin != -1) {
        this->mspSoftSerial->begin(fc_msp_baud, EspSoftwareSerial::SWSERIAL_8N1, EEPROMManager::data.fc_soft_serial_rx_pin, EEPROMManager::data.fc_soft_serial_tx_pin);
        this->mspSerial = this->mspSoftSerial;
    }
    this->mspSerial = this->mspSoftSerial;

    logInline("App::setupSerial done");
}

void App::setup() {
    EEPROMManager::setup();
    this->setupSerial(false);

    logInline("App::setup init...");

    this->linkCallbacks();

    this->webUI.setup();
    this->rotorHazard.setup();
    if (this->mspSerial == NULL || this->mspSerial == nullptr) {
        logLine("App::setupSerial mspSerial is NULL, skipping MSP Controller setup");
    } else {
        this->mspController.setup(*this->mspSerial);
    }
    this->wifiConnection.setup();

    logInline("App::setupSerial done");
}

void App::loop() {
    this->wifiConnection.loop();
    if (this->mspSerial != NULL && this->mspSerial != nullptr) {
        this->mspController.loop();
    }
    this->rotorHazard.loop();
    this->webUI.loop();
    this->ota.loop();
}

void App::linkCallbacks() {
    this->rotorHazard.onLapFinished([this](rh_lap_finished_data lapFinishedData) {
        logLine("rotorHazard.onLapFinished in lap finished callback");

        // skip message if lap_time is NULL
        if (lapFinishedData.lap_time == NULL) {
            logLine("rotorHazard.onLapFinished lap_time is NULL, skipping message");
            return;
        }

        // build a message to send to the quad
        // "Lap 1: 1:23.456 (fastest!)"
        logLine("rotorHazard.onLapFinished building message");
        String lapMessage;
        lapMessage.concat("Lap ");
        lapMessage.concat(lapFinishedData.lap_number);
        lapMessage.concat(": ");

        lapMessage.concat(lapFinishedData.lap_time);

        this->mspController.printMessage(lapMessage, 2500);

        if (lapFinishedData.was_fastest_lap) {
            String fastestLapMessage("Fastest lap!");
            this->mspController.printMessage(fastestLapMessage, 1000);
        }
    });

    this->rotorHazard.onConnectionStatusChange([this](rh_connection_status status) {
        // build a message to send to the quad
        // "RH connected", "RH disconnected", etc.
        
        String rhMessage;
        unsigned long duration = 5000;
        switch (status) {
            case RH_CONNECTED:
                rhMessage.concat("RH connected");
                duration = 2000;
                break;
            case RH_DISCONNECTED:
                rhMessage.concat("RH disconnected");
                break;
            case RH_CONNECTION_FAILED:
                rhMessage.concat("RH connection failed");
                break;
        }

        this->mspController.printMessage(rhMessage, duration);
    });

    this->wifiConnection.onWiFiConnectionStatusChanged([this](wl_status_t status, IPAddress ip) {
        // build a message to send to the quad
        // "WiFi connected", "Wrong WiFi password", "WiFi disconnected", etc.
        
        String wifiMessage("");
        switch (status) {
            case WL_CONNECTED:
                wifiMessage.concat("WiFi connected");
                break;
            case WL_NO_SSID_AVAIL:
                wifiMessage.concat("WiFi SSID not found");
                break;
            case WL_CONNECT_FAILED:
                wifiMessage.concat("WiFi connection failed");
                break;
            case WL_CONNECTION_LOST:
                wifiMessage.concat("WiFi connection lost");
                break;
            case WL_DISCONNECTED:
                wifiMessage.concat("WiFi disconnected");
                break;
            default:
                wifiMessage.concat("WiFi unknown status");
                break;
        }

        logInline("wifiConnection.onWiFiConnectionStatusChanged sending message: ");
        logLine(wifiMessage);
        this->mspController.printMessage(wifiMessage, 2000);

        if (status == WL_CONNECTED) {
            // print ip address
            String ipString = ipToString(ip);
            this->mspController.printMessage(ipString, 5000);

            /*if (!this->rotorHazard.isEnabled()) {
                logLine("wifiConnection.onWiFiConnectionStatusChanged enabling rotorHazard");
                this->rotorHazard.resetTimeout();
                this->rotorHazard.enable();
            }

            if (!this->webUI.isEnabled() && !this->race_mode_on_boot) {
                logLine("wifiConnection.onWiFiConnectionStatusChanged enabling webUI");
                this->webUI.resetTimeout();
                this->webUI.enable();
            }*/
        } else {
            /*if (this->rotorHazard.isEnabled()) {
                logLine("wifiConnection.onWiFiConnectionStatusChanged disabling rotorHazard");
                this->rotorHazard.disable();
            }

            if (this->webUI.isEnabled()) {
                logLine("wifiConnection.onWiFiConnectionStatusChanged disabling webUI");
                this->webUI.disable();
            }*/
        }
    });

    this->wifiConnection.onHotspotEnabled([this](bool enabled, String ssid, String password, IPAddress ip) {
        // build a message to send to the quad
        // "Hotspot enabled", "Hotspot disabled", etc.
        
        if (!enabled) {
            String hotspotMessage("Hotspot disabled");
            this->mspController.printMessage(hotspotMessage, 2000);
            return;
        }

        String hotspotMessage("Hotspot enabled");
        this->mspController.printMessage(hotspotMessage, 2000);

        String ssidMessage("SSID: ");
        ssidMessage.concat(ssid);
        this->mspController.printMessage(ssidMessage, 2000);

        String passwordMessage("Pass: ");
        passwordMessage.concat(password);
        this->mspController.printMessage(passwordMessage, 2000);

        String ipMessage("IP: ");
        ipMessage.concat(ipToString(ip));
        this->mspController.printMessage(ipMessage, 2000);
    });

    this->webUI.onPinChanged([this](String pin) {
        // build a message to send to the quad
        // "Web PIN: 1234", etc.
        
        String pinMessage;
        pinMessage.concat("Web pin: ");
        pinMessage.concat(pin);

        this->mspController.printMessage(pinMessage, 7000);
    });

    this->webUI.setConectToWifiHandler([this]() {
        this->wifiConnection.connectToWiFi();
    });

    this->webUI.setSetWifiCredentialsHandler([this](String ssid, String password) {
        this->wifiConnection.setWifiCredentials(ssid, password);
    });

    this->webUI.setGetHotspotStatusHandler([this]() {
        return this->wifiConnection.isHotspotEnabled();
    });

    this->webUI.setEnableHotspothandler([this]() {
        logLine("Web ui enable hotspot handler called");
        return this->wifiConnection.enableHotspot();
    });

    this->webUI.setSetRotorhazardPilotIdHandler([this](uint32_t id) {
        this->rotorHazard.setPilotIdForLapFiltering(id);
    });

    this->webUI.setGetRotorhazardPilotIdHandler([this]() {
        return this->rotorHazard.getPilotIdForLapFiltering();
    });

    this->webUI.setGetStoredWifiSSIDHandler([this]() {
        return this->wifiConnection.getWifiSSID();
    });

    this->webUI.setSetRotorhazardConnectionDataHandler([this](String hostname, int port, String password) {
        this->rotorHazard.storeConnectionData(hostname, port, password);
    });

    this->webUI.setGetRotorhazardConnectionDataHandler([this]() {
        return this->rotorHazard.getConnectionData();
    });

    this->webUI.setGetHotspotCredentialsHandler([this]() {
        return this->wifiConnection.getHotspotCredentials();
    });

    this->webUI.setSetHotspotCredentialsHandler([this](String ssid, String password) {
        this->wifiConnection.storeHotspotCredentials(ssid, password);
    });

    this->webUI.setGetFlightControllerBaudRateHandler([this]() {
        return this->getMspBaudrate();
    });

    this->webUI.setSetFlightControllerBaudRateHandler([this](int baudRate) {
        logLine("webUI.setSetFlightControllerBaudRateHandler called");

        this->setMspBaudrate(baudRate);

        logLine("webUI.setSetFlightControllerBaudRateHandler done");
    });

    this->webUI.setGetLoggingBaudRateHandler([this]() {
        return this->getLoggingBaudrate();
    });

    this->webUI.setSetLoggingBaudRateHandler([this](int baudRate) {
        logLine("webUI.setSetLoggingBaudRateHandler called");

        this->setLoggingBaudrate(baudRate);

        logLine("webUI.setSetLoggingBaudRateHandler done");
    });

    this->webUI.setGetOtaEnabledHandler([this]() {
        return this->ota.isEnabled();
    });

    this->webUI.setEnableOtaHandler([this]() {
        this->ota.enable();
    });

    this->webUI.setDisableOtaHandler([this]() {
        this->ota.disable();
    });

    this->webUI.setGetFcSerialUsesMainSerialHandler([this]() {
        return EEPROMManager::data.fc_serial_uses_main_serial;
    });

    this->webUI.setToggleFcSerialUsesMainSerialHandler([this]() {
        EEPROMManager::data.fc_serial_uses_main_serial = !EEPROMManager::data.fc_serial_uses_main_serial;
        EEPROMManager::writeEEPROM();
        return EEPROMManager::data.fc_serial_uses_main_serial;
    });
}

uint32_t App::getMspBaudrate() {
    return EEPROMManager::data.fc_msp_baud == 0 ? DEFAULT_FC_SERIAL_BAUD : EEPROMManager::data.fc_msp_baud;
}

uint32_t App::getLoggingBaudrate() {
    return EEPROMManager::data.logging_baud == 0 ? DEFAULT_LOGGING_BAUD : EEPROMManager::data.logging_baud;
}

void App::setMspBaudrate(uint32_t baudrate) {
    EEPROMManager::data.fc_msp_baud = baudrate;
    EEPROMManager::writeEEPROM();

    this->setupSerial();
}

void App::setLoggingBaudrate(uint32_t baudrate) {
    EEPROMManager::data.logging_baud = baudrate;
    EEPROMManager::writeEEPROM();

    this->setupSerial();
}
