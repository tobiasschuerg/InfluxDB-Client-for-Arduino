
#include "HTTPService.h"
#include "Platform.h"
#include "Version.h"

#include "util/debug.h"

const char *UserAgent = "influxdb-client-arduino/" INFLUXDB_CLIENT_VERSION " (" INFLUXDB_CLIENT_PLATFORM " " INFLUXDB_CLIENT_PLATFORM_VERSION ")";
const char *RetryAfter = "Retry-After";
const char *TransferEncoding = "Transfer-Encoding";


HTTPService::HTTPService(ConnectionInfo *pConnInfo):_pConnInfo(pConnInfo) {
  _apiURL = pConnInfo->serverUrl;
  _apiURL += "/api/v2/";
}

void HTTPService::setHTTPOptions(const HTTPOptions & httpOptions) {
  _httpOptions = httpOptions;
}


bool HTTPService::beforeRequest(const char *url, const char *method, const char *contentType) {
  _pConnInfo->lastError = (char *)nullptr;
   if(!initRequest(url, method)) {
    if(!_pConnInfo->lastError.length()) {
      _pConnInfo->lastError = F("begin failed");
    }
    return false;
  }
  if(contentType) {
    setHeader(F("Content-Type"), FPSTR(contentType));
  }
  if(_pConnInfo->authToken.length() > 0) {
    setHeader(F("Authorization"), "Token " + _pConnInfo->authToken);  
  }
  return true;
}


bool HTTPService::afterRequest(int expectedStatusCode, httpResponseCallback cb,  bool modifyLastConnStatus) {
  String header;
  INFLUXDB_CLIENT_DEBUG("[D] HTTP status code - %d\n", _lastStatusCode);
  if(modifyLastConnStatus) {
    _lastRequestTime = millis();
    _lastRetryAfter = 0;
    if(_lastStatusCode >= 429) { //retryable server errors
      header = getHeader(RetryAfter);
      if(header.length()) {
          _lastRetryAfter = header.toInt();
          INFLUXDB_CLIENT_DEBUG("[D] Reply after - %d\n", _lastRetryAfter);
      }
    }
  }
  header = getHeader(TransferEncoding);
  _isChunked = header.length() && header.equalsIgnoreCase("chunked");
  bool ret = _lastStatusCode == expectedStatusCode;
  bool endConnection = true;
  if(!ret) {
      if(_lastStatusCode > 0) {
          _pConnInfo->lastError = getString();
          INFLUXDB_CLIENT_DEBUG("[D] Response:\n%s\n", _pConnInfo->lastError.c_str());
      } else {
          _pConnInfo->lastError = errorToString(_lastStatusCode);
          INFLUXDB_CLIENT_DEBUG("[E] Error - %s\n", _pConnInfo->lastError.c_str());
      }
  } else if(cb){
    endConnection = cb(this);
  }
  if(endConnection) {
      end();
  }
  return ret;
}

bool HTTPService::doPOST(const char *url, const char *data, const char *contentType, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] POST request - %s, data: %dbytes, type %s\n", url, strlen(data), contentType);
  if(!beforeRequest(url, "POST", contentType)) {
    return false;
  }
 
  _lastStatusCode = POST(data);
  return afterRequest(expectedCode, cb);
}

bool HTTPService::doPOST(const char *url, Stream *stream, const char *contentType, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] POST request - %s, data: %dbytes, type %s\n", url, stream->available(), contentType);
  if(!beforeRequest(url, "POST", contentType)) {
    return false;
  }
  
  _lastStatusCode = POST(stream);
  return afterRequest(expectedCode, cb);
}

bool HTTPService::doGET(const char *url, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] GET request - %s\n", url);
  if(!beforeRequest(url, "GET", nullptr)) {
    return false;
  }
  _lastStatusCode = GET();
  return afterRequest(expectedCode, cb, false);
}

bool HTTPService::doDELETE(const char *url, int expectedCode, httpResponseCallback cb) {
  INFLUXDB_CLIENT_DEBUG("[D] DELETE - %s\n", url);
  if(!beforeRequest(url, "DELETE", nullptr)) {
    return false;
  }
  _lastStatusCode = DELETE();
  return afterRequest(expectedCode, cb, false);
}