
#include "Arduino.h"
#include <ESP8266HTTPClient.h>
#include <list>

#include "InfluxData.h"

class Influxdb
{
public:
  Influxdb(String host, uint16_t port = 8086);

  void setDb(String db);

  void prepare(InfluxData data);
  boolean post();

  boolean post(InfluxData data);
  boolean post(String data);

private:
  HTTPClient http;
  String _host;
  uint16_t _port;
  String _db;
  std::list<InfluxData> prepared;
};