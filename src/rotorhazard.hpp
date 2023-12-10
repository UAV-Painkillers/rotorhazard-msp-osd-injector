#pragma once

#include <WebSocketsClient.h>
#include <SocketIOclient.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include <functional>

#include "const.h"
#include "logging.hpp"
#include "eeprom.hpp"

struct rh_frequency_data {
    int heat_slot_index;
    const char* band;
    int channel;
    int frequency;
};

struct rh_heat_slot_data {
    uint8_t pilot_id;
    uint8_t slot_index;
};

struct rh_heat_data {
    uint8_t heat_id;
    rh_heat_slot_data pilot_slots[8];
};

struct rh_lap_data {
    const char* lap_time;
    int lap_number;
};

struct rh_pilot_data {
    int id;
    const char* callsign;
    const char* name;
};

struct rh_current_laps_data {
    int slot_index;
    int fastest_lap_index;
    int last_lap_index;
    bool finished_flag;
    rh_lap_data laps[128];
    rh_pilot_data pilot;
};

// type definition for on lap finished callback
struct rh_lap_finished_data {
    int slot_index;
    uint32_t lap_number;
    const char* lap_time;
    bool race_finished_flag;
    bool was_fastest_lap;
    rh_pilot_data pilot;
};
typedef std::function<void(rh_lap_finished_data lap_finished_data)> rh_lap_finished_callback;

// socketio status callback
enum rh_connection_status {
    RH_CONNECTED,
    RH_DISCONNECTED,
    RH_CONNECTION_FAILED
};
typedef std::function<void(rh_connection_status status)> rh_connection_status_callback;

struct rh_connection_data_t {
    String hostname;
    uint16_t port;
    String socketio_path;
};

class RotorHazard {
    public:
        void setup();
        void loop();

        rh_pilot_data* getPilots();
        void onLapFinished(rh_lap_finished_callback callback);
        void onConnectionStatusChange(rh_connection_status_callback callback);
        void connect();
        void storeConnectionData(String hostname, uint16_t port, String socketio_path = DEFAULT_RH_SOCKET_IO_PATH);
        rh_connection_data_t getConnectionData();
        void setPilotIdForLapFiltering(uint32_t pilot_id);
        uint32_t getPilotIdForLapFiltering();

    private:
        SocketIOclient socketIO;
        void onSocketEvent(socketIOmessageType_t type, uint8_t * payload, size_t length);
        void handleTimerEvent(uint8_t * payload);
        void updateFrequencyData(JsonObject data);
        void updateCurrentLaps(JsonObject data);
        void updatePilotData(JsonObject data);
        void handleConnectionStatusChange(rh_connection_status);
        void requestAllData();

        String getHostname();
        String getSocketIOPath();
        uint16_t getPort();
        void setHostname(String hostname);
        void setSocketIOPath(String socketio_path);
        void setPort(uint16_t port);

        StaticJsonDocument<4096> incomingJsonDocument;

        uint8_t last_known_node_index_of_pilot = -1;
        uint32_t last_known_finished_lap_number = -1;

        rh_lap_finished_callback lap_finished_callback;
        rh_connection_status_callback connection_status_callback;

        rh_connection_status lastStatus;
};
