#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <core_version.h>

#define STRHELPER(x) #x
#define STR(x) STRHELPER(x) // stringifier

#if defined(ESP8266)
# define INFLUXDB_CLIENT_PLATFORM "ESP8266"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP8266_GIT_DESC)
#elif defined(ESP32)
# define INFLUXDB_CLIENT_PLATFORM "ESP32"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP32_GIT_DESC)
#endif

#endif //_PLATFORM_H_
