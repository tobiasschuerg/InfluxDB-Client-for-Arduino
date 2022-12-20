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
#include <Client.h>

#include "Options.h"


class Test;
class HTTPService;

#ifndef ARDUINO_ARCH_AVR
#include <functional>
#endif

typedef std::function<bool(HTTPService *client)> httpResponseCallback;
extern const char *TransferEncoding;
extern const char *RetryAfter;
extern const char *UserAgent;

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
    // Underlying client
    Client *tcpClient;
};

/**
 * HTTPService provides  HTTP methods for communicating with InfluxDBServer,
 * while taking care of Authorization and error handling
 **/
class HTTPService {
  friend class Test;  
  protected:
    // Connection info data
    ConnectionInfo *_pConnInfo;    
    // Server API URL
    String _apiURL;
    // Last time in ms we made are a request to server
    uint32_t _lastRequestTime = 0;
    // HTTP status code of last request to server
    int _lastStatusCode = 0;
    // Store retry timeout suggested by server after last request
    int _lastRetryAfter = 0;     
    // was last call chunked
    bool _isChunked = false; 
     // HTTP options
    HTTPOptions _httpOptions;
  protected:
    // Initilises HTTP request and sets common request params, such as Authentication
    virtual bool beforeRequest(const char *url, const char *method, const char *contentType);
    // Handles response status, parses common headers, such as Retry-After
    // When modifyLastConnStatus is true, this method must set lastRequestTime and 
    virtual bool afterRequest(int expectedStatusCode, httpResponseCallback cb, bool modifyLastConnStatus = true);
  public: 
    // Creates HTTPService instance
    // serverUrl - url of the InfluxDB 2 server (e.g. http://localhost:8086)
    // authToken - InfluxDB 2 authorization token 
    // certInfo - InfluxDB 2 server trusted certificate (or CA certificate) or certificate SHA1 fingerprint. Should be stored in PROGMEM.
    HTTPService(ConnectionInfo *pConnInfo);
    // Clean instance on deletion
    virtual ~HTTPService() {};
    // Sets custom HTTP options. See HTTPOptions doc for more info. 
    // Must be called before calling any method initiating a connection to server.
    // When overriging to apply setting to an HTTP client, always call ancestor function
    // Example: 
    //    service.setHTTPOptions(HTTPOptions().httpReadTimeout(20000)).
    virtual void setHTTPOptions(const HTTPOptions &httpOptions);
    // Returns current HTTPOption
    virtual HTTPOptions &getHTTPOptions() { return _httpOptions; }
      // Returns InfluxDBServer API URL
    virtual String getServerAPIURL() const { return _apiURL; }
    // Returns value of the Retry-After HTTP header from recent call. 0 if it was missing.
    virtual int getLastRetryAfter() const { return _lastRetryAfter; }
    // Returns HTTP status code of recent call.
    virtual int getLastStatusCode() const { return  _lastStatusCode;  }
    // Returns time of recent call successful call.
    virtual uint32_t getLastRequestTime() const { return _lastRequestTime; }
    // Returns response of last failed call.
    virtual String getLastErrorMessage() const { return _pConnInfo->lastError; }
    // Returns true if last HTTP call returned chunked response
    virtual bool isChunked() const { return _isChunked; }
    // Handles HTTP POST by sending data. On success calls response call back  
    virtual bool doPOST(const char *url, const char *data, const char *contentType, int expectedCode, httpResponseCallback cb);
    // Handles HTTP POST by sending stream. On success calls response call back  
    virtual bool doPOST(const char *url, Stream *stream, const char *contentType, int expectedCode, httpResponseCallback cb);
    // Handles HTTP GET. On success calls response call back    
    virtual bool doGET(const char *url, int expectedCode, httpResponseCallback cb);
    // Handles HTTP DELETE. On success calls response call back    
    virtual bool doDELETE(const char *url, int expectedCode, httpResponseCallback cb);

    // -----------------------------------
    // Functions to override on implementations
    //-------------------------------------
  protected:
    // Initilises HTTP request 
    virtual bool initRequest(const char *url, const char *method) = 0;
  public:
    // Initialize service
    virtual bool init() = 0;
    // Performs HTTP POST by sending data.
    virtual int POST(const char *data) = 0;
    // Performs HTTP POST by sending stream. 
    virtual int POST(Stream *stream) = 0;
    // Performs HTTP GET.
    virtual int GET() = 0;
    // Performs HTTP DELETE.
    virtual int DELETE() = 0;
    // Set HTTP header to request
    virtual void setHeader(const String &name, const String &value) = 0;
    // Get HTTP response header value
    virtual String getHeader(const char *name) = 0;
    // Returns true if HTTP connection is kept open
    virtual bool isConnected() const = 0;
    // Returns complete response as a string 
    virtual String getString()  = 0;
    // Returns pointer to underlying TCP client implementing Stream to read response
    // Stream must be at the state after reading out headers 
    virtual Stream *getStreamPtr() = 0;
    // Returns size of the current response if Content-Lenght headers was set, otherwise -1
    virtual int getSize() = 0;
    // End HTTP call
    virtual void end() = 0;
    // Returns string representation of error code
    virtual String errorToString(int error) = 0;;
};

#endif //_HTTP_SERVICE_H_
