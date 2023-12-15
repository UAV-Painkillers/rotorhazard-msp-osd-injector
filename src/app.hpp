#pragma once

#include <SoftwareSerial.h>

#include "rotorhazard.hpp"
#include "msp-controller.hpp"
#include "wifi-connection.hpp"
#include "eeprom.hpp"
#include "web-ui.hpp"
#include "const.h"
#include "utils.hpp"
#include "ota.hpp"

class App {
    public:
        void setup();
        void loop();

    private:
        WiFiConnection wifiConnection;
        RotorHazard rotorHazard;
        WebUI webUI;
        MSPController mspController;
        OTA ota;

        EspSoftwareSerial::UART *mspSoftSerial;
        Stream *mspSerial;

        void setupSerial(bool endOldSerial = true);
        void linkCallbacks();
        uint32_t getMspBaudrate();
        uint32_t getLoggingBaudrate();
        void setMspBaudrate(uint32_t baudrate);
        void setLoggingBaudrate(uint32_t baudrate);
};
