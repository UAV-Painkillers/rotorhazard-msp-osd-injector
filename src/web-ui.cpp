#include "web-ui.hpp"

void WebUI::setDefaults()
{
    // cors
    this->server.sendHeader("Access-Control-Allow-Origin", "*");
    this->server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT");
    this->server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept, Authorization");

    // cache control
    this->server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    this->server.sendHeader("Pragma", "no-cache");
}

void WebUI::setup()
{
    logLine("WebUi::setup started");

    if (!LittleFS.begin())
    {
        logLine("WebUi::setup An Error has occurred while mounting LittleFS");
        return;
    }

    this->setupRoutes();

    this->server.begin();
}

void WebUI::setupRoutes()
{
    logLine("WebUi::setupRoutes started");

    this->server.on("/api/pin", HTTP_GET, [this]()
                    {
        logLine("WebUi::server POST /api/pin requested");
        this->setDefaults();
        this->generateNewPin();
        this->server.send(200, "application/json", "{\"success\": true}");
        logLine("WebUi::server POST /api/pin finished"); });

    this->server.on("/api/pin/verify", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/pin/verify requested");
        
        this->setDefaults();

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, this->server.arg("plain"));

        if (error)
        {
            logLine("WebUi::server POST /api/pin/verify deserializeJson() failed:");
            logLine(error.c_str());

            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/pin/verify finished");
            return;
        }

        if (!doc.containsKey("pin")) {
            logLine("WebUi::server POST /api/pin/verify no pin in payload");
            this->server.send(403, "application/json", "{\"error\": \"no pin in payload\"}");
            logLine("WebUi::server POST /api/pin/verify finished");
            return;
        }

        String pin = doc["pin"].as<String>();
        if (pin.length() != 4) {
            logLine("WebUi::server POST /api/pin/verify pin length is not 4");
            this->server.send(403, "application/json", "{\"error\": \"pin length is not 4\"}");
            logLine("WebUi::server POST /api/pin/verify finished");
            return;
        }

        if (!pin.equals(this->getPinAsString())) {
            logLine("WebUi::server POST /api/pin/verify pin is incorrect");
            this->server.send(403, "application/json", "{\"error\": \"pin is incorrect\"}");
            logLine("WebUi::server POST /api/pin/verify finished");
            return;
        }

        logLine("WebUi::server POST /api/pin/verify pin is correct");
        this->server.send(200, "application/json", "{\"success\": true}");
        logLine("WebUi::server POST /api/pin/verify finished"); });

    this->server.on("/api/wifi/credentials", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/wifi/credentials requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, this->server.arg("plain"));

        if (error)
        {
            logLine("WebUi::server POST /api/wifi/credentials deserializeJson() failed:");
            logLine(error.c_str());

            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/wifi/credentials finished");
            return;
        }

        if (!doc.containsKey("ssid")) {
            logLine("WebUi::server POST /api/wifi/credentials no ssid in payload");
            this->server.send(400, "application/json", "{\"error\": \"no ssid in payload\"}");
            logLine("WebUi::server POST /api/wifi/credentials finished");
            return;
        }

        if (!doc.containsKey("password")) {
            logLine("WebUi::server POST /api/wifi/credentials no ssid in payload");
            this->server.send(400, "application/json", "{\"error\": \"no ssid in payload\"}");
            logLine("WebUi::server POST /api/wifi/credentials finished");
            return;
        }

        String ssid = doc["ssid"].as<String>();
        String password = doc["password"].as<String>();

        if (this->setWifiCredentials == nullptr || this->setWifiCredentials == NULL) {
            logLine("WebUi::server POST /api/wifi/credentials no setWifiCredentials handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/wifi/credentials finished");
            return;
        }

        this->setWifiCredentials(ssid, password);

        this->server.send(200, "application/json", "{\"success\": true}");

        logLine("WebUi::server POST /api/wifi/credentials finished"); });

    this->server.on("/api/wifi/actions/connect", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/wifi/actions/connect requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        if (this->connectToWifi == NULL || this->connectToWifi == nullptr) {
            logLine("WebUi::server POST /api/wifi/actions/connect no connectToWifi handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/wifi/actions/connect finished");
            return;
        }

        this->server.send(200, "application/json", "{\"success\": true}");

        this->connectToWifi();

        logLine("WebUi::server POST /api/wifi/actions/connect finished"); });

    this->server.on("/api/wifi/actions/hotspot", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/wifi/actions/hotspot requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        if (this->enableHotspot == NULL || this->enableHotspot == nullptr) {
            logLine("WebUi::server POST /api/wifi/actions/hotspot no enableHotspot handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/wifi/actions/hotspot finished");
            return;
        }

        logLine("WebUi::server POST /api/wifi/actions/hotspot calling enableHotspot");
        bool success = this->enableHotspot();
        if (success) {
            this->server.send(200, "application/json", "{\"success\": true}");
        } else {
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
        }

        logLine("WebUi::server POST /api/wifi/actions/hotspot finished"); });

    this->server.on("/api/wifi/hotspot/credentials", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/wifi/hotspot/credentials requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, this->server.arg("plain"));

        if (error) {
            logLine("WebUi::server POST /api/wifi/hotspot/credentials deserializeJson() failed:");
            logLine(error.c_str());

            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/wifi/hotspot/credentials finished");
            return;
        }

        if (!doc.containsKey("ssid")) {
            logLine("WebUi::server POST /api/wifi/hotspot/credentials no ssid in payload");
            this->server.send(400, "application/json", "{\"error\": \"no ssid in payload\"}");
            logLine("WebUi::server POST /api/wifi/hotspot/credentials finished");
            return;
        }

        if (!doc.containsKey("password")) {
            logLine("WebUi::server POST /api/wifi/hotspot/credentials no password in payload");
            this->server.send(400, "application/json", "{\"error\": \"no password in payload\"}");
            logLine("WebUi::server POST /api/wifi/hotspot/credentials finished");
            return;
        }

        String ssid = doc["ssid"].as<String>();
        String password = doc["password"].as<String>();

        if (this->setHotspotCredentials == NULL || this->setHotspotCredentials == nullptr) {
            logLine("WebUi::server POST /api/wifi/hotspot/credentials no setHotspotCredentials handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/wifi/hotspot/credentials finished");
            return;
        }

        this->setHotspotCredentials(ssid, password);

        this->server.send(200, "application/json", "{\"success\": true}");

        logLine("WebUi::server POST /api/wifi/hotspot/credentials finished"); });

    this->server.on("/api/system/actions/reboot", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/actions/reboot requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        logLine("rebooting now...");
        ESP.restart(); });

    this->server.on("/api/rotorhazard/pilot_id", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/rotorhazard/pilot_id requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, this->server.arg("plain"));

        if (error) {
            logLine("WebUi::server POST /api/rotorhazard/pilot_id deserializeJson() failed:");
            logLine(error.c_str());

            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/rotorhazard/pilot_id finished");
            return;
        }

        if (!doc.containsKey("pilotId")) {
            logLine("WebUi::server POST /api/rotorhazard/pilot_id no pilotId in payload");
            this->server.send(400, "application/json", "{\"error\": \"no pilotId in payload\"}");
            logLine("WebUi::server POST /api/rotorhazard/pilot_id finished");
            return;
        }

        uint32_t pilotId = doc["pilotId"].as<uint32_t>();

        if (this->setRotorhazardPilotId == NULL || this->setRotorhazardPilotId == nullptr) {
            logLine("WebUi::server POST /api/rotorhazard/pilot_id no setRotorhazardPilotId handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/rotorhazard/pilot_id finished");
            return;
        }

        logInline("WebUi::server POST /api/rotorhazard/pilot_id calling setRotorhazardPilotId with pilotId: ");
        logLine(pilotId);
        this->setRotorhazardPilotId(pilotId);

        this->server.send(200, "application/json", "{\"success\": true}"); });

    this->server.on("/api/rotorhazard/connection", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/rotorhazard/connection-settings requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, this->server.arg("plain"));

        if (error) {
            logLine("WebUi::server POST /api/rotorhazard/connection-settings deserializeJson() failed:");
            logLine(error.c_str());

            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/rotorhazard/connection-settings finished");
            return;
        }

        if (!doc.containsKey("hostname")) {
            logLine("WebUi::server POST /api/rotorhazard/connection-settings no hostname in payload");
            this->server.send(400, "application/json", "{\"error\": \"no hostname in payload\"}");
            logLine("WebUi::server POST /api/rotorhazard/connection-settings finished");
            return;
        }

        String hostname = doc["hostname"].as<String>();
        int port = doc["port"].as<int>();
        String socketioPath = doc["socketioPath"].as<String>();
        
        if (socketioPath.equals("null")) {
            socketioPath = "";
        }

        if (this->setRotorhazardConnectionData == NULL || this->setRotorhazardConnectionData == nullptr) {
            logLine("WebUi::server POST /api/rotorhazard/connection-settings no setRotorhazardConnectionData handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/rotorhazard/connection-settings finished");
            return;
        }

        this->setRotorhazardConnectionData(hostname, port, socketioPath);

        this->server.send(200, "application/json", "{\"success\": true}");

        logLine("WebUi::server POST /api/rotorhazard/connection-settings finished"); });

    this->server.on("/api/flight_controller/baud_rate", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/flight-controller/baud-rate requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, this->server.arg("plain"));

        if (error) {
            logLine("WebUi::server POST /api/flight-controller/baud-rate deserializeJson() failed:");
            logLine(error.c_str());

            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/flight-controller/baud-rate finished");
            return;
        }

        if (!doc.containsKey("baudRate")) {
            logLine("WebUi::server POST /api/flight-controller/baud-rate no baudRate in payload");
            this->server.send(400, "application/json", "{\"error\": \"no baudRate in payload\"}");
            logLine("WebUi::server POST /api/flight-controller/baud-rate finished");
            return;
        }

        int baudRate = doc["baudRate"].as<int>();

        if (this->setFlightControllerBaudRate == NULL || this->setFlightControllerBaudRate == nullptr) {
            logLine("WebUi::server POST /api/flight-controller/baud-rate no setBaudRate handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/flight-controller/baud-rate finished");
            return;
        }

        this->setFlightControllerBaudRate(baudRate);

        this->server.send(200, "application/json", "{\"success\": true}");

        logLine("WebUi::server POST /api/flight-controller/baud-rate finished"); });

    this->server.on("/api/logging/baud-rate", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/logging/baud-rate requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, this->server.arg("plain"));

        if (error) {
            logLine("WebUi::server POST /api/logging/baud-rate deserializeJson() failed:");
            logLine(error.c_str());

            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/logging/baud-rate finished");
            return;
        }

        if (!doc.containsKey("baudRate")) {
            logLine("WebUi::server POST /api/logging/baud-rate no baudRate in payload");
            this->server.send(400, "application/json", "{\"error\": \"no baudRate in payload\"}");
            logLine("WebUi::server POST /api/logging/baud-rate finished");
            return;
        }

        int baudRate = doc["baudRate"].as<int>();

        if (this->setLoggingBaudRate == NULL || this->setLoggingBaudRate == nullptr) {
            logLine("WebUi::server POST /api/logging/baud-rate no setBaudRate handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/logging/baud-rate finished");
            return;
        }

        this->setLoggingBaudRate(baudRate);

        this->server.send(200, "application/json", "{\"success\": true}");

        logLine("WebUi::server POST /api/logging/baud-rate finished"); });

    this->server.on("/api/ota/actions/enable", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/ota/actions/enable requested");
        
        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        if (this->enableOta == NULL || this->enableOta == nullptr) {
            logLine("WebUi::server POST /api/ota/actions/enable no enableOta handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/ota/actions/enable finished");
            return;
        }

        this->enableOta();

        this->server.send(200, "application/json", "{\"success\": true}");

        logLine("WebUi::server POST /api/ota/actions/enable finished"); });

    this->server.on("/api/ota/actions/disable", HTTP_POST, [this]()
                    {
        logLine("WebUi::server POST /api/ota/actions/disable requested");

        this->setDefaults();

        if (!this->requireAuthentication()) {
            return;
        }

        if (this->disableOta == NULL || this->disableOta == nullptr) {
            logLine("WebUi::server POST /api/ota/actions/disable no disableOta handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server POST /api/ota/actions/disable finished");
            return;
        }

        this->disableOta();

        this->server.send(200, "application/json", "{\"success\": true}");

        logLine("WebUi::server POST /api/ota/actions/disable finished"); });

    this->server.on("/api/all", HTTP_GET, [this]()
                    {
        // collect all data of all HTTP_GET endpoints above and send in one big response
        logLine("WebUi::server GET /api/all requested");

        this->setDefaults();

        if (this->getHotspotStatus == NULL || this->getHotspotStatus == nullptr) {
            logLine("WebUi::server GET /api/all no hotspotStatusCallback");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server GET /api/all finished");
            return;
        }

        if (this->getStoredWifiSSID == NULL || this->getStoredWifiSSID == nullptr) {
            logLine("WebUi::server GET /api/all no getStoredWifiSSID handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server GET /api/all finished");
            return;
        }

        if (this->getHotspotCredentials == NULL || this->getHotspotCredentials == nullptr) {
            logLine("WebUi::server GET /api/all no getHotspotCredentials handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server GET /api/all finished");
            return;
        }

        if (this->getRotorhazardPilotId == NULL || this->getRotorhazardPilotId == nullptr) {
            logLine("WebUi::server GET /api/all no getRotorhazardPilotId handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server GET /api/all finished");
            return;
        }

        if (this->getRotorhazardConnectionData == NULL || this->getRotorhazardConnectionData == nullptr) {
            logLine("WebUi::server GET /api/all no getRotorhazardConnectionData handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server GET /api/all finished");
            return;
        }

        if (this->getFlightControllerBaudRate == NULL || this->getFlightControllerBaudRate == nullptr) {
            logLine("WebUi::server GET /api/all no getFlightControllerBaudRate handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server GET /api/all finished");
            return;
        }

        if (this->getLoggingBaudRate == NULL || this->getLoggingBaudRate == nullptr) {
            logLine("WebUi::server GET /api/all no getLoggingBaudRate handler");
            this->server.send(500, "application/json", "{\"error\": \"Internal Server Error\"}");
            logLine("WebUi::server GET /api/all finished");
            return;
        }

        StaticJsonDocument<1024> doc;
        // wifi subobject for wifi and hotspot
        JsonObject network = doc.createNestedObject("wifi");
        network["storedSSID"] = this->getStoredWifiSSID();


        // hotsport subobject
        JsonObject hotspot = doc.createNestedObject("hotspot");
        hotspot["isEnabled"] = this->getHotspotStatus();

        // hotspot.credentials subobject
        network_credentials_t hotspotCredentials = this->getHotspotCredentials();
        JsonObject hotspotCredentialsSubObject = hotspot.createNestedObject("credentials");
        hotspotCredentialsSubObject["ssid"] = hotspotCredentials.ssid;
        hotspotCredentialsSubObject["password"] = hotspotCredentials.password;


        // rotorhazard subobject
        JsonObject rotorhazard = doc.createNestedObject("rotorhazard");
        rotorhazard["pilotId"] = this->getRotorhazardPilotId();

        // rotorhazard.connection subobject
        rh_connection_data_t connectionData = this->getRotorhazardConnectionData();
        JsonObject connection = rotorhazard.createNestedObject("connection");
        connection["hostname"] = connectionData.hostname;
        connection["port"] = connectionData.port;
        connection["socketIOPath"] = connectionData.socketio_path;


        // serial subobject
        JsonObject serial = doc.createNestedObject("serial");
        serial["flightControllerBaudRate"] = this->getFlightControllerBaudRate();
        serial["loggingBaudRate"] = this->getLoggingBaudRate();


        // ota subobject
        JsonObject ota = doc.createNestedObject("ota");
        ota["isEnabled"] = this->getOtaEnabled();

        String response;
        serializeJson(doc, response);

        this->server.send(200, "application/json", response);

        logLine("WebUi::server GET /api/all finished"); });

    server.on(
        "/api/system/actions/update", HTTP_POST, [this]()
        {
            logLine("WebUI::server POST /api/system/actions/update started");
            this->setDefaults();

            this->server.sendHeader("Connection", "close");

            if (!this->requireAuthentication()) {
                logLine("WebUI::server POST /api/system/actions/update failed auth");
                return;
            }

            if (this->uploadFailed) {
                logLine("WebUI::server POST /api/system/actions/update failed upload");

                switch (this->uploadFailedReason) {
                    // case OTA_UPLOAD_FAILED_REASON_AUTHENTICATION:
                    case 0:
                        logLine("Update failed, unauthorized");
                        this->server.send(403, "application/json", "{\"success\": false, \"error\": \"unauthorized\"}");
                        break;

                    // case OTA_UPLOAD_FAILED_REASON_FIRMWARE_FILE_NAME:
                    case 1:
                        logLine("Update of firmware can only be done using a firmware.bin file");
                        this->server.send(400, "application/json", "{\"success\": false, \"error\": \"wrong firmware file\"}");
                        break;

                    // case OTA_UPLOAD_FAILED_REASON_FILESYSTEM_FILE_NAME:
                    case 2:
                        logLine("Update of filesystem can only be done using a filesystem.bin file");
                        this->server.send(400, "application/json", "{\"success\": false, \"error\": \"wrong filesystem file\"}");
                        break;
                }

                this->uploadFailed = false;
                return;
            }

            this->server.send(200, "application/json", "{\"success\": " + String((Update.hasError()) ? "false" : "true") + "}");

            if (this->uploadMode == FIRMWARE_UPLOAD_MODE_SKETCH) {
                ESP.restart();
            } },
        [this]()
        {
            if (this->uploadFailed)
            {
                logLine("WebUI::server POST /api/system/actions/update failed flag is present");
                yield();
                return;
            }

            if (!this->isAuthenticated())
            {
                logLine("WebUI::server POST /api/system/actions/update not authenticated");
                this->uploadFailed = true;
                this->uploadFailedReason = FIRMWARE_UPLOAD_FAILED_REASON_AUTHENTICATION;
                return;
            }

            this->uploadMode = FIRMWARE_UPLOAD_MODE_SKETCH;
            // Get mode from arg
            if (this->server.hasArg("mode"))
            {
                String argValue = this->server.arg("mode");
                if (argValue == "fs")
                {
                    this->uploadMode = FIRMWARE_UPLOAD_MODE_FILESYSTEM;
                }
            }

            HTTPUpload &upload = this->server.upload();

            if (this->uploadMode == FIRMWARE_UPLOAD_MODE_SKETCH && !upload.filename.equals("firmware.bin"))
            {
                logLine("Update of firmware can only be done using a firmware.bin file");
                this->uploadFailed = true;
                this->uploadFailedReason = FIRMWARE_UPLOAD_FAILED_REASON_FIRMWARE_FILE_NAME;
                return;
            }

            if (this->uploadMode == FIRMWARE_UPLOAD_MODE_FILESYSTEM && !upload.filename.equals("filesystem.bin"))
            {
                logLine("Update of filesystem can only be done using a filesystem.bin file");
                this->uploadFailed = true;
                this->uploadFailedReason = FIRMWARE_UPLOAD_FAILED_REASON_FILESYSTEM_FILE_NAME;
                return;
            }

            if (upload.status == UPLOAD_FILE_START)
            {
                WiFiUDP::stopAll();
                logInline("Update using file: ");
                logLine(upload.filename.c_str());

                uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;

                FSInfo fs_info;
                LittleFS.info(fs_info);
                uint32_t maxFSSpace = fs_info.totalBytes;

                uint32_t update_size = this->uploadMode == FIRMWARE_UPLOAD_MODE_FILESYSTEM ? maxFSSpace : maxSketchSpace;
                if (!Update.begin(update_size, this->uploadMode == FIRMWARE_UPLOAD_MODE_FILESYSTEM ? U_FS : U_FLASH))
                {
                    StreamString errorStr;
                    Update.printError(errorStr);
                    logLine("Update: ERROR");
                    logLine(errorStr.c_str());
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE)
            {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                    StreamString errorStr;
                    Update.printError(errorStr);
                    logLine("Update: ERROR");
                    logLine(errorStr.c_str());
                }
            }
            else if (upload.status == UPLOAD_FILE_END)
            {
                if (Update.end(true))
                {
                    logLine("Update: SUCCESS");
                }
                else
                {
                    StreamString errorStr;
                    Update.printError(errorStr);
                    logLine("Update: ERROR");
                    logLine(errorStr.c_str());
                }
            }
            yield();
        });

    this->server.onNotFound([this]()
                            {
        logLine("WebUI::onNotFound: " + this->server.uri());
        
        this->setDefaults();

        // make sure OPTIONS request allways are 200 OK
        if (this->server.method() == HTTP_OPTIONS)
        {
            logLine("WebUI::onNotFound: OPTIONS request");
            this->server.send(200);
            return;
        }

        // check if path corresponds to a file in the filesystem
        String uri = this->server.uri();
        if (this->handleFileRead(uri))
        {
            return;
        }

        if (uri.startsWith("/api/"))
        {
            logLine("WebUI::onNotFound: api request");
            this->server.send(404, "application/json", "{\"error\": \"Not Found\"}");
            return;
        }

        logLine("WebUI::onNotFound: redirecting to index.html");
        this->server.sendHeader("Location", String("http://") + ipToString(this->server.client().localIP()), true);
        this->server.send(302, "text/plain", "");
        this->server.client().stop();

        logLine("WebUI::onNotFound: finished");
        return; });
}

