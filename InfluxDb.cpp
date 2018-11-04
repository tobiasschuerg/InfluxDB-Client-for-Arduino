/**
    ESP8266 InfluxDb: Influxdb.cpp

    Purpose: Helps with sending measurements to an Influx database.

    @author Tobias Schürg
*/
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
  begin();
}

/**
 * Set the database to be used with authentication.
 */
void Influxdb::setDbAuth(String db, String user, String pass) {
  _db = String(db);
  _user = user;
  _pass = pass;
  begin();
}

void Influxdb::begin() {
  // TODO: recreate HttpClient on db change?
  if (_user && _pass) {
    http.begin(_host, _port, "/write?u=" + _user + "&p=" + _pass + "&db=" + _db);
  } else {
    http.begin(_host, _port, "/write?db=" + _db);
  }
  http.addHeader("Content-Type", "text/plain");
}

/**
 * Prepare a measurement to be sent.
 */
void Influxdb::prepare(InfluxData data) { prepared.push_back(data); }

/**
 * Write all prepared measurements into the db.
 */
boolean Influxdb::write() {
  String data = "";
  for (auto const& i : prepared) {
    data = (data == "") ? (i.toString()) : (data + "\n" + i.toString());
  }
  prepared.clear();
  return write(data);
}

/**
 * Write a single measurement into the db.
 */
boolean Influxdb::write(InfluxData data) { return write(data.toString()); }

/**
 * Send raw data to InfluxDb.
 *
 * @see
 * https://github.com/esp8266/Arduino/blob/cc0bfa04d401810ed3f5d7d01be6e88b9011997f/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L44-L55
 * for a list of error codes.
 */
boolean Influxdb::write(String data) {
  Serial.print(" --> writing to " + _db + ":\n");
  Serial.println(data);

  int httpResponseCode = http.POST(data);
  Serial.print(" <-- Response: ");
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
