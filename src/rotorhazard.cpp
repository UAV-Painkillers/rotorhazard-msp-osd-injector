#include "rotorhazard.hpp"

void RotorHazard::setup() {
    logLine("RotorHazard::setup init...");

    this->connect();

    logLine("RotorHazard::setup complete");
}

void RotorHazard::loop() {
    this->socketIO.loop();
}

void RotorHazard::onSocketEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            logLine("RotorHazard::onSocketEvent sIOtype_DISCONNECT");
            this->handleConnectionStatusChange(RH_DISCONNECTED);
            break;
        case sIOtype_CONNECT:
            logLine("RotorHazard::onSocketEvent sIOtype_CONNECT");
            this->handleConnectionStatusChange(RH_CONNECTED);

            // join default namespace (no auto join in Socket.IO V3)
            this->socketIO.send(sIOtype_CONNECT, "/");
            break;
        case sIOtype_EVENT:
            this->handleTimerEvent(payload);
            break;
        case sIOtype_ERROR:
            logLine("RotorHazard::onSocketEvent sIOtype_ERROR");
            this->handleConnectionStatusChange(RH_CONNECTION_FAILED);
            break;
    }
}

void RotorHazard::requestAllData() {
    logLine("RotorHazard::handleTimerEvent Received load_all event");
    DynamicJsonDocument docToSend(1024);
    JsonArray docToSendAsArray = docToSend.to<JsonArray>();

    docToSendAsArray.add("load_data");

    JsonObject loadTypesObject = docToSendAsArray.createNestedObject();
    JsonArray loadTypes = loadTypesObject.createNestedArray("load_types");

    loadTypes.add("current_laps");

    String output;
    serializeJson(docToSendAsArray, output);

    this->socketIO.sendEVENT(output);
}

void RotorHazard::handleTimerEvent(uint8_t * payload) {
    this->incomingJsonDocument.clear();

    DeserializationError error = deserializeJson(this->incomingJsonDocument, payload);
    if (error) {
        logLine("RotorHazard::handleTimerEvent deserializeJson() failed: ");
        logLine(error.f_str());
        return;
    }

    JsonArray docArray = this->incomingJsonDocument.as<JsonArray>();
    String eventType = docArray[0];

    if (eventType.equals("load_all")) {
        this->requestAllData();
    }

    if (eventType.equals("current_laps")) {
        this->updateCurrentLaps(docArray[1].as<JsonObject>());
    }
}

void RotorHazard::updateCurrentLaps(JsonObject data) {
    logLine("RotorHazard::updateCurrentLaps started");

    JsonArray nodes = data["current"]["node_index"].as<JsonArray>();

    uint32_t pilotIdForFiltering = this->getPilotIdForLapFiltering();
    for (int nodeIndex = 0; nodeIndex < 8; nodeIndex++) {
        JsonObject nodeData = nodes[nodeIndex].as<JsonObject>();
        JsonObject pilot = nodeData["pilot"].as<JsonObject>();

        uint32_t pilotId = pilot["id"].as<uint32_t>();
        if (pilotId != pilotIdForFiltering) {
            continue;
        }

        if (this->last_known_node_index_of_pilot != nodeIndex) {
            this->last_known_node_index_of_pilot = nodeIndex;
            logInline("RotorHazard::updateCurrentLaps node index of pilot changed to ");
            logLine(this->last_known_node_index_of_pilot);
        }

        JsonArray laps = nodeData["laps"].as<JsonArray>();
        int nextLastLapIndex = -1;
        uint32_t nextLastLapNumber = this->last_known_finished_lap_number;

        for (JsonVariant lap : laps) {
            JsonObject lapData = lap.as<JsonObject>();
            uint32_t lapNumber = lapData["lap_number"].as<uint32_t>();

            if (this->last_known_finished_lap_number == -1 || lapNumber > (uint32_t)this->last_known_finished_lap_number) {
                nextLastLapIndex = lapData["lap_index"].as<int>();
                nextLastLapNumber = lapNumber;
            }
        }

        if (nextLastLapIndex == -1) {
            logLine("RotorHazard::updateCurrentLaps no new lap found for pilot");
            continue;
        }

        this->last_known_finished_lap_number = nextLastLapNumber;

        logInline("RotorHazard::updateCurrentLaps new lap found for pilot, lap index: ");
        logInline(nextLastLapIndex);
        logInline(", lap number: ");
        logLine(nextLastLapNumber);

        bool finishedFlag = nodeData["finished_flag"].as<bool>();
        JsonObject lapData = laps[nextLastLapIndex].as<JsonObject>();
        int fastestLapIndex = nodeData["fastest_lap_index"].as<int>();

        // prepare lap finished data
        rh_lap_finished_data lapFinishedData = {
            .slot_index = nodeIndex,
            .lap_number = nextLastLapNumber,
            .lap_time = lapData["lap_time"].as<const char*>(),
            .race_finished_flag = finishedFlag,
            .was_fastest_lap = fastestLapIndex == nextLastLapIndex,
            .pilot = {
                .id = pilotId,
                .callsign = pilot["callsign"].as<const char*>(),
                .name = pilot["name"].as<const char*>()
            }
        };

        if (this->lap_finished_callback != NULL && this->lap_finished_callback != nullptr) {
            logLine("RotorHazard::updateCurrentLaps calling lap_finished_callback");
            this->lap_finished_callback(lapFinishedData);
        }
    }

    logLine("RotorHazard::updateCurrentLaps done");
}

