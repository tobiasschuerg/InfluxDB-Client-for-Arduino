/**
 * 
 * InfluxDBClient.h: InfluxDB Client for Arduino
 * 
 * MIT License
 * 
 * Copyright (c) 2020 InfluxData
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
#ifndef _INFLUXDB_CLIENT_H_
#define _INFLUXDB_CLIENT_H_

#define INFLUXDB_CLIENT_VERSION "3.6.1"

#include <Arduino.h>
#if defined(ESP8266)
# include <WiFiClientSecureBearSSL.h>
# include <ESP8266HTTPClient.h>
#elif defined(ESP32)
# include <HTTPClient.h>
#else
# error "This library currently supports only ESP8266 and ESP32."
#endif

#include "Point.h"
#include "WritePrecision.h"
#include "query/FluxParser.h"
#include "util/helpers.h"
#include "Options.h"


#ifdef USING_AXTLS
#error AxTLS does not work
#endif

class Test;

/**
 * InfluxDBClient handles connection and basic operations for an InfluxDB server.
 * It provides write API with ability to write data in batches and retrying failed writes.
 * Automaticaly retries failed writes during next write, if server is overloaded.
 */
class InfluxDBClient {
  public:
    // Creates InfluxDBClient unconfigured instance. 
    // Call to setConnectionParams is required to set up client 
    InfluxDBClient();
    // Creates InfluxDBClient instance for unsecured connection to InfluxDB 1
    // serverUrl - url of the InfluxDB 1 server (e.g. http://localhost:8086)
    // db - database name where to store or read data
    InfluxDBClient(const char *serverUrl, const char *db);
    // Creates InfluxDBClient instance for unsecured connection
    // serverUrl - url of the InfluxDB 2 server (e.g. http://localhost:8086)
    // org - name of the organization, which bucket belongs to    
    // bucket - name of the bucket to write data into
    // authToken - InfluxDB 2 authorization token
    InfluxDBClient(const char *serverUrl, const char *org, const char *bucket, const char *authToken);
    // Creates InfluxDBClient instance for secured connection
    // serverUrl - url of the InfluxDB 2 server (e.g. https://localhost:8086)
    // org - name of the organization, which bucket belongs to 
    // bucket - name of the bucket to write data into
    // authToken - InfluxDB 2 authorization token
    // certInfo - InfluxDB 2 server trusted certificate (or CA certificate) or certificate SHA1 fingerprint. Should be stored in PROGMEM.
    InfluxDBClient(const char *serverUrl, const char *org, const char *bucket, const char *authToken, const char *certInfo);
    // Clears instance.
    ~InfluxDBClient();
    // Allows insecure connection. setInsecure must be called before calling any method initiating a connection to server.
    // Works only on ESP8266. ESP32 allows unsecured connections by default (status for latest 1.0.4 ESP32 Arduino SDK).
    void setInsecure(bool value);
    // precision - timestamp precision of written data
    // batchSize - number of points that will be written to the databases at once. Default 1 - writes immediately
    // bufferSize - maximum size of Points buffer. Buffer contains new data that will be written to the database
    //             and also data that failed to be written due to network failure or server overloading
    // flushInterval - maximum number of seconds data will be held in buffer before are written to the db. 
    //                 Data are written either when number of points in buffer reaches batchSize or time of  
    // preserveConnection - true if HTTP connection should be kept open. Usable for frequent writes.
    void setWriteOptions(WritePrecision precision, uint16_t batchSize = 1, uint16_t bufferSize = 5, uint16_t flushInterval = 60, bool preserveConnection = true) __attribute__ ((deprecated("Use setWriteOptions(const WriteOptions &writeOptions)")));
    // Sets custom write options. See WriteOptions doc for more info. 
    // Must be called before calling any method initiating a connection to server.
    // Example: 
    //    client.setWriteOptions(WriteOptions().batchSize(10).bufferSize(50)).
    void setWriteOptions(const WriteOptions &writeOptions);
    // Sets custom HTTP options. See HTTPOptions doc for more info. 
    // Must be called before calling any method initiating a connection to server.
    // Example: 
    //    client.setHTTPOptions(HTTPOptions().httpReadTimeout(20000)).
    void setHTTPOptions(const HTTPOptions &httpOptions);
    // Sets connection parameters for InfluxDB 2
    // serverUrl - url of the InfluxDB 2 server (e.g. https//localhost:8086)
    // org - name of the organization, which bucket belongs to 
    // bucket - name of the bucket to write data into
    // authToken - InfluxDB 2 authorization token
    // serverCert - Optional. InfluxDB 2 server trusted certificate (or CA certificate) or certificate SHA1 fingerprint.  Should be stored in PROGMEM. Only in case of https connection.
    void setConnectionParams(const char *serverUrl, const char *org, const char *bucket, const char *authToken, const char *certInfo = nullptr);
    // Sets parameters for connection to InfluxDB 1
    // serverUrl - url of the InfluxDB server (e.g. http://localhost:8086)
    // db - database name where to store or read data
    // user - Optional. User name, in case of server requires authetication
    // password - Optional. User password, in case of server requires authetication
    // certInfo - Optional. InfluxDB server trusted certificate (or CA certificate) or certificate SHA1 fingerprint.  Should be stored in PROGMEM. Only in case of https connection.
    void setConnectionParamsV1(const char *serverUrl, const char *db, const char *user = nullptr, const char *password = nullptr, const char *certInfo = nullptr);
    // Creates line protocol string from point data and optional default tags set in WriteOptions.
    String pointToLineProtocol(const Point& point);
    // Validates connection parameters by conecting to server
    // Returns true if successful, false in case of any error
    bool validateConnection();
    // Writes record in InfluxDB line protocol format to write buffer.
    // Returns true if successful, false in case of any error 
    bool writeRecord(String &record);
    // Writes record represented by Point to buffer
    // Returns true if successful, false in case of any error 
    bool writePoint(Point& point);
    // Sends Flux query and returns FluxQueryResult object for subsequently reading flux query response.
    // Use FluxQueryResult::next() method to iterate over lines of the query result.
    // Always call of FluxQueryResult::close() when reading is finished. Check FluxQueryResult doc for more info.
    FluxQueryResult query(String fluxQuery);
    // Forces writing of all points in buffer, even the batch is not full.
    // Returns true if successful, false in case of any error 
    bool flushBuffer();
    // Returns true if points buffer is full. Usefull when server is overloaded and we may want increase period of write points or decrease number of points
    bool isBufferFull() const  { return _bufferCeiling == _writeBufferSize; };
    // Returns true if buffer is empty. Usefull when going to sleep and check if there is sth in write buffer (it can happens when batch size if bigger than 1). Call flushBuffer() then.
    bool isBufferEmpty() const { return _bufferCeiling == 0 && !_writeBuffer[0]; };
    // Checks points buffer status and flushes if number of points reached batch size or flush interval runs out.
    // Returns true if successful, false in case of any error
    bool checkBuffer();
    // Wipes out buffered points
    void resetBuffer();
    // Returns HTTP status of last request to server. Usefull for advanced handling of failures.
    int getLastStatusCode() const { return _lastStatusCode;  }
    // Returns last response when operation failed
    String getLastErrorMessage() const { return _lastErrorResponse; }
    // Returns server url
    String getServerUrl() const { return _serverUrl; }
    // Check if it is possible to send write/query request to server. 
    // Returns true if write or query can be send, or false, if server is overloaded and retry strategy is applied.
    // Use getRemainingRetryTime() to get wait time in such case.
    bool canSendRequest() { return getRemainingRetryTime() == 0; }
    // Returns remaining wait time in seconds when retry strategy is applied.
    uint32_t getRemainingRetryTime();
  protected:
    // Checks params and sets up security, if needed.
    // Returns true in case of success, otherwise false
    bool init();
    // Sets request params
    void beforeRequest();
    // Handles response
    void afterRequest(int expectedStatusCode, bool modifyLastConnStatus = true);
    // Cleans instances
    void clean();
  protected:
    class Batch {
      private:
        uint8_t _size = 0;
      public:
        uint8_t pointer = 0;
        String *buffer = nullptr;
        uint8_t retryCount = 0;
        Batch(int size):_size(size) {  buffer = new String[size]; }
        ~Batch() { delete [] buffer; }
        bool append(String &line);
        char *createData();
        bool isFull() const {
          return pointer == _size;
        }
    };
  friend class Test;
    // Connection info
    String _serverUrl;
    String _bucket;
    String _org;
    // token authetication
    String _authToken;
    // V1 user authetication
    String _user;
    String _password;
    // Cached full write url
    String _writeUrl;
    // Cached full query url
    String _queryUrl;
    // Points buffer
    Batch **_writeBuffer = nullptr;
    // Batch buffer size
    uint8_t _writeBufferSize;
    // Write options
    WriteOptions _writeOptions;
    // HTTP options
    HTTPOptions _httpOptions;
    // Index to buffer where to store new batch
    uint8_t _bufferPointer = 0;
    // Actual count of batches in buffer 
    uint8_t _bufferCeiling = 0;
    // Index of bath start for next write
    uint8_t _batchPointer = 0;
    // Last time in sec buffer has been successfully flushed
    uint32_t _lastFlushed = 0;
    // Last time in ms we made are a request to server
    uint32_t _lastRequestTime = 0;
    // HTTP status code of last request to server
    int _lastStatusCode = 0;
    // Server reponse or library error message for last failed request
    String _lastErrorResponse;
    // Underlying HTTPClient instance 
    HTTPClient *_httpClient = nullptr;
    // Underlying connection object 
    WiFiClient *_wifiClient = nullptr;
    // Certificate info
    const char *_certInfo = nullptr;
    // Version of InfluxDB 1 or 2
    uint8_t _dbVersion = 2;
#ifdef  ESP8266
  BearSSL::X509List *_cert = nullptr;   
#endif
    // if true - allow insecure connection
    bool _insecure = 0;
    // Store retry timeout suggested by server after last request
    int _lastRetryAfter = 0;
    // Sends POST request with data in body
    int postData(const char *data);
    // Sets cached InfluxDB server API URLs
    void setUrls();
    // Ensures buffer has required size
    void reserveBuffer(int size);
    // Drops current batch and advances batch pointer
    void dropCurrentBatch();
    // Writes all points in buffer, with respect to the batch size, and in case of success clears the buffer.
    //  flashOnlyFull - whether to flush only full batches
    // Returns true if successful, false in case of any error 
    bool flushBufferInternal(bool flashOnlyFull);
};


#endif //_INFLUXDB_CLIENT_H_