void WebUI::loop()
{
    this->server.handleClient();
}

inline String getContentType(String filename)
{
    if (filename.endsWith(".html"))
    {
        return "text/html";
    }

    if (filename.endsWith(".css"))
    {
        return "text/css";
    }

    if (filename.endsWith(".js"))
    {
        return "application/javascript";
    }

    if (filename.endsWith(".ico"))
    {
        return "image/x-icon";
    }

    if (filename.endsWith(".png"))
    {
        return "image/png";
    }

    if (filename.endsWith(".jpg"))
    {
        return "image/jpeg";
    }

    if (filename.endsWith(".jpeg"))
    {
        return "image/jpeg";
    }

    if (filename.endsWith(".gif"))
    {
        return "image/gif";
    }

    if (filename.endsWith(".svg"))
    {
        return "image/svg+xml";
    }

    return "text/plain";
}

String WebUI::getDefaultIndexHTML()
{
    String html;

    html.concat("<!DOCTYPE html>");
    html.concat("<html lang='en'>");
    html.concat("<head>");
    html.concat("<meta charset='utf-8'>");
    html.concat("<meta name='viewport' content='width=device-width, initial-scale=1, shrink-to-fit=no'>");
    html.concat("<title>RotorHazard MSP OSD Injector</title>");
    html.concat("<script type='text/javascript'>");
    html.concat("let uploadFilesystemForm;");
    html.concat("const hostname = location.hostname;");
    html.concat("const protocol = location.protocol;");
    html.concat("const apiUrl = protocol + '//' + hostname + '/api';");

    html.concat("function requestPin() {");
    html.concat("const fullUrl = apiUrl + '/pin';");
    html.concat("fetch(fullUrl, {");
    html.concat("method: 'GET',");
    html.concat("headers: {");
    html.concat("'Content-Type': 'application/json'");
    html.concat("}");
    html.concat("})");
    html.concat(".then(response => {");
    html.concat("if (response.ok) {");
    html.concat("alert('PIN requested');");
    html.concat("} else {");
    html.concat("alert('Error: ' + response.statusText);");
    html.concat("}");
    html.concat("})");
    html.concat(".catch(error => alert('Error: ' + error));");
    html.concat("}");

    html.concat("function onSubmit(e) {");
    html.concat("e.preventDefault();");
    html.concat("const submitBtn = uploadFilesystemForm.querySelector(\"button[type='submit']\");");
    html.concat("const originalButtonText = submitBtn.textContent;");
    html.concat("const fullUrl = apiUrl + '/system/actions/update?mode=fs';");
    html.concat("const form = document.forms[0];");
    html.concat("const formData = new FormData(form);");
    // send as multipart/form-data
    html.concat("submitBtn.textContent = '...uploading...';");
    html.concat("fetch(fullUrl, {");
    html.concat("method: 'POST',");
    html.concat("body: formData,");
    html.concat("headers: {");
    html.concat("Authorization: `Basic ${btoa(`rh-osd:${formData.get('pinCode')}`)}`");
    html.concat("}");
    html.concat("})");
    // reload page after upload
    html.concat(".then(response => {");
    html.concat("if (response.ok) {");
    html.concat("alert('Upload successful');");
    html.concat("location.reload();");
    html.concat("} else {");
    html.concat("alert('Error: ' + response.statusText);");
    html.concat("}");
    html.concat("})");
    html.concat(".catch(error => alert('Error: ' + error))");
    html.concat(".finally(() => {");
    html.concat("submitBtn.textContent = originalButtonText;");
    html.concat("});");
    html.concat("}");

    // attach event listener to form submit
    html.concat("document.addEventListener('DOMContentLoaded', function() {");
    html.concat("uploadFilesystemForm = document.getElementById('upload-fs-form');");
    html.concat("uploadFilesystemForm.addEventListener('submit', onSubmit);");

    // attach event listener to pin code request button
    html.concat("document.getElementById('pin-code-request-btn').addEventListener('click', requestPin);");
    html.concat("});");

    html.concat("</script>");
    html.concat("</head>");

    html.concat("<body>");

    html.concat("<h1>Rotorhazard MSP OSD Injector</h1>");
    html.concat("<small>First time setup</small>");

    html.concat("<p>Please upload the filesystem / webUI binary</p>");

    html.concat("<form id='upload-fs-form'>");
    // inform the user that file uploads might not work inside the systems wifi login window and they should try using a real browser
    // put link of current page for reference
    html.concat("<p>File uploads might not work inside the systems wifi login window. Please try using a real browser like Chrome, Firefox, Safari, etc.</p>");
    html.concat("<p>The address to this site is <a href='http://10.0.0.1'>http://10.0.0.1</a></p>");

    html.concat("<input required type='file' name='update'><br />");

    html.concat("<input required type='text' name='pinCode' placeholder='pin code'>");
    html.concat("<button type='button' id='pin-code-request-btn'>Request PIN (OSD)</button><br />");
    html.concat("<button type='submit'>Upload</button>");
    html.concat("</form>");

    html.concat("</body>");
    html.concat("</html>");

    return html;
}

