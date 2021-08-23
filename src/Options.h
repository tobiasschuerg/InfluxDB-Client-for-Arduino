/**
 * 
 * Options.h: InfluxDB Client write options and HTTP options
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
#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include "WritePrecision.h"

class InfluxDBClient;
class HTTPService;
class Influxdb;
class Test;

/**
 * WriteOptions holds write related options
 */
class WriteOptions {
private:
    friend class InfluxDBClient;
    friend class Influxdb;
    friend class Test;
    // Points timestamp precision
    WritePrecision _writePrecision; 
    // Number of points that will be written to the databases at once. 
    // Default 1 (immediate write, no batching)
    uint16_t _batchSize;
    // Write buffer size - maximum number of record to keep.
    // When max size is reached, oldest records are overwritten.
    // Default 5
    uint16_t _bufferSize;
    // Maximum number of seconds points can be held in buffer before are written to the db. 
    // Buffer is flushed when it reaches batch size or when flush interval runs out. 
    uint16_t _flushInterval;
    // Default retry interval in sec, if not sent by server. Default 5s
    uint16_t _retryInterval;
    // Maximum retry interval in sec, default 5min (300s)
    uint16_t _maxRetryInterval;
    // Maximum count of retry attempts of failed writes, default 3
    uint16_t _maxRetryAttempts;
    // Default tags. Default tags are added to every written point. 
    // There cannot be duplicate tags in default tags and tags included in a point.
    String _defaultTags;
public:
    WriteOptions():
        _writePrecision(WritePrecision::NoTime),
        _batchSize(1),
        _bufferSize(5),
        _flushInterval(60),
        _retryInterval(5),
        _maxRetryInterval(300),
        _maxRetryAttempts(3) {
        }
    WriteOptions& writePrecision(WritePrecision precision) { _writePrecision = precision; return *this; }
    WriteOptions& batchSize(uint16_t batchSize) { _batchSize = batchSize; return *this; }
    WriteOptions& bufferSize(uint16_t bufferSize) { _bufferSize = bufferSize; return *this; }
    WriteOptions& flushInterval(uint16_t flushIntervalSec) { _flushInterval = flushIntervalSec; return *this; }
    WriteOptions& retryInterval(uint16_t retryIntervalSec) { _retryInterval = retryIntervalSec; return *this; }
    WriteOptions& maxRetryInterval(uint16_t maxRetryIntervalSec) { _maxRetryInterval = maxRetryIntervalSec; return *this; }
    WriteOptions& maxRetryAttempts(uint16_t maxRetryAttempts) { _maxRetryAttempts = maxRetryAttempts; return *this; }
    WriteOptions& addDefaultTag(String name, String value);
    WriteOptions& clearDefaultTags() { _defaultTags = (char *)nullptr; return *this; }
};

/**
 * HTTPOptions holds HTTP related options
 */
class HTTPOptions {
private:    
    friend class InfluxDBClient;
    friend class HTTPService;
    friend class Influxdb;
    friend class Test;
    // true if HTTP connection should be kept open. Usable for frequent writes.
    // Default false.
    bool _connectionReuse;
    // Timeout [ms] for reading server response.
    // Default 5000ms  
    int _httpReadTimeout;
public:
    HTTPOptions():
        _connectionReuse(false),
        _httpReadTimeout(5000) {
        }
    HTTPOptions& connectionReuse(bool connectionReuse) { _connectionReuse = connectionReuse; return *this; }
    HTTPOptions& httpReadTimeout(int httpReadTimeoutMs) { _httpReadTimeout = httpReadTimeoutMs; return *this; }
};

#endif //_OPTIONS_H_
