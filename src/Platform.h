#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "config.h"
#ifdef INFLUXDB_CLIENT_NET_ESP
#include <core_version.h>
#endif

#define STRHELPER(x) #x
#define STR(x) STRHELPER(x) // stringifier

#if defined(ESP8266)
# define INFLUXDB_CLIENT_PLATFORM "ESP8266"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP8266_GIT_DESC)
#elif defined(ESP32)
# define INFLUXDB_CLIENT_PLATFORM "ESP32"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP32_GIT_DESC)
#elif defined(ARDUINO_ARCH_SAMD)
# define INFLUXDB_CLIENT_PLATFORM "Atmel SAMD"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  "1.0" //TODO how to get it from a header?
#else
#error Define platorm info in Platform.h!
#endif

#endif //_PLATFORM_H_