bool WebUI::handleFileRead(String uriInput)
{
    logLine("WebUI::handleFileRead: " + uriInput);

    // if the uri is empty, send the index.html file
    if (uriInput.length() == 1)
    {
        uriInput = "/index.html";
    }

    bool found = false;
    String uriForFileLookup = String(uriInput);
    if (LittleFS.exists(uriForFileLookup + ".gz"))
    {
        uriForFileLookup += ".gz";
        found = true;
    }
    else if (LittleFS.exists(uriForFileLookup))
    {
        found = true;
    }

    if (!found && uriInput == "/index.html")
    {
        logLine("WebUI::handleFileRead: index.html not found in fs, sending default index.html");
        String indexHtml = this->getDefaultIndexHTML();
        this->server.send(200, "text/html", indexHtml);
    }

    // if the file exists, send it
    if (!found)
    {
        logLine("WebUI::handleFileRead file not found: " + uriInput);
        return false;
    }

    File file = LittleFS.open(uriForFileLookup, "r");
    String contentType = getContentType(uriInput);

    logInline("WebUI::handleFielRead streaming file with content type: ");
    logLine(contentType);
    this->server.streamFile(file, contentType);

    file.close();

    logLine("WebUI::handleFileRead file sent: " + uriForFileLookup);
    return true;
}