void RotorHazard::onLapFinished(rh_lap_finished_callback callback) {
    this->lap_finished_callback = callback;
}

void RotorHazard::onConnectionStatusChange(rh_connection_status_callback callback) {
    this->connection_status_callback = callback;
}

void RotorHazard::handleConnectionStatusChange(rh_connection_status status) {
    if (this->lastStatus == status) {
        return;
    }

    logInline("RotorHazard::handleConnectionStatusChange status changed to ");
    switch (status) {
        case RH_CONNECTED:
            logLine("CONNECTED");
            break;
        case RH_DISCONNECTED:
            logLine("DISCONNECTED");
            break;
        case RH_CONNECTION_FAILED:
            logLine("CONNECTION_FAILED");
            break;
    }

    this->lastStatus = status;

    if (this->connection_status_callback != NULL && this->connection_status_callback != nullptr) {
        logLine("RotorHazard::handleConnectionStatusChange calling connection_status_callback");
        this->connection_status_callback(status);
    }
}

void RotorHazard::connect() {
    logLine("RotorHazard::connect started");

    String hostname = this->getHostname();
    if (hostname.equals("")) {
        logLine("RotorHazard::connect no hostname set, aborting");
        return;
    }
    logInline("RotorHazard::connect hostname: ");
    logLine(hostname);

    uint16_t port = this->getPort();
    logInline("RotorHazard::connect port: ");
    logLine(port);

    String socketIoPath = this->getSocketIOPath();
    logInline("RotorHazard::connect socketIoPath: ");
    logLine(socketIoPath);

    logLine("RotorHazard::connect socketIO instance created, connecting...");
    this->socketIO.begin(hostname, port, socketIoPath);

    logLine("RotorHazard::connect socketIO connected, setting onEvent callback...");
    this->socketIO.onEvent([this](socketIOmessageType_t type, uint8_t * payload, size_t length) {
        this->onSocketEvent(type, payload, length);
    });

    logLine("RotorHazard::connect done");
}

void RotorHazard::storeConnectionData(String hostname, uint16_t port, String socketio_path) {
    logLine("RotorHazard::storeConnectionData started");

    hostname.trim();
    socketio_path.trim();

    if (hostname.equals("")) {
        logLine("RotorHazard::storeConnectionData hostname is empty, aborting");
        return;
    }

    if (port == 0) {
        logLine("RotorHazard::storeConnectionData port is 0, using default port");
        port = DEFAULT_RH_PORT;
    }

    if (socketio_path.equals("")) {
        logLine("RotorHazard::storeConnectionData socketio_path is empty, using default path");
        socketio_path = DEFAULT_RH_SOCKET_IO_PATH;
    }

    this->setHostname(hostname);
    this->setPort(port);
    this->setSocketIOPath(socketio_path);

    this->connect();

    logLine("RotorHazard::storeConnectionData done");
}

rh_connection_data_t RotorHazard::getConnectionData() {
    rh_connection_data_t connectionData = {
        .hostname = this->getHostname(),
        .port = this->getPort(),
        .socketio_path = this->getSocketIOPath()
    };

    return connectionData;
}

void RotorHazard::setPilotIdForLapFiltering(uint32_t pilot_id) {
    logLine("RotorHazard::setPilotIdForLapFiltering started");

    logInline("RotorHazard::setPilotIdForLapFiltering pilot id: ");
    logLine(pilot_id);
    EEPROMManager::data.rh_pilot_id = pilot_id;
    EEPROMManager::writeEEPROM();

    logLine("RotorHazard::setPilotIdForLapFiltering done");
}

uint32_t RotorHazard::getPilotIdForLapFiltering() {
    return EEPROMManager::data.rh_pilot_id;
}

String RotorHazard::getHostname() {
    String hostname = String(EEPROMManager::data.rh_hostname);
    hostname.trim();
    return hostname;
}

uint16_t RotorHazard::getPort() {
    return EEPROMManager::data.rh_port == 0 ? DEFAULT_RH_PORT : EEPROMManager::data.rh_port;
}

String RotorHazard::getSocketIOPath() {
    String socketIoPath = String(EEPROMManager::data.rh_socketio_path);
    socketIoPath.trim();

    if (socketIoPath.equals("")) {
        socketIoPath = DEFAULT_RH_SOCKET_IO_PATH;
    }

    return socketIoPath;
}

void RotorHazard::setHostname(String hostname) {
    hostname.trim();
    hostname.toCharArray(EEPROMManager::data.rh_hostname, 64);
    EEPROMManager::writeEEPROM();
}

void RotorHazard::setPort(uint16_t port) {
    EEPROMManager::data.rh_port = port;
    EEPROMManager::writeEEPROM();
}

void RotorHazard::setSocketIOPath(String socketio_path) {
    socketio_path.trim();
    socketio_path.toCharArray(EEPROMManager::data.rh_socketio_path, 64);
    EEPROMManager::writeEEPROM();
}
