#ifndef _ARDUINO_HTTP_SERVICE_H_
#define _ARDUINO_HTTP_SERVICE_H_
#include "config.h"
#if defined(INFLUXDB_CLIENT_NET_WIFININA) || defined(INFLUXDB_CLIENT_NET_EXTERNAL)
#include "ArduinoHttpClient.h"
#include "HTTPService.h"

class ArduinoHTTPService : public HTTPService {
protected:
  
public:
  ArduinoHTTPService(ConnectionInfo *pConnInfo);
  virtual ~ArduinoHTTPService();
protected:
  virtual bool initRequest(const char *url, const char *method) override;
  void readHeaders();
  int completeRequest();
public: 
  virtual bool init() override;

  virtual void setHTTPOptions(const HTTPOptions &httpOptions) override;

  virtual int POST(const char *data) override;

  virtual int POST(Stream *stream) override;

  virtual int GET() override;

  virtual int DELETE() override;

  virtual void setHeader(const String &name, const String &value) override;
  
  virtual String getHeader(const char *name) override;

  virtual bool isConnected() const override { return _pHttpClient->connected(); } 

  virtual int getSize() override { return _pHttpClient->contentLength(); }

  virtual String getString() override {return _pHttpClient->responseBody();}

  virtual Stream *getStreamPtr() override { return _pClient?_pClient:_pConnInfo->tcpClient; }

  virtual String errorToString(int error) override;

  virtual bool isChunked() const override { return _pHttpClient->isResponseChunked(); }


  virtual void end() override;
protected:
  HttpClient *_pHttpClient = nullptr; 
  // External or own TCP client (WiFiClient in case of WiFiNINA)
  // When external, is coppied from ConnectionInfo
  Client *_pClient = nullptr;
  // Header value found during processing response
  String _retryAfter;
  // Need to keep host for HttpClient
  char *_pHost = nullptr;
};

#endif //defined(INFLUXDB_CLIENT_NET_WIFININA) || defined(INFLUXDB_CLIENT_NET_EXTERNAL)
#endif //_ARDUINO_HTTP_SERVICE_H_
