#include "utils.hpp"

String ipToString(IPAddress ip) {
    return String(ip[0]) + String(".") +\
            String(ip[1]) + String(".") +\
            String(ip[2]) + String(".") +\
            String(ip[3]); 
}