void WebUI::onPinChanged(pin_changed_callback_t callback)
{
    this->pinChangedCallback = callback;
}

void WebUI::setGetHotspotStatusHandler(get_hotspot_status_handler_t handler)
{
    this->getHotspotStatus = handler;
}

void WebUI::setConectToWifiHandler(connect_to_wifi_handler_t handler)
{
    this->connectToWifi = handler;
}

void WebUI::setGetStoredWifiSSIDHandler(get_stored_wifi_ssid_handler_t handler)
{
    this->getStoredWifiSSID = handler;
}

void WebUI::generateNewPin()
{
    logLine("WebUi::generateNewPin started");

    for (int i = 0; i < 4; i++)
    {
        this->pin[i] = random(0, 9);
    }

    logLine("WebUi::generateNewPin generated new pin: ");
    logLine(this->pin[0]);
    logLine(this->pin[1]);
    logLine(this->pin[2]);
    logLine(this->pin[3]);

    if (this->pinChangedCallback != NULL && this->pinChangedCallback != nullptr)
    {
        logLine("WebUi::generateNewPin calling pinChangedCallback");
        this->pinChangedCallback(this->getPinAsString());
    }

    logLine("WebUi::generateNewPin finished");
}

String WebUI::getPinAsString()
{
    String pinAsString = "";
    for (int i = 0; i < 4; i++)
    {
        pinAsString += String(this->pin[i]);
    }

    return pinAsString;
}

