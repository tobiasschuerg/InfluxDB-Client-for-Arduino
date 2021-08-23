#ifndef _HTTP_SERVICE_H_
#define _HTTP_SERVICE_H_

#include <Arduino.h>
#if defined(ESP8266)
# include <WiFiClientSecureBearSSL.h>
# include <ESP8266HTTPClient.h>
#elif defined(ESP32)
# include <HTTPClient.h>
#else
# error "This library currently supports only ESP8266 and ESP32."
#endif
#include "Options.h"

class Test;

typedef std::function<bool(HTTPClient *client)> httpResponseCallback;
extern const char *TransferEncoding;

class HTTPService {
friend class Test;  
  private:    
    // Server API URL
    String _apiURL;
    // authetication token
    String _authToken;
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
#ifdef  ESP8266
    BearSSL::X509List *_cert = nullptr;   
#endif
    // Store retry timeout suggested by server after last request
    int _lastRetryAfter = 0;     
     // HTTP options
    HTTPOptions _httpOptions;
protected:
    // Sets request params
    bool beforeRequest(const char *url);
    // Handles response
    bool afterRequest(int expectedStatusCode, httpResponseCallback cb, bool modifyLastConnStatus = true);
public:  
    HTTPService(const String &serverUrl, const String &authToken, const char *certInfo, bool insecure);
    ~HTTPService();
    // Sets custom HTTP options. See HTTPOptions doc for more info. 
    // Must be called before calling any method initiating a connection to server.
    // Example: 
    //    service.setHTTPOptions(HTTPOptions().httpReadTimeout(20000)).
    void setHTTPOptions(const HTTPOptions &httpOptions);
    HTTPOptions &getHTTPOptions() { return _httpOptions; }
    // Performs HTTP POST by sending data. On success calls response call back  
    bool doPOST(const char *url, const char *data, const char *contentType, int expectedCode, httpResponseCallback cb);
    // Performs HTTP GET. On success calls response call back    
    bool doGET(const char *url, int expectedCode, httpResponseCallback cb);

    String getServerAPIURL() const { return _apiURL; }
    int getLastRetryAfter() const { return _lastRetryAfter; }
    int getLastStatusCode() const { return  _lastStatusCode;  }
    uint32_t getLastRequestTime() const { return _lastRequestTime; }
    // Returns last response when operation failed
    String getLastErrorMessage() const { return _lastErrorResponse; }
};

#endif //_HTTP_SERVICE_H_