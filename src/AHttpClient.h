#include "config.h"
#if defined(ESP8266)
# include <ESP8266HTTPClient.h>
#elif defined(ESP32)
# include <HTTPClient.h>
#elif defined(INFLUXDB_CLIENT_NET_WIFININA) || defined(INFLUXDB_CLIENT_NET_EXTERNAL)
#include <ArduinoHttpClient.h>
#endif