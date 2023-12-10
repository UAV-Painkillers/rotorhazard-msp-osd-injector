#include "ota.hpp"

void OTA::setup() {
    logLine("OTA::setup");

    ArduinoOTA.setHostname(this->getHostname().c_str());

    ArduinoOTA.onStart([this]() {
        logLine("OTA::onStart");

        if (this->onStatusChangedCallback) {
            this->onStatusChangedCallback(OTA_IN_PROGRESS);
        }
    });

    ArduinoOTA.onEnd([this]() {
        logLine("OTA::onEnd");

        if (this->onStatusChangedCallback) {
            this->onStatusChangedCallback(OTA_SUCCEEDED);
        }
    });

    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        logInline("OTA::onProgress: ");
        logInline(progress / (total / 100));
        logLine("%");

        if (this->onProgressChangedCallback) {
            this->onProgressChangedCallback(progress / (total / 100));
        }
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        logInline("OTA::onError: ");
        logLine(error);

        if (this->onErrorCallback) {
            this->onErrorCallback(error);
        }

        if (this->onStatusChangedCallback) {
            this->onStatusChangedCallback(OTA_FAILED);
        }
    });

    logLine("OTA::setup done");
}

void OTA::loop() {
    if (this->enabled) {
        ArduinoOTA.handle();
    }
}

void OTA::enable() {
    logLine("OTA::enable");
    this->enabled = true;

    ArduinoOTA.begin(true);

    if (this->onStatusChangedCallback) {
        this->onStatusChangedCallback(OTA_ENABLED_WAITING);
    }
}

void OTA::disable() {
    logLine("OTA::disable");
    this->enabled = false;

    if (this->onStatusChangedCallback) {
        this->onStatusChangedCallback(OTA_DISABLED);
    }
}

void OTA::onStatusChanged(ota_status_changed_callback_t callback) {
    this->onStatusChangedCallback = callback;
}

void OTA::onProgressChanged(ota_progress_changed_callback_t callback) {
    this->onProgressChangedCallback = callback;
}

void OTA::onError(ota_error_callback_t callback) {
    this->onErrorCallback = callback;
}

String OTA::getHostname() {
    String hostname = EEPROMManager::data.hostname;
    
    if (hostname.equals("")) {
        hostname = String(DEFAULT_HOSTNAME);
    }

    return hostname;
}

bool OTA::isEnabled() {
    return this->enabled;
}
