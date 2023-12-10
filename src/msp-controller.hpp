#pragma once

#include <MSP.h>

#include "const.h"
#include "logging.hpp"

struct msp_message {
    String message;
    unsigned long durationMs;
    bool active = false;
};

class MSPController {
    public:
        void setup(Stream & stream);
        MSP msp;
        void printMessage(String message, unsigned long durationMs);
        void loop();

    private:
        int my_pilot_id;
        // queue
        msp_message messageQueue[10];
        unsigned long nextMessageTime = 0;

        void clearDisplay();
};
