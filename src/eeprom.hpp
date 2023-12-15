#pragma once

#include <Arduino.h>
#include <EEPROM.h>

#include "logging.hpp"

#define EEPROM_VERSION 9

struct eeprom_data_t {
    // EEPROM version
    uint8_t eeprom_version;
    // wifi ssid
    char wifi_ssid[64];
    // wifi password
    char wifi_password[64];
    // RotorHazard server hostname
    char rh_hostname[64];
    // RotorHazard server port
    uint16_t rh_port;
    // hotspot ssid
    char hotspot_ssid[64];
    // hotspot password
    char hotspot_password[64];
    // hostname
    char hostname[64];
    // RotorHazard server socket io path
    char rh_socketio_path[64];
    // pilot id for lap filtering
    uint32_t rh_pilot_id;
    // flightcontroller msp baud
    uint32_t fc_msp_baud;
    // logging baud
    uint32_t logging_baud;
    // uses main serial for fc communication
    bool fc_serial_uses_main_serial;
    int fc_soft_serial_rx_pin;
    int fc_soft_serial_tx_pin;
};

class EEPROMManager {
    public:
        static void setup();
        static void readEEPROM();
        static void writeEEPROM();
        static eeprom_data_t data;
};
