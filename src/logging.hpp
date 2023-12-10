#pragma once
#include <Arduino.h>

inline void logInline(const char* message) {
  #ifndef FC_SERIAL_USES_MAIN_SERIAL
    Serial.print(message);
  #endif
}

inline void logInline(int message) {
  #ifndef FC_SERIAL_USES_MAIN_SERIAL
    Serial.print(message);
  #endif
}

inline void logInline(String message) {
  #ifndef FC_SERIAL_USES_MAIN_SERIAL
    Serial.print(message);
  #endif
}

inline void logLine(const char* message) {
  #ifndef FC_SERIAL_USES_MAIN_SERIAL
    Serial.println(message);
  #endif
}

inline void logLine(int message) {
  #ifndef FC_SERIAL_USES_MAIN_SERIAL
    Serial.println(message);
  #endif
}

inline void logLine(String message) {
  #ifndef FC_SERIAL_USES_MAIN_SERIAL
    Serial.println(message);
  #endif
}