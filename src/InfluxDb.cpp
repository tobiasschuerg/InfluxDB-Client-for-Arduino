/**
 * 
 * InfluxDb.cpp: InfluxDB Client for Arduino
 * 
 * MIT License
 * 
 * Copyright (c) 2018-2020 Tobias Schürg
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#include "InfluxDb.h"
#include "Arduino.h"

/**
 * Construct an InfluxDb instance.
 * @param host the InfluxDb host
 * @param port the InfluxDb port
 */
Influxdb::Influxdb(String host, uint16_t port) {
  if(port == 443) {
    // this happens usualy when influxdb is behind fw/proxy. Mostly, when influxdb is switched to https, the port remains the same (8086)
    // port number shouldn't be qualificator for secure connection, either scheme or a flag
    _connInfo.serverUrl = "https://";
  } else {
    _connInfo.serverUrl = "http://";
  }
  _connInfo.serverUrl += host + ":" + String(port);
  _connInfo.dbVersion = 1;
}

/**
 * Set the database to be used.
 * @param db the Influx Database to be written to.
 */
void Influxdb::setDb(String db) {
  _connInfo.bucket = db;
}

/**
 * Set the database to be used with authentication.
 */
void Influxdb::setDbAuth(String db, String user, String pass) {
  _connInfo.bucket = db;
  _connInfo.user = user;
  _connInfo.password = pass;
}

/**
 * Set the Bucket to be used v2.0 ONLY.
 * @param bucket the InfluxDB Bucket which must already exist
 */
void Influxdb::setBucket(String bucket) {
  _connInfo.bucket = bucket;
}

/**
 * Set the influxDB port.
 * @param port both v1.x and v3 use 8086
 */
void Influxdb::setPort(uint16_t port){
  int b = _connInfo.serverUrl.indexOf(":",5);
  if(b > 0) {
    _connInfo.serverUrl = _connInfo.serverUrl.substring(0, b+1) + String(port);
  }
}
/**
 * Set the Organization to be used v2.0 ONLY
 * @param org the Name of the organization unit to use which must already exist
 */
void Influxdb::setOrg(String org){
  _connInfo.org = org;
}

/**
 * Set the authorization token v2.0 ONLY
 * @param token the Auth Token from InfluxDBv2 *required*
 */
void Influxdb::setToken(String token){
  _connInfo.authToken = token;
}

/**
 * Set the version of InfluxDB to write to
 * @param version accepts 1 for version 1.x or 2 for version 2.x
 */
void Influxdb::setVersion(uint16_t version){
  _connInfo.dbVersion = version;
}

#if defined(ESP8266)
/**
 * Set server certificate finger print 
 * @param fingerPrint server certificate finger print 
 */
void Influxdb::setFingerPrint(const char *fingerPrint){
  _connInfo.certInfo = fingerPrint;
}
#endif

void Influxdb::begin() {
 
}

/**
 * Prepare a measurement to be sent.
 */
void Influxdb::prepare(InfluxData data) { 
  ++_preparedPoints;
  if(_writeOptions._batchSize <= _preparedPoints) {
    // for preparation, batchsize must be greater than number of prepared points, or it will send data right away
    _writeOptions._batchSize = _preparedPoints+1;
    reserveBuffer(2*_writeOptions._batchSize);
  }
  write(data);
}

/**
 * Write all prepared measurements into the db.
 */
boolean Influxdb::write() {
  _preparedPoints = 0;
  return flushBuffer();
}

/**
 * Write a single measurement into the db.
 */
boolean Influxdb::write(InfluxData data) { 
    return write(pointToLineProtocol(data));
}

/**
 * Send raw data to InfluxDb.
 *
 * @see
 * https://github.com/esp8266/Arduino/blob/cc0bfa04d401810ed3f5d7d01be6e88b9011997f/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L44-L55
 * for a list of error codes.
 */
boolean Influxdb::write(String data) {
  return writeRecord(data);
}
