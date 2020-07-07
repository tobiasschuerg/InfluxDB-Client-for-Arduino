/**
 * 
 * InfluxDBClient.cpp: InfluxDB Client for Arduino
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
#include "InfluxDbClient.h"
#include <core_version.h>

#define STRHELPER(x) #x
#define STR(x) STRHELPER(x) // stringifier

#if defined(ESP8266)
# define INFLUXDB_CLIENT_PLATFORM "ESP8266"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP8266_GIT_DESC)
#elif defined(ESP32)
# define INFLUXDB_CLIENT_PLATFORM "ESP32"
# define INFLUXDB_CLIENT_PLATFORM_VERSION  STR(ARDUINO_ESP32_GIT_DESC)
#endif

static const char UserAgent[] PROGMEM = "influxdb-client-arduino/" INFLUXDB_CLIENT_VERSION " (" INFLUXDB_CLIENT_PLATFORM " " INFLUXDB_CLIENT_PLATFORM_VERSION ")";

// Uncomment bellow in case of a problem and rebuild sketch
//#define INFLUXDB_CLIENT_DEBUG_ENABLE
#include "util/debug.h"

static const char UnitialisedMessage[] PROGMEM = "Unconfigured instance"; 
// This cannot be put to PROGMEM due to the way how it is used
static const char RetryAfter[] = "Retry-After";
static const char TransferEnconding[] = "Transfer-Encoding";

static String escapeJSONString(String &value);

static String precisionToString(WritePrecision precision, uint8_t version = 2) {
    switch(precision) {
        case WritePrecision::US:
            return version==1?"u":"us";
        case WritePrecision::MS:
            return "ms";
        case WritePrecision::NS:
            return "ns";
        case WritePrecision::S:
            return "s";
        default:
            return "";
    }
}

Point::Point(String measurement):
    _measurement(escapeKey(measurement, false)),
    _tags(""),
    _fields(""),
    _timestamp("")
{

}

void Point::addTag(String name, String value) {
    if(_tags.length() > 0) {
        _tags += ',';
    }
    _tags += escapeKey(name);
    _tags += '=';
    _tags += escapeKey(value);
}

void Point::addField(String name, const char *value) { 
    putField(name, "\"" + escapeValue(value) + "\""); 
}

void Point::putField(String name, String value) {
    if(_fields.length() > 0) {
        _fields += ',';
    }
    _fields += escapeKey(name);
    _fields += '=';
    _fields += value;
}

String Point::toLineProtocol() const {
    String line =  _measurement;
    if(hasTags()) {
        line += "," + _tags;
    }
    if(hasFields()) {
        line += " " + _fields;
    }
    if(hasTime()) {
        line += " " + _timestamp;
    }
    return line;
}

void  Point::setTime(WritePrecision precision) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    switch(precision) {
        case WritePrecision::NS:
            setTime(getTimeStamp(&tv,9));
            break;
        case WritePrecision::US:
            setTime(getTimeStamp(&tv,6));
            break;
        case WritePrecision::MS: 
            setTime(getTimeStamp(&tv,3));
            break;
        case WritePrecision::S:
            setTime(getTimeStamp(&tv,0));
            break;
        case WritePrecision::NoTime:
            _timestamp = "";
            break;
    }
}

void  Point::setTime(unsigned long long timestamp) {
    _timestamp = timeStampToString(timestamp);
}

void  Point::setTime(String timestamp) {
    _timestamp = timestamp;
}

void  Point::clearFields() {
    _fields = "";
    _timestamp = "";
}

void Point:: clearTags() {
    _tags = "";
}

InfluxDBClient::InfluxDBClient() { 
    _pointsBuffer = new String[_bufferSize];
}

InfluxDBClient::InfluxDBClient(const char *serverUrl, const char *db):InfluxDBClient() {
    setConnectionParamsV1(serverUrl, db);
}

InfluxDBClient::InfluxDBClient(const char *serverUrl, const char *org, const char *bucket, const char *authToken):InfluxDBClient(serverUrl, org, bucket, authToken, nullptr) { 
}

InfluxDBClient::InfluxDBClient(const char *serverUrl, const char *org, const char *bucket, const char *authToken, const char *serverCert):InfluxDBClient() {
    setConnectionParams(serverUrl, org, bucket, authToken, serverCert);
}

void InfluxDBClient::setInsecure(bool value){
  _insecure = value;
}

void InfluxDBClient::setConnectionParams(const char *serverUrl, const char *org, const char *bucket, const char *authToken, const char *certInfo) {
    clean();
    _serverUrl = serverUrl;
    _bucket = bucket;
    _org = org;
    _authToken = authToken;
    _certInfo = certInfo;
    _dbVersion = 2;
}

void InfluxDBClient::setConnectionParamsV1(const char *serverUrl, const char *db, const char *user, const char *password, const char *certInfo) {
    clean();
    _serverUrl = serverUrl;
    _bucket = db;
    _user = user;
    _password = password;
    _certInfo = certInfo;
    _dbVersion = 1;
}

bool InfluxDBClient::init() {
    INFLUXDB_CLIENT_DEBUG("Init\n");
    INFLUXDB_CLIENT_DEBUG("  Server url: %s\n", _serverUrl.c_str());
    INFLUXDB_CLIENT_DEBUG("  Org: %s\n", _org.c_str());
    INFLUXDB_CLIENT_DEBUG("  Bucket: %s\n", _bucket.c_str());
    INFLUXDB_CLIENT_DEBUG("  Token: %s\n", _authToken.c_str());
    INFLUXDB_CLIENT_DEBUG("  DB version: %d\n", _dbVersion);
    if(_serverUrl.length() == 0 || (_dbVersion == 2 && (_org.length() == 0 || _bucket.length() == 0 || _authToken.length() == 0))) {
         INFLUXDB_CLIENT_DEBUG("[E] Invalid parameters\n");
        return false;
    }
    if(_serverUrl.endsWith("/")) {
        _serverUrl = _serverUrl.substring(0,_serverUrl.length()-1);
    }
    setUrls();
    bool https = _serverUrl.startsWith("https");
    if(https) {
#if defined(ESP8266)         
        BearSSL::WiFiClientSecure *wifiClientSec = new BearSSL::WiFiClientSecure;
        if(_certInfo && strlen_P(_certInfo) > 0) {
            if(strlen_P(_certInfo) > 60 ) { //differentiate fingerprint and cert
                _cert = new BearSSL::X509List(_certInfo); 
                wifiClientSec->setTrustAnchors(_cert);
            } else {
                wifiClientSec->setFingerprint(_certInfo);
            }
        }
        if (_insecure) {
            wifiClientSec->setInsecure();
        }
#elif defined(ESP32)
        WiFiClientSecure *wifiClientSec = new WiFiClientSecure;  
        if(_certInfo && strlen_P(_certInfo) > 0) { 
              wifiClientSec->setCACert(_certInfo);
         }
#endif    
        _wifiClient = wifiClientSec;
    } else {
        _wifiClient = new WiFiClient;
    }
    _httpClient.setReuse(false);

    _httpClient.setUserAgent(FPSTR(UserAgent));
    return true;
}

InfluxDBClient::~InfluxDBClient() {
     if(_pointsBuffer) {
        delete [] _pointsBuffer;
        _pointsBuffer = nullptr;
        _bufferPointer = 0;
        _batchPointer = 0;
        _bufferCeiling = 0;
    }
    clean();
}

void InfluxDBClient::clean() {
    // if(_wifiClient) {
    //     delete _wifiClient;
    //     _wifiClient = nullptr;
    // }
     _wifiClient = nullptr;
#if defined(ESP8266)     
    if(_cert) {
        delete _cert;
        _cert = nullptr;
    }
#endif
    _lastStatusCode = 0;
    _lastErrorResponse = "";
    _lastFlushed = 0;
    _lastRequestTime = 0;
    _lastRetryAfter = 0;
}

void InfluxDBClient::setUrls() {
    INFLUXDB_CLIENT_DEBUG("setUrls\n");
    if(_dbVersion == 2) {
        _writeUrl = _serverUrl;
        _writeUrl += "/api/v2/write?org=";
        _writeUrl +=  _org ;
        _writeUrl += "&bucket=";
        _writeUrl += _bucket;
        INFLUXDB_CLIENT_DEBUG("  writeUrl: %s\n", _writeUrl.c_str());
        _queryUrl = _serverUrl;
        _queryUrl += "/api/v2/query?org=";
        _queryUrl +=  _org;
        INFLUXDB_CLIENT_DEBUG("  queryUrl: %s\n", _queryUrl.c_str());
    } else {
        _writeUrl = _serverUrl;
        _writeUrl += "/write?db=";
        _writeUrl += _bucket;
        _queryUrl = _serverUrl;
        _queryUrl += "/api/v2/query";
        if(_user.length() > 0 && _password.length() > 0) {
            String auth = "&u=";
            auth += _user;
            auth += "&p=";
            auth += _password;
            _writeUrl += auth;  
            _queryUrl += "?";
            _queryUrl += auth;
        }
        INFLUXDB_CLIENT_DEBUG("  writeUrl: %s\n", _writeUrl.c_str());
        INFLUXDB_CLIENT_DEBUG("  queryUrl: %s\n", _queryUrl.c_str());
    }
    if(_writePrecision != WritePrecision::NoTime) {
        _writeUrl += "&precision=";
        _writeUrl += precisionToString(_writePrecision, _dbVersion);
        INFLUXDB_CLIENT_DEBUG("  writeUrl: %s\n", _writeUrl.c_str());
    }
    
}

void InfluxDBClient::setWriteOptions(WritePrecision precision, uint16_t batchSize, uint16_t bufferSize, uint16_t flushInterval, bool preserveConnection) {
    if(_writePrecision != precision) {
        _writePrecision = precision;
        setUrls();
    }
    if(batchSize > 0) {
        _batchSize = batchSize;
    }
    if(bufferSize < batchSize) {
        bufferSize = 2*batchSize;
        INFLUXDB_CLIENT_DEBUG("[D] Changing buffer size to %d\n", bufferSize);
    }
    if(_bufferSize > 0 && bufferSize > 0 && _bufferSize != bufferSize) {
        _bufferSize = bufferSize;
        if(_bufferSize <  _batchSize) {
            _bufferSize = 2*_batchSize;
            INFLUXDB_CLIENT_DEBUG("[D] Changing buffer size to %d\n", _bufferSize);
        }
        resetBuffer();
    }
    _flushInterval = flushInterval;
    _httpClient.setReuse(preserveConnection);
}

void InfluxDBClient::resetBuffer() {
    if(_pointsBuffer) {
        delete [] _pointsBuffer;
    }
    _pointsBuffer = new String[_bufferSize];
    _bufferPointer = 0;
    _batchPointer = 0;
    _bufferCeiling = 0;
}

void InfluxDBClient::reserveBuffer(int size) {
    if(size > _bufferSize) {
        String *newBuffer = new String[size];
        INFLUXDB_CLIENT_DEBUG("Resising buffer from %d to %d\n", _bufferSize, size);
        for(int i=0;i<_bufferCeiling; i++) {
            newBuffer[i] = _pointsBuffer[i];
        }
        
        delete [] _pointsBuffer;
        _pointsBuffer = newBuffer;
        _bufferSize = size;
    }
}

bool InfluxDBClient::writePoint(Point & point) {
    if (point.hasFields()) {
        if(_writePrecision != WritePrecision::NoTime && !point.hasTime()) {
            point.setTime(_writePrecision);
        }
        String line = point.toLineProtocol();
        return writeRecord(line);
    }
    return false;
}

bool InfluxDBClient::writeRecord(String &record) {
    _pointsBuffer[_bufferPointer] = record;
    _bufferPointer++;
    if(_bufferPointer == _bufferSize) {
        _bufferPointer = 0;
        INFLUXDB_CLIENT_DEBUG("[W] Reached buffer size, old points will be overwritten\n");
        if(isBufferFull()) {
            //if already isBufferFull
            _batchPointer = 0;
        }
    } 
    if(_bufferCeiling < _bufferSize) {
        _bufferCeiling++;
    }
    if(isBufferFull() && _batchPointer < _bufferPointer) {
        // When we are overwriting buffer and nothing is written, batchPointer must point to the oldest point
        _batchPointer = _bufferPointer;
    }
    return checkBuffer();
}

bool InfluxDBClient::checkBuffer() {
    // in case we (over)reach batchSize with non full buffer
    bool bufferReachedBatchsize = !isBufferFull() && _bufferPointer - _batchPointer >= _batchSize;
    // or flush interval timed out
    bool flushTimeout = _flushInterval > 0 && _lastFlushed > 0 && (millis()/1000 - _lastFlushed) > _flushInterval; 

    if(bufferReachedBatchsize || flushTimeout || isBufferFull() ) {
        INFLUXDB_CLIENT_DEBUG("[D] Flushing buffer: is oversized %s, is timeout %s, is buffer full %s\n", bufferReachedBatchsize?"true":"false",flushTimeout?"true":"false", isBufferFull()?"true":"false");
       return flushBuffer();
    } 
    return true;
}

bool InfluxDBClient::flushBuffer() {
    if(_lastRetryAfter > 0 && (millis()-_lastRequestTime)/1000 < _lastRetryAfter) {
        // retry after period didn't run out yet
        return false;
    }
    char *data;
    int size;
    bool success = true;
    // send all batches, It could happen there was long network outage and buffer is full
    while(data = prepareBatch(size)) {
        INFLUXDB_CLIENT_DEBUG("[D] Writing batch, size %d\n", size);
        int statusCode = postData(data);
        delete [] data;
        // retry on unsuccessfull connection or retryable status codes
        bool retry = statusCode < 0 || statusCode == 429 || statusCode == 503;
        success = statusCode == 204;
        // advance even on message failure (4xx != 429) or server failure (5xx != 503) 
        if(success || !retry) {
            _lastFlushed = millis()/1000;
            _batchPointer += size;
            //did we got over top?
            if(_batchPointer >= _bufferSize) {
                // restart _batchPointer in ring buffer from start
                _batchPointer = _batchPointer - _bufferSize;
                // we reached buffer size, that means buffer was full and now lower ceiling 
                _bufferCeiling = _bufferPointer;
            }
        } else {
            INFLUXDB_CLIENT_DEBUG("[D] Leaving data in buffer for retry\n");
            // in case of retryable failure break loop
            break;
        }
       yield();
    }
    //Have we emptied the buffer?
    if(success) {
        if(_batchPointer == _bufferPointer) {
            _bufferPointer = 0;
            _batchPointer = 0;
            _bufferCeiling = 0;
            INFLUXDB_CLIENT_DEBUG("[D] Buffer empty\n");
        }
    }
    return success;
}

char *InfluxDBClient::prepareBatch(int &size) {
    size = 0;
    int length = 0;
    char *buff = nullptr;
    uint16_t top = _batchPointer+_batchSize;
    INFLUXDB_CLIENT_DEBUG("[D] Prepare batch: bufferPointer: %d, batchPointer: %d, ceiling %d\n", _bufferPointer, _batchPointer, _bufferCeiling);
    if(top > _bufferCeiling ) {
        // are we returning to the begining?
        if(isBufferFull()) {
            top = top - _bufferCeiling;
            // in case we are writing points in the begining of the buffer that have been overwritten, end on _bufferPointer
            if(top > _bufferPointer) {
                top = _bufferPointer;
            }
        } else {
            top = _bufferCeiling;
        }
    } 
    if(top > _batchPointer) { 
        size = top - _batchPointer;
    } else if(top < _batchPointer) {
        size = _bufferSize - (_batchPointer - top);
    }
    INFLUXDB_CLIENT_DEBUG("[D] Prepare batch size %d\n", size);
    if(size) {
        int i = _batchPointer;
        for(int c=0; c < size; c++) {
            length += _pointsBuffer[i++].length();
            if(i == _bufferSize) {
                i = 0;
            }
            yield();
        }
        //create buffer for all lines including new line char and terminating char
        buff = new char[length + size + 1];
        if(buff) {
            buff[0] = 0;
            int i = _batchPointer;
            for(int c=0; c < size; c++) {
                strcat(buff+strlen(buff), _pointsBuffer[i++].c_str());
                strcat(buff+strlen(buff), "\n");
                if(i == _bufferSize) {
                    i = 0;
                }
                yield();
            }
        } else {
            size = 0;
        }
    }
    return buff;
}

bool InfluxDBClient::validateConnection() {
    if(!_wifiClient && !init()) {
        _lastStatusCode = 0;
        _lastErrorResponse = FPSTR(UnitialisedMessage);
        return false;
    }
    // on version 1.x /ping will by default return status code 204, without verbose
    String url = _serverUrl + (_dbVersion==2?"/health":"/ping?verbose=true");
    INFLUXDB_CLIENT_DEBUG("[D] Validating connection to %s\n", url.c_str());

    if(!_httpClient.begin(*_wifiClient, url)) {
        INFLUXDB_CLIENT_DEBUG("[E] begin failed\n");
        return false;
    }
    _httpClient.addHeader(F("Accept"), F("application/json"));
    
    _lastStatusCode = _httpClient.GET();

   _lastErrorResponse = "";
    
    postRequest(200);

    _httpClient.end();

    return _lastStatusCode == 200;
}

void InfluxDBClient::preRequest() {
    if(_authToken.length() > 0) {
        _httpClient.addHeader(F("Authorization"), "Token " + _authToken);
    }
    const char * headerKeys[] = {RetryAfter, TransferEnconding} ;
    _httpClient.collectHeaders(headerKeys, 2);
}

int InfluxDBClient::postData(const char *data) {
    if(!_wifiClient && !init()) {
        _lastStatusCode = 0;
        _lastErrorResponse = FPSTR(UnitialisedMessage);
        return 0;
    }
    if(data) {
        INFLUXDB_CLIENT_DEBUG("[D] Writing to %s\n", _writeUrl.c_str());
        if(!_httpClient.begin(*_wifiClient, _writeUrl)) {
            INFLUXDB_CLIENT_DEBUG("[E] Begin failed\n");
            return false;
        }
        INFLUXDB_CLIENT_DEBUG("[D] Sending:\n%s\n", data);       

        _httpClient.addHeader(F("Content-Type"), F("text/plain"));   
        
        preRequest();        
        
        _lastStatusCode = _httpClient.POST((uint8_t*)data, strlen(data));
        
        postRequest(204);

        
        _httpClient.end();
    } 
    return _lastStatusCode;
}



static const char QueryDialect[] PROGMEM = "\
\"dialect\": {\
\"annotations\": [\
\"datatype\"\
],\
\"dateTimeFormat\": \"RFC3339\",\
\"header\": true,\
\"delimiter\": \",\",\
\"commentPrefix\": \"#\"\
}}";

FluxQueryResult InfluxDBClient::query(String fluxQuery) {
    if(_lastRetryAfter > 0 && (millis()-_lastRequestTime)/1000 < _lastRetryAfter) {
        // retry after period didn't run out yet
        return FluxQueryResult("Too early");
    }
    if(!_wifiClient && !init()) {
        _lastStatusCode = 0;
        _lastErrorResponse = FPSTR(UnitialisedMessage);
        return FluxQueryResult(_lastErrorResponse);
    }
    INFLUXDB_CLIENT_DEBUG("[D] Query to %s\n", _queryUrl.c_str());
    if(!_httpClient.begin(*_wifiClient, _queryUrl)) {
        INFLUXDB_CLIENT_DEBUG("[E] begin failed\n");
        return FluxQueryResult("");;
    }
    _httpClient.addHeader(F("Content-Type"), F("application/json"));
    
    preRequest();

    INFLUXDB_CLIENT_DEBUG("[D] JSON query:\n%s\n", fluxQuery.c_str());

    String body = F("{\"type\":\"flux\",\"query\":\"");
    body += escapeJSONString(fluxQuery) + "\",";
    body += FPSTR(QueryDialect);

    _lastStatusCode = _httpClient.POST(body);
    
    postRequest(200);
    if(_lastStatusCode == 200) {
        bool chunked = false;
        if(_httpClient.hasHeader(TransferEnconding)) {
            String header = _httpClient.header(TransferEnconding);
            chunked = header.equalsIgnoreCase("chunked");
        }
        INFLUXDB_CLIENT_DEBUG("[D] chunked: %s\n", chunked?"true":"false");
        HttpStreamScanner *scanner = new HttpStreamScanner(&_httpClient, chunked);
        CsvReader *reader = new CsvReader(scanner);
        
        return FluxQueryResult(reader);
    } else {
        _httpClient.end();
        return FluxQueryResult(_lastErrorResponse);
    }
}

void InfluxDBClient::postRequest(int expectedStatusCode) {
    _lastRequestTime = millis();
     INFLUXDB_CLIENT_DEBUG("[D] HTTP status code - %d\n", _lastStatusCode);
    if(_lastStatusCode == 429 || _lastStatusCode == 503) { //retryable 
        int retry = 0;
        if(_httpClient.hasHeader(RetryAfter)) {
            retry = _httpClient.header(RetryAfter).toInt();
            if(retry > 0 ) {
                _lastRetryAfter = retry;
                 INFLUXDB_CLIENT_DEBUG("[D] Reply after - %d\n", _lastRetryAfter);
            }
        }
        if(retry == 0) {
            _lastRetryAfter = 60;
        }
    } else {
        _lastRetryAfter = 0;
    }
    _lastErrorResponse = "";
    if(_lastStatusCode != expectedStatusCode) {
        if(_lastStatusCode > 0) {
            _lastErrorResponse = _httpClient.getString();
            INFLUXDB_CLIENT_DEBUG("[D] Response:\n%s\n", _lastErrorResponse.c_str());
        } else {
            _lastErrorResponse = _httpClient.errorToString(_lastStatusCode);
            INFLUXDB_CLIENT_DEBUG("[E] Error - %s\n", _lastErrorResponse.c_str());
        }
    }
}

static String escapeJSONString(String &value) {
    String ret;
    int d = 0;
    int i,from = 0;
    while((i = value.indexOf('"',from)) > -1) {
        d++;
        if(i == value.length()-1) {
            break;
        }
        from = i+1;
    }
    ret.reserve(value.length()+d); //most probably we will escape just double quotes
    for (char c: value)
    {
        switch (c)
        {
            case '"': ret += "\\\""; break;
            case '\\': ret += "\\\\"; break;
            case '\b': ret += "\\b"; break;
            case '\f': ret += "\\f"; break;
            case '\n': ret += "\\n"; break;
            case '\r': ret += "\\r"; break;
            case '\t': ret += "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    ret += "\\u";
                    char buf[3 + 8 * sizeof(unsigned int)];
                    sprintf(buf,  "\\u%04u", c);
                    ret += buf;
                } else {
                    ret += c;
                }
        }
    }
    return ret;
}
