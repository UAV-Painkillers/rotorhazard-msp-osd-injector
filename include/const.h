#pragma once

#include "secrets.h"

// RX Communication -------------------------------
#define DEFAULT_FC_SERIAL_BAUD                  57600

// LOGGING ---------------------------------------
#define DEFAULT_LOGGING_BAUD                    57600

// MSP -------------------------------------------

#define MSP_SET_NAME                            11

// RotorHazard -----------------------------------

#define DEFAULT_RH_SOCKET_IO_PATH               "/socket.io/?EIO=4"
#define DEFAULT_RH_PORT                         5000

// WiFi -------------------------------------------------

#define WIFI_CONNECTION_ATTEMPT_DURATION_S      30
#define DEFAULT_HOSTNAME                        "rotorhazard-osd-injector"

// HOTSPOT ----------------------------------------------

#define DEFAULT_HOTSPOT_SSID                    "rotorhazard-osd-injector"
#define DEFAULT_HOTSPOT_PASSWORD                "rotorhazard"