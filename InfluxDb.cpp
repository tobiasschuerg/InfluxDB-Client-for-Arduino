#include "InfluxDb.h"
#include "Arduino.h"

/**
 * Construct an InfluxDb instance.
 * @param host the InfluxDb host
 * @param port the InfluxDb port
 */
Influxdb::Influxdb(String host, uint16_t port) {
  _port = port;
  _host = host;
}

/**
 * Set the database to be used.
 */
void Influxdb::setDb(String db) {
  _db = String(db);
  // TODO: recreate client on db change
  // http = new HTTPClient();
  http.begin(_host, _port, "/write?db=" + _db);
  http.addHeader("Content-Type", "text/plain");
}

// TODO: set db with user & password

void Influxdb::prepare(InfluxData data) { prepared.push_back(data); }

boolean Influxdb::post() {
  String data = "";
  for (auto const& i : prepared) {
    data = (data == "") ? (i.toString()) : (data + "\n" + i.toString());
  }
  return post(data);
}

/**
 * Send a single measurement to the InfluxDb.
 */
boolean Influxdb::post(InfluxData data) { return post(data.toString()); }

/**
 * Send raw data to InfluxDb.
 */
boolean Influxdb::post(String data) {
  Serial.print(" -> writing to " + _db + ": ");
  Serial.println(data);

  int httpResponseCode = http.POST(data);
  Serial.print(" <- Response: ");
  Serial.print(httpResponseCode);

  String response = http.getString();
  Serial.println(" \"" + response + "\"");

  boolean success;
  if (httpResponseCode == 204) {
    success = true;
  } else {
    Serial.println("#####\nPOST FAILED\n#####");
    success = false;
  }

  http.end();
  return success;
}
