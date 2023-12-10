#include "eeprom.hpp"
#include "const.h"

eeprom_data_t EEPROMManager::data;

void EEPROMManager::setup() {
    logLine("EEPROMManager::setup");
    EEPROM.begin(sizeof(eeprom_data_t));
    EEPROMManager::readEEPROM();
    logLine("EEPROMManager::setup done");
}

void EEPROMManager::readEEPROM() {
    logLine("EEPROMManager::readEEPROM");
    EEPROM.get(0, EEPROMManager::data);

    // set defaults and clear junk
    if (EEPROMManager::data.eeprom_version != EEPROM_VERSION) {
        logLine("EEPROMManager::readEEPROM no valid data found, setting defaults");
        EEPROMManager::data.eeprom_version = EEPROM_VERSION;
        EEPROMManager::data.wifi_ssid[0] = '\0';
        EEPROMManager::data.wifi_password[0] = '\0';
        EEPROMManager::data.rh_hostname[0] = '\0';
        EEPROMManager::data.rh_port = 0;
        EEPROMManager::data.rh_socketio_path[0] = '\0';
        EEPROMManager::data.hotspot_ssid[0] = '\0';
        EEPROMManager::data.hotspot_password[0] = '\0';
        EEPROMManager::data.hostname[0] = '\0';
        EEPROMManager::data.fc_msp_baud = 0;
        EEPROMManager::data.logging_baud = 0;

        logLine("EEPROMManager::readEEPROM writing defaults");
        EEPROMManager::writeEEPROM();
    }

    logLine("EEPROMManager::readEEPROM done");
}

void EEPROMManager::writeEEPROM() {
    logLine("EEPROMManager::writeEEPROM");
    EEPROM.put(0, EEPROMManager::data);

    logLine("EEPROMManager::writeEEPROM committing");
    EEPROM.commit();

    logLine("EEPROMManager::writeEEPROM done");
}
