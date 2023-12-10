#pragma once

#include <ArduinoOTA.h>
#include <functional>

#include "logging.hpp"
#include "const.h"
#include "eeprom.hpp"

enum ota_status_t {
    OTA_DISABLED,
    OTA_ENABLED_WAITING,
    OTA_IN_PROGRESS,
    OTA_FAILED,
    OTA_SUCCEEDED,
};

// callback to be called when OTA status changes. containing: status, progress (%) and error (if any)
typedef std::function<void(ota_status_t)> ota_status_changed_callback_t;
typedef std::function<void(int)> ota_progress_changed_callback_t;
typedef std::function<void(ota_error_t)> ota_error_callback_t;


class OTA {
    public:
        void setup();
        void loop();
        void enable();
        void disable();
        void onStatusChanged(ota_status_changed_callback_t callback);
        void onProgressChanged(ota_progress_changed_callback_t callback);
        void onError(ota_error_callback_t callback);
        bool isEnabled();

    private:
        String getHostname();

        ota_status_changed_callback_t onStatusChangedCallback;
        ota_progress_changed_callback_t onProgressChangedCallback;
        ota_error_callback_t onErrorCallback;

        bool enabled = false;
};
