/**
 * 
 * HTTPService.h: HTTP Service
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
#ifndef _HTTP_SERVICE_H_
#define _HTTP_SERVICE_H_

#include <Arduino.h>
#if defined(ESP8266)
# include <WiFiClientSecureBearSSL.h>
# include <ESP8266HTTPClient.h>
#elif defined(ESP32)
# include <HTTPClient.h>
# include <WiFiClient.h>
# include <WiFiClientSecure.h>
#else
# error "This library currently supports only ESP8266 and ESP32."
#endif
#include "Options.h"


class Test;
typedef std::function<bool(HTTPClient *client)> httpResponseCallback;
extern const char *TransferEncoding;

struct ConnectionInfo {
    // Connection info
    String serverUrl;
    // Write & query targets
    String bucket;
    String org;
    // v2 authetication token
    String authToken;
    // Version of InfluxDB 1 or 2
    uint8_t dbVersion;
    // V1 user authetication
    String user;
    String password;
    // Certificate info
    const char *certInfo; 
    // flag if https should ignore cert validation
    bool insecure;
    // Error message of last failed operation
    String lastError;
    // HTTP options
    HTTPOptions httpOptions;
};

/**
 * HTTPService provides  HTTP methods for communicating with InfluxDBServer,
 * while taking care of Authorization and error handling
 **/
class HTTPService {
friend class Test;  
  private:
    // Connection info data
    ConnectionInfo *_pConnInfo;    
    // Server API URL
    String _apiURL;
    // Last time in ms we made are a request to server
    uint32_t _lastRequestTime = 0;
    // HTTP status code of last request to server
    int _lastStatusCode = 0;
    // Underlying HTTPClient instance 
    HTTPClient *_httpClient = nullptr;
    // Underlying connection object 
    WiFiClient *_wifiClient = nullptr;
#ifdef  ESP8266
    // Trusted cert chain
    BearSSL::X509List *_cert = nullptr;   
#endif
    // Store retry timeout suggested by server after last request
    int _lastRetryAfter = 0;     
   
protected:
    // Sets request params
    bool beforeRequest(const char *url);
    // Handles response
    bool afterRequest(int expectedStatusCode, httpResponseCallback cb, bool modifyLastConnStatus = true);
public: 
    // Creates HTTPService instance
    // serverUrl - url of the InfluxDB 2 server (e.g. http://localhost:8086)
    // authToken - InfluxDB 2 authorization token 
    // certInfo - InfluxDB 2 server trusted certificate (or CA certificate) or certificate SHA1 fingerprint. Should be stored in PROGMEM.
    HTTPService(ConnectionInfo *pConnInfo);
    // Clean instance on deletion
    ~HTTPService();
    // Propagates http options to http client.
    void setHTTPOptions();
    // Returns current HTTPOption
    HTTPOptions &getHTTPOptions() { return _pConnInfo->httpOptions; }
    // Performs HTTP POST by sending data. On success calls response call back  
    bool doPOST(const char *url, const char *data, const char *contentType, int expectedCode, httpResponseCallback cb);
    // Performs HTTP POST by sending stream. On success calls response call back  
    bool doPOST(const char *url, Stream *stream, const char *contentType, int expectedCode, httpResponseCallback cb);
    // Performs HTTP GET. On success calls response call back    
    bool doGET(const char *url, int expectedCode, httpResponseCallback cb);
    // Performs HTTP DELETE. On success calls response call back    
    bool doDELETE(const char *url, int expectedCode, httpResponseCallback cb);
    // Returns InfluxDBServer API URL
    String getServerAPIURL() const { return _apiURL; }
    // Returns value of the Retry-After HTTP header from recent call. 0 if it was missing.
    int getLastRetryAfter() const { return _lastRetryAfter; }
    // Returns HTTP status code of recent call.
    int getLastStatusCode() const { return  _lastStatusCode;  }
    // Returns time of recent call successful call.
    uint32_t getLastRequestTime() const { return _lastRequestTime; }
    // Returns response of last failed call.
    String getLastErrorMessage() const { return _pConnInfo->lastError; }
    // Returns true if HTTP connection is kept open
    bool isConnected() const { return _httpClient && _httpClient->connected(); }
};

#endif //_HTTP_SERVICE_H_

