#include "eeprom.hpp"
#include "const.h"

eeprom_data_t EEPROMManager::data;

void EEPROMManager::setup() {
    logLine("EEPROMManager::setup");
    EEPROM.begin(sizeof(eeprom_data_t));
    EEPROMManager::readEEPROM();
    logLine("EEPROMManager::setup done");
}

inline void migrateVersion8To9() {
    logLine("EEPROMManager::migrateVersion8To9");

    #ifdef FC_SERIAL_USES_MAIN_SERIAL
        EEPROMManager::data.fc_serial_uses_main_serial = true;
    #else
        EEPROMManager::data.fc_serial_uses_main_serial = false;
    #endif

    #ifdef FC_SOFT_SERIAL_TX_PIN
        EEPROMManager::data.fc_soft_serial_tx_pin = FC_SOFT_SERIAL_TX_PIN;
    #else
        EEPROMManager::data.fc_soft_serial_tx_pin = -1;
    #endif

    #ifdef FC_SOFT_SERIAL_RX_PIN
        EEPROMManager::data.fc_soft_serial_rx_pin = FC_SOFT_SERIAL_RX_PIN;
    #else
        EEPROMManager::data.fc_soft_serial_rx_pin = -1;
    #endif

    logLine("EEPROMManager::migrateVersion8To9 done");
}

void EEPROMManager::readEEPROM() {
    logLine("EEPROMManager::readEEPROM");
    EEPROM.get(0, EEPROMManager::data);

    bool needsWrite = false;
    // set defaults and clear junk
    if (EEPROMManager::data.eeprom_version < 8) {
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
        
        needsWrite = true;
    }

    if (EEPROMManager::data.eeprom_version < 9) {
        logLine("EEPROMManager::readEEPROM migrating from version 8 to 9");
        migrateVersion8To9();
        needsWrite = true;
    }

    if (needsWrite) {
        logLine("EEPROMManager::readEEPROM writing migrated data");
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
