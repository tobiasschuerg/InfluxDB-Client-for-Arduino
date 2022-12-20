#include "config.h"
#ifdef INFLUXDB_CLIENT_NET_ESP
#include "ESPHTTPService.h"
#include "util/debug.h"

#if defined(ESP8266)         
bool checkMFLN(BearSSL::WiFiClientSecure  *client, String url);
#endif

ESPHTTPService::ESPHTTPService(ConnectionInfo *pConnInfo):HTTPService(pConnInfo) {

};

bool ESPHTTPService::init() {
  bool https = _pConnInfo->serverUrl.startsWith("https");
  if(https) {
#if defined(ESP8266)         
    BearSSL::WiFiClientSecure *wifiClientSec = new BearSSL::WiFiClientSecure;
    if (_pConnInfo->insecure) {
      wifiClientSec->setInsecure();
    } else if(_pConnInfo->certInfo && strlen_P(_pConnInfo->certInfo) > 0) {
      if(strlen_P(_pConnInfo->certInfo) > 60 ) { //differentiate fingerprint and cert
         _cert = new BearSSL::X509List(_pConnInfo->certInfo); 
         wifiClientSec->setTrustAnchors(_cert);
      } else {
         wifiClientSec->setFingerprint(_pConnInfo->certInfo);
      }
    }
    checkMFLN(wifiClientSec, _pConnInfo->serverUrl);
#elif defined(ESP32)
    WiFiClientSecure *wifiClientSec = new WiFiClientSecure;  
    if (_pConnInfo->insecure) {
#ifndef ARDUINO_ESP32_RELEASE_1_0_4
      // This works only in ESP32 SDK 1.0.5 and higher
      wifiClientSec->setInsecure();
#endif            
    } else if(_pConnInfo->certInfo && strlen_P(_pConnInfo->certInfo) > 0) { 
      wifiClientSec->setCACert(_pConnInfo->certInfo);
    }
#endif    
    _wifiClient = wifiClientSec;
  } else {
    _wifiClient = new WiFiClient;
  }
  if(!_httpClient) {
    _httpClient = new HTTPClient;
  }
  _httpClient->setReuse(_httpOptions.isConnectionReuse());
  _httpClient->setUserAgent(FPSTR(UserAgent));
  return true;
}

ESPHTTPService::~ESPHTTPService() {
  if(_httpClient) {
    delete _httpClient;
    _httpClient = nullptr;
  }
  if(_wifiClient) {
    delete _wifiClient;
    _wifiClient = nullptr;
  }
#if defined(ESP8266)     
  if(_cert) {
    delete _cert;
    _cert = nullptr;
}
#endif
}


void ESPHTTPService::setHTTPOptions(const HTTPOptions & httpOptions) {
   HTTPService::setHTTPOptions(httpOptions);
    if(!_httpClient) {
        _httpClient = new HTTPClient;
    }
    _httpClient->setReuse(_httpOptions.isConnectionReuse());
    _httpClient->setTimeout(_httpOptions.getHttpReadTimeout());
#if defined(ESP32) 
     _httpClient->setConnectTimeout(_httpOptions.getHttpReadTimeout());
#endif
}

// parse URL for host and port and call probeMaxFragmentLength
#if defined(ESP8266)         
bool checkMFLN(BearSSL::WiFiClientSecure  *client, String url) {
    String host, path, user, pass;
    int port;
    if(!parseURL(url, host, port, path, user, pass)) {
      return false;
    }
    INFLUXDB_CLIENT_DEBUG("[D] probeMaxFragmentLength to %s:%d\n", host.c_str(), port);
    bool mfln = client->probeMaxFragmentLength(host, port, 1024);
    INFLUXDB_CLIENT_DEBUG("[D]  MFLN:%s\n", mfln ? "yes" : "no");
    if (mfln) {
        client->setBufferSizes(1024, 1024);
    } 
    return mfln;
}
#endif //ESP8266

bool ESPHTTPService::initRequest(const char *url, const char *method) {
  if(!_httpClient->begin(*_wifiClient, url)) {
    return false;
  }
  const char * headerKeys[] = {RetryAfter, TransferEncoding} ;
  _httpClient->collectHeaders(headerKeys, 2);
  return true;
}

void ESPHTTPService::setHeader(const String &name, const String &value) {
  _httpClient->addHeader(name, value);  
}

String ESPHTTPService::getHeader(const char *name) {
  return _httpClient->header(name);
}


int ESPHTTPService::POST(const char *data) {
  return _httpClient->POST((uint8_t *) data, strlen(data));
}

int ESPHTTPService::POST(Stream *stream) {
  return _httpClient->sendRequest("POST", stream, stream->available());
}

int ESPHTTPService::GET() {
  return _httpClient->GET();
}

int ESPHTTPService::DELETE() {
  return _httpClient->sendRequest("DELETE");
}

#endif //INFLUXDB_CLIENT_NET_ESP
