
#include "Arduino.h"
#include <ESP8266HTTPClient.h>
#include "InfluxData.h"

class Influxdb
{
public:
  Influxdb(String host, uint16_t port);

  void setDb(String db);

  boolean post(InfluxData data);
  boolean post(String data);

private:
  String _host;
  uint16_t _port;
  String _db;
};