bool WebUI::isAuthenticated()
{
    return this->server.authenticate("rh-osd", this->getPinAsString().c_str());
}

bool WebUI::requireAuthentication()
{
    if (!this->isAuthenticated())
    {
        logLine("Request is not authenticated... sending 403");
        this->server.send(403, "application/json", "{\"success\": false, \"error\": \"unauthorized\"}");
        return false;
    }

    return true;
}

void WebUI::setSetWifiCredentialsHandler(set_wifi_credentials_handler_t handler)
{
    this->setWifiCredentials = handler;
}

void WebUI::setEnableHotspothandler(enable_hotspot_handler_t handler)
{
    this->enableHotspot = handler;
}

void WebUI::setSetRotorhazardPilotIdHandler(set_rotorhazard_pilot_id_handler_t handler)
{
    this->setRotorhazardPilotId = handler;
}

void WebUI::setSetRotorhazardConnectionDataHandler(set_rotorhazard_connection_data_handler_t handler)
{
    this->setRotorhazardConnectionData = handler;
}

void WebUI::setGetRotorhazardConnectionDataHandler(get_rotorhazard_connection_data_handler_t handler)
{
    this->getRotorhazardConnectionData = handler;
}

void WebUI::setGetRotorhazardPilotIdHandler(get_rotorhazard_pilot_id_handler_t handler)
{
    this->getRotorhazardPilotId = handler;
}

