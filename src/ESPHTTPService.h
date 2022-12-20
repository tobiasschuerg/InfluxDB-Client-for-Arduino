#ifndef _ESP_HTTP_SERVICE_H_
#define _ESP_HTTP_SERVICE_H_
#include "config.h"
#ifdef INFLUXDB_CLIENT_NET_ESP

#include "AHttpClient.h"
#include "AWifi.h"
#if defined(ESP8266)
# include <WiFiClientSecureBearSSL.h>
#endif
#include "HTTPService.h"

class ESPHTTPService : public HTTPService {
  protected:
    // Underlying HTTPClient instance 
    HTTPClient *_httpClient = nullptr;
    // Underlying connection object 
    WiFiClient *_wifiClient = nullptr;
  #ifdef  ESP8266
    // Trusted cert chain
    BearSSL::X509List *_cert = nullptr; 
  #endif
  protected:
    virtual bool initRequest(const char *url, const char *method) override;
  public: 
    ESPHTTPService(ConnectionInfo *pConnInfo);

    virtual ~ESPHTTPService();

    virtual bool init() override;
    
    virtual void setHTTPOptions(const HTTPOptions &httpOptions) override;

    virtual int POST(const char *data) override;

    virtual int POST(Stream *stream) override;

    virtual int GET() override;

    virtual int DELETE() override;

    virtual void setHeader(const String &name, const String &value) override;
    
    virtual String getHeader(const char *name) override;

    virtual bool isConnected() const override { return _httpClient && _httpClient->connected(); }

    virtual int getSize() override { return _httpClient->getSize(); }

    virtual String getString() override {  return _httpClient->getString(); }

    virtual Stream *getStreamPtr() override  { return _httpClient->getStreamPtr(); }

    virtual String errorToString(int error) override { return HTTPClient::errorToString(error); }

    virtual void end() override { _httpClient->end(); }
};

#endif //INFLUXDB_CLIENT_NET_ESP
#endif //_ESP_HTTP_SERVICE_H_
