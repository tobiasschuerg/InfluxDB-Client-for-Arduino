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
 * @param db the Influx Database to be written to.
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

/**
 * Set the Bucket to be used v2.0 ONLY.
 * @param bucket the InfluxDB Bucket which must already exist
 */
void Influxdb::setBucket(String bucket) {
  _bucket = String(bucket);
  begin();
}

/**
 * Set the influxDB port.
 * @param port v1.x uses 8086, v2 uses 9999
 */
void Influxdb::setPort(uint16_t port){
  _port = port;
  begin();
}
/**
 * Set the Organization to be used v2.0 ONLY
 * @param org the Name of the organization unit to use which must already exist
 */
void Influxdb::setOrg(String org){
  _org = String(org);
  begin();
}

/**
 * Set the authorization token v2.0 ONLY
 * @param token the Auth Token from InfluxDBv2 *required*
 */
void Influxdb::setToken(String token){
  _token = String(token);
  begin();
}

/**
 * Set the version of InfluxDB to write to
 * @param version accepts 1 for version 1.x or 2 for version 2.x
 */
void Influxdb::setVersion(uint16_t version){
  _db_v = version;
  begin();
}

#if defined(ESP8266)
/**
 * Set servers finger print for HTTPS v2 Influx proto
 * @param version accepts 1 for version 1.x or 2 for version 2.x
 */
void Influxdb::setFingerPrint(char *fingerPrint){
  _fingerPrint = fingerPrint;
  begin();
}
#endif

void Influxdb::begin() {
  // TODO: recreate HttpClient on db change?
  if(_db_v == 2){
#if defined(ESP8266)
    if (_port == 443) {
        if (_fingerPrint)
          client.setFingerprint(_fingerPrint);
        http.begin(client, _host, _port, "/api/v2/write?org=" + _org + "&bucket=" + _bucket, true);
    }
    else
#endif
    {
        http.begin(_host, _port, "/api/v2/write?org=" + _org + "&bucket=" + _bucket);
    }
  } else {
    if (_user && _pass) {
      http.begin(_host, _port, "/write?u=" + _user + "&p=" + _pass + "&db=" + _db);
    } else {
      http.begin(_host, _port, "/write?db=" + _db);
    }
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
  if(_db_v == 2){
    if(_token == NULL || _token.length() < 10){
      Serial.println("#####\nInvalid Access Token\n#####");
      return false;
    }
    Serial.print(" --> writing to host: " + _host + " Port: " + _port + " URL: /api/v2/write?org=" + _org + "&bucket=" + _bucket + ":\n");
    Serial.println(data);
  } else {
    Serial.print(" --> writing to " + _db + ":\n");
    Serial.println(data);
  }
  if(_db_v == 2)
    http.addHeader("Authorization", "Token " + _token);
  int httpResponseCode = http.POST(data);
  Serial.print(" <-- Response: ");
  Serial.print(httpResponseCode);

#if defined(ESP32)
  // The ESP32 HTTP Lib seems to hang if you call getString if the server has not
  // written anything in response.
  if (http.getSize() > 0) {
    String response = http.getString();
    Serial.println(" \"" + response + "\"");
  }
  else {
    Serial.println();
  }
#else
  String response = http.getString();
  Serial.println(" \"" + response + "\"");
#endif

  boolean success;
  if (httpResponseCode == HTTP_CODE_NO_CONTENT) {
    success = true;
  } else {
    Serial.println("#####\nPOST FAILED\n#####");
    success = false;
  }

  http.end();
  return success;
}