void WebUI::setGetFlightControllerBaudRateHandler(get_baud_rate_handler_t handler)
{
    this->getFlightControllerBaudRate = handler;
}

void WebUI::setSetFlightControllerBaudRateHandler(set_baud_rate_handler_t handler)
{
    this->setFlightControllerBaudRate = handler;
}

void WebUI::setSetHotspotCredentialsHandler(set_hotspot_credentials_handler_t handler)
{
    this->setHotspotCredentials = handler;
}

void WebUI::setGetHotspotCredentialsHandler(get_hotspot_credentials_handler_t handler)
{
    this->getHotspotCredentials = handler;
}

void WebUI::setGetLoggingBaudRateHandler(get_baud_rate_handler_t handler)
{
    this->getLoggingBaudRate = handler;
}

void WebUI::setSetLoggingBaudRateHandler(set_baud_rate_handler_t handler)
{
    this->setLoggingBaudRate = handler;
}

void WebUI::setEnableOtaHandler(enable_ota_handler_t handler)
{
    this->enableOta = handler;
}

void WebUI::setDisableOtaHandler(disable_ota_handler_t handler)
{
    this->disableOta = handler;
}

void WebUI::setGetOtaEnabledHandler(get_ota_enabled_handler_t handler)
{
    this->getOtaEnabled = handler;
}
