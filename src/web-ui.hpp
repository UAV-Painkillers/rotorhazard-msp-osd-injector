#pragma once

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "logging.hpp"
#include "rotorhazard.hpp"
#include "wifi-connection.hpp"

enum firmware_upload_mode_t {
    FIRMWARE_UPLOAD_MODE_SKETCH = 0,
    FIRMWARE_UPLOAD_MODE_FILESYSTEM = 1
};

enum firmware_upload_failed_reason_t {
    FIRMWARE_UPLOAD_FAILED_REASON_FIRMWARE_FILE_NAME = 0,
    FIRMWARE_UPLOAD_FAILED_REASON_FILESYSTEM_FILE_NAME = 1,
    FIRMWARE_UPLOAD_FAILED_REASON_AUTHENTICATION = 2,
};

// onPinChange callback definition
typedef std::function<void(String)> pin_changed_callback_t;

// handler to be called with ssid and password when user wants to connect to a wifi network, return bool for success
typedef std::function<void(String, String)> set_wifi_credentials_handler_t;

// handler that connects to wifi
typedef std::function<void()> connect_to_wifi_handler_t;

// handler that returns hotspot status (bool)
typedef std::function<bool()> get_hotspot_status_handler_t;

// handler that returns stored ssid or null
typedef std::function<String()> get_stored_wifi_ssid_handler_t;

// handler that enables hotspot
typedef std::function<bool()> enable_hotspot_handler_t;

// handler that sets my rotorhazard pilot id
typedef std::function<void(int)> set_rotorhazard_pilot_id_handler_t;

// handler that returns rotorhazard pilot id
typedef std::function<int()> get_rotorhazard_pilot_id_handler_t;

// handler called for setting rotorhazard connection data
typedef std::function<void(String, int, String)> set_rotorhazard_connection_data_handler_t;

// handler for getting rotorhazard connection data
typedef std::function<rh_connection_data_t()> get_rotorhazard_connection_data_handler_t;

// handler to get current baud rate for communicating with the flight controller
typedef std::function<int()> get_baud_rate_handler_t;

// handler to set current baud rate for communicating with the flight controller
typedef std::function<void(int)> set_baud_rate_handler_t;

// handler to set hotspot ssid and password
typedef std::function<void(String, String)> set_hotspot_credentials_handler_t;

// handler to get hotspot ssid and password
typedef std::function<network_credentials_t()> get_hotspot_credentials_handler_t;

// handler to be called i user wants to enable ota
typedef std::function<void()> enable_ota_handler_t;

// handler to be called i user wants to disable ota
typedef std::function<void()> disable_ota_handler_t;

// handler that returns if ota is enabled
typedef std::function<bool()> get_ota_enabled_handler_t;

class WebUI {
    public:
        void setup();
        void loop();

        void onPinChanged(pin_changed_callback_t callback);

        void setConectToWifiHandler(connect_to_wifi_handler_t handler);
        void setSetWifiCredentialsHandler(set_wifi_credentials_handler_t handler);
        
        void setGetStoredWifiSSIDHandler(get_stored_wifi_ssid_handler_t handler);

        void setGetHotspotStatusHandler(get_hotspot_status_handler_t callback);
        void setEnableHotspothandler(enable_hotspot_handler_t handler);
        void setSetHotspotCredentialsHandler(set_hotspot_credentials_handler_t handler);
        void setGetHotspotCredentialsHandler(get_hotspot_credentials_handler_t handler);

        void setSetRotorhazardPilotIdHandler(set_rotorhazard_pilot_id_handler_t handler);
        void setGetRotorhazardPilotIdHandler(get_rotorhazard_pilot_id_handler_t handler);
        void setSetRotorhazardConnectionDataHandler(set_rotorhazard_connection_data_handler_t handler);
        void setGetRotorhazardConnectionDataHandler(get_rotorhazard_connection_data_handler_t handler);

        void setGetFlightControllerBaudRateHandler(get_baud_rate_handler_t handler);
        void setSetFlightControllerBaudRateHandler(set_baud_rate_handler_t handler);

        void setGetLoggingBaudRateHandler(get_baud_rate_handler_t handler);
        void setSetLoggingBaudRateHandler(set_baud_rate_handler_t handler);

        void setEnableOtaHandler(enable_ota_handler_t handler);
        void setDisableOtaHandler(disable_ota_handler_t handler);
        void setGetOtaEnabledHandler(get_ota_enabled_handler_t handler);

    private:
        ESP8266WebServer server;
        uint8_t pin[4];

        bool handleFileRead(String uri);
        void generateNewPin();
        bool requireAuthentication();
        bool isAuthenticated();
        void setupRoutes();
        void setDefaults();
        String getPinAsString();
        String getDefaultIndexHTML();

        bool uploadFailed;
        firmware_upload_failed_reason_t uploadFailedReason;
        firmware_upload_mode_t uploadMode;

        pin_changed_callback_t pinChangedCallback;

        set_wifi_credentials_handler_t setWifiCredentials;
        connect_to_wifi_handler_t connectToWifi;
        get_stored_wifi_ssid_handler_t getStoredWifiSSID;

        get_hotspot_status_handler_t getHotspotStatus;
        enable_hotspot_handler_t enableHotspot;
        set_hotspot_credentials_handler_t setHotspotCredentials;
        get_hotspot_credentials_handler_t getHotspotCredentials;

        set_rotorhazard_pilot_id_handler_t setRotorhazardPilotId;
        get_rotorhazard_pilot_id_handler_t getRotorhazardPilotId;
        set_rotorhazard_connection_data_handler_t setRotorhazardConnectionData;
        get_rotorhazard_connection_data_handler_t getRotorhazardConnectionData;

        get_baud_rate_handler_t getFlightControllerBaudRate;
        set_baud_rate_handler_t setFlightControllerBaudRate;

        get_baud_rate_handler_t getLoggingBaudRate;
        set_baud_rate_handler_t setLoggingBaudRate;

        enable_ota_handler_t enableOta;
        disable_ota_handler_t disableOta;
        get_ota_enabled_handler_t getOtaEnabled;
};
