#include "Arduino.h"
#include "InfluxDb.h"

Influxdb::Influxdb(String host, uint16_t port)
{
  _port = port;
  _host = host;
}

void Influxdb::setDb(String db)
{
  _db = String(db);
}

// TODO: set db with user

/**
 * Send a single measurement to the InfluxDb.
 **/
boolean Influxdb::post(InfluxData data)
{
  return post(data.toString());
}

boolean Influxdb::post(String data)
{
  Serial.print("write ");
  Serial.println(data);

  // TODO: check if the client can be reused
  HTTPClient http;
  http.begin(_host, _port, "/write?db=" + _db);
  http.addHeader("Content-Type", "text/plain");
  //

  int httpResponseCode = http.POST(data);
  Serial.print(" -> Response: ");
  Serial.print(httpResponseCode);

  String response = http.getString();
  Serial.println(" -> \"" + response + "\"");

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
