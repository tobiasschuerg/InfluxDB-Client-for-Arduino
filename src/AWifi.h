#include "config.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(INFLUXDB_CLIENT_NET_WIFININA)
#include <WiFiNINA.h>
#endif