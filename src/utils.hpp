#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#define EVERY_N_MILLISECONDS(N) for(static unsigned long _lastTime = 0; \
                                     (millis() - _lastTime) >= (N); \
                                     _lastTime = millis())

String ipToString(IPAddress ip);