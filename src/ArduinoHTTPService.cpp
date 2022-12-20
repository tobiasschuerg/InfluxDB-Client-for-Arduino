#include "ArduinoHTTPService.h"
#if defined(INFLUXDB_CLIENT_NET_WIFININA) || defined(INFLUXDB_CLIENT_NET_EXTERNAL)

#ifdef INFLUXDB_CLIENT_NET_WIFININA
#include <WiFiNINA.h>
#endif //INFLUXDB_CLIENT_NET_WIFININA
#include "util/helpers.h"
#include "util/debug.h"

ArduinoHTTPService::ArduinoHTTPService(ConnectionInfo *pConnInfo):HTTPService(pConnInfo) {

};

ArduinoHTTPService::~ArduinoHTTPService() {
  if(_pHttpClient) {
    delete _pHttpClient;
  }
#ifdef INFLUXDB_CLIENT_NET_WIFININA
  if(_pClient) {
    delete _pClient;
  }
#endif 
  if(_pHost) {
    delete [] _pHost;
  }
};

void ArduinoHTTPService::setHTTPOptions(const HTTPOptions &httpOptions) { 
  HTTPService::setHTTPOptions(httpOptions); 
  //TODO: set options to httpclient
}

// Init 
bool ArduinoHTTPService::init() {
  String host, path, user, pass;
  int port;
  if(!parseURL(_pConnInfo->serverUrl, host, port, path, user, pass)) {
     _pConnInfo->lastError = F("Error parsing server URL");
    return false;
  }
  if(_pHost) {
    delete [] _pHost;
  }
  _pHost = cloneStr(host.c_str());
  bool https = _pConnInfo->serverUrl.startsWith("https");
#ifdef INFLUXDB_CLIENT_NET_WIFININA
  if(!_pConnInfo->tcpClient) {
    if(https) {
      _pClient = new WiFiSSLClient;
    } else {
      _pClient = new WiFiClient;
    }
    _pHttpClient = new HttpClient(*_pClient, _pHost, port);
#else
  if(!_pConnInfo->tcpClient) {
    _pConnInfo->lastError = F("No TCP client provided");
    return false;
#endif
  } else {
    _pHttpClient = new HttpClient(*_pConnInfo->tcpClient, _pHost, port);
  }  
  if(user.length()) {
    _pHttpClient->sendBasicAuth(user, pass);
  }
  HttpClient::kUserAgent = UserAgent;

  if(_httpOptions.isConnectionReuse()) {
    //can be only enabled
    _pHttpClient->connectionKeepAlive();
  }
  _pHttpClient->setHttpResponseTimeout(_httpOptions.getHttpReadTimeout());
  return true;
}


bool ArduinoHTTPService::initRequest(const char *url, const char *method) { 
  String host, path, user, pass;
  int port;
  if(!parseURL(url, host, port, path, user, pass)) {
     _pConnInfo->lastError = F("Error parsing URL ");
    return false;
  }
  if(!path.length()) {
    path = "/";
  }
  _pHttpClient->beginRequest();
  int r = _pHttpClient->startRequest(path.c_str(), method);
  if(r) {
    _pConnInfo->lastError = F("startRequest failed: ");
    _pConnInfo->lastError.concat(r);
    _lastStatusCode = r;
    return false;
  }
  return true; 
}


int ArduinoHTTPService::completeRequest() {
  int code = _pHttpClient->responseStatusCode();
  if(!code) {
    //code == 0 means most probably timeout
    code = -3;
  }
  if(code > 0) {
    readHeaders();
  }
  return code;
}

int ArduinoHTTPService::POST(const char *data) { 
  // content-type header is set by ancestor
  _pHttpClient->sendHeader(HTTP_HEADER_CONTENT_LENGTH, strlen(data));
  _pHttpClient->endRequest();
  _pHttpClient->print(data);
  return completeRequest();
}

#define HTTP_BUFFSIZE  1024 
int ArduinoHTTPService::POST(Stream *stream) {
  _pHttpClient->sendHeader(HTTP_HEADER_CONTENT_LENGTH, stream->available());
  _pHttpClient->endRequest();
  int len = stream->available();
  int bytesWritten = 0;
  int buffSize = HTTP_BUFFSIZE;
  if(len > 0 && len < buffSize ) {
    buffSize = len;
  }
  char *buff = (char *)malloc(buffSize);
  if(!buff) {
     _pConnInfo->lastError = F("cannot send stream, error allocating ");
     _pConnInfo->lastError.concat(buffSize);
     return -9;
  }
  // Copied from ESP32 HTTPClient
  while(isConnected() && (stream->available() > -1) && (len > 0 || len == -1)) {

    // get available data size
    int sizeAvailable = stream->available();

    if(sizeAvailable) {

        int readBytes = sizeAvailable;

        // read only the asked bytes
        if(len > 0 && readBytes > len) {
            readBytes = len;
        }

        // not read more the buffer can handle
        if(readBytes > buffSize) {
            readBytes = buffSize;
        }

        // read data
        int bytesRead = stream->readBytes(buff, readBytes);

        // write it to Stream
        int bytesWrite = _pHttpClient->write((const uint8_t *) buff, bytesRead);
        bytesWritten += bytesWrite;

        // are all Bytes a writen to stream ?
        if(bytesWrite != bytesRead) {
           INFLUXDB_CLIENT_DEBUG("short write, asked for %d but got %d, retry...", bytesRead, bytesWrite);
            // check for write error
            if(getStreamPtr()->getWriteError()) {
                INFLUXDB_CLIENT_DEBUG("stream write error %d", getStreamPtr()->getWriteError());

                //reset write error for retry
                getStreamPtr()->clearWriteError();
            }

            // some time for the stream
            delay(1);

            int leftBytes = (readBytes - bytesWrite);

            // retry to send the missed bytes
            bytesWrite = _pHttpClient->write((const uint8_t *) (buff + bytesWrite), leftBytes);
            bytesWritten += bytesWrite;

            if(bytesWrite != leftBytes) {
                // failed again
                INFLUXDB_CLIENT_DEBUG("short write, asked for %d but got %d failed.", leftBytes, bytesWrite);
                free(buff);
                return -10;
            }
        }

        // check for write error
        if(getStreamPtr()->getWriteError()) {
            INFLUXDB_CLIENT_DEBUG("stream write error %d", getStreamPtr()->getWriteError());
            free(buff);
            return -10;;
        }

        // count bytes to read left
        if(len > 0) {
            len -= readBytes;
        }
        delay(0);
    } else {
        delay(1);
    }
  }
 
  free(buff);
  if(len && len != bytesWritten) {
      INFLUXDB_CLIENT_DEBUG("Stream payload bytesWritten %d and size %d mismatch!.", bytesWritten, len);
      INFLUXDB_CLIENT_DEBUG("ERROR SEND PAYLOAD FAILED!");
      return -10;
  }
  return completeRequest();
}

int ArduinoHTTPService::GET() {
  _pHttpClient->endRequest();
  return completeRequest();
}

int ArduinoHTTPService::DELETE() { 
  _pHttpClient->endRequest();
  return completeRequest();
}

void ArduinoHTTPService::setHeader(const String &name, const String &value) {
  _pHttpClient->sendHeader(name, value);
}

String ArduinoHTTPService::getHeader(const char *name) { 
  if(name == RetryAfter) {
    return _retryAfter;
  }
  return String(); 
}

void ArduinoHTTPService::readHeaders() {
  _retryAfter = (const char *) nullptr;
  while(_pHttpClient->headerAvailable()) {
    String headerName = _pHttpClient->readHeaderName();
    if(headerName.equals(RetryAfter)) {
      _retryAfter = _pHttpClient->readHeaderValue();
    }
  }
}

void ArduinoHTTPService::end() {
  if(!_httpOptions.isConnectionReuse()) {
    _pHttpClient->stop();
    //Workaround - HttpClient resets timeout on stop
    _pHttpClient->setHttpResponseTimeout(_httpOptions.getHttpReadTimeout());
  }
}

 String ArduinoHTTPService::errorToString(int error) { 
   switch(error) {
    case HTTP_ERROR_CONNECTION_FAILED:
        return F("connection failed");
    case HTTP_ERROR_API:
        return F("invalid API usage");
    case HTTP_ERROR_INVALID_RESPONSE:
        return F("invalid response");
    case HTTP_ERROR_TIMED_OUT:
        return F("read Timeout");
    }
  return String(error); 
}

#endif //defined(INFLUXDB_CLIENT_NET_WIFININA) || defined(INFLUXDB_CLIENT_NET_EXTERNAL)
