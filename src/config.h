#ifndef _CONFIG_H_
#define _CONFIG_H_

// Uncomment following to allow also external net providers, e.g. GSM connection
//# define INFLUXDB_CLIENT_NET_EXTERNAL
#if defined(ESP8266) || defined(ESP32) 
# define INFLUXDB_CLIENT_NET_ESP
# define  INFLUXDB_CLIENT_HAVE_WIFI
#elif defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT)
# define INFLUXDB_CLIENT_NET_WIFININA
# define  INFLUXDB_CLIENT_HAVE_WIFI
#elif defined(ARDUINO_ARCH_AVR)
# error "AVR devices are not supported. Only Atmel SAM or ESP"
#else
# if !defined(INFLUXDB_CLIENT_NET_EXTERNAL)
#    define INFLUXDB_CLIENT_NET_EXTERNAL
# endif
#endif


#endif //_CONFIG_H_