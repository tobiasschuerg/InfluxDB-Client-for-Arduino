
#include "Arduino.h"
#include <ESP8266HTTPClient.h>
#include "InfluxData.h"

class Influxdb
{
public:
  Influxdb(const char *host, uint16_t port);

  void setDb(const char *db);

  boolean post(InfluxData data);
  boolean post(String data);

private:
  String _port;
  String _host;
  String _db;
};