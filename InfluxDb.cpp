#include "Arduino.h"
#include "InfluxDb.h"

Influxdb::Influxdb(const char *host, uint16_t port)
{
  _port = String(port);
  _host = String(host);
}

void Influxdb::setDb(const char *db)
{
  _db = String(db);
}

boolean Influxdb::post(InfluxData data)
{
  return post(data.toString());
}

boolean Influxdb::post(String data)
{
  Serial.print("write ");
  Serial.println(data);

  HTTPClient http;
  http.begin("http://" + _host + ":" + _port + "/write?db=" + _db);
  http.addHeader("Content-Type", "text/plain");

  int httpResponseCode = http.POST(data);
  Serial.print(" -> Response code ");
  Serial.println(httpResponseCode);

  String response = http.getString();
  Serial.println(" -> Response: \"" + response + "\".");

  boolean success;
  if (httpResponseCode == 204)
  {

    success = true;
  }
  else
  {
    Serial.println("#####\nPOST FAILED\n#####");
    success = false;
  }

  http.end();
  return success;
}
