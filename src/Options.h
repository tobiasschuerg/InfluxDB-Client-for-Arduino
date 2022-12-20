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


/**
 * WriteOptions holds write related options
 */
class WriteOptions {
private:
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
    // Default retry interval in sec, if not sent by server. Default 5s. 
    // Setting to zero disables retrying.
    uint16_t _retryInterval;
    // Maximum retry interval in sec, default 5min (300s)
    uint16_t _maxRetryInterval;
    // Maximum count of retry attempts of failed writes, default 3
    uint16_t _maxRetryAttempts;
    // Default tags. Default tags are added to every written point. 
    // There cannot be the same tags in the default tags and among the tags included with a point.
    String _defaultTags;
    //  Let server assign timestamp in given precision. Do not sent timestamp.
    bool _useServerTimestamp;
public:
    WriteOptions():
        _writePrecision(WritePrecision::NoTime),
        _batchSize(1),
        _bufferSize(5),
        _flushInterval(60),
        _retryInterval(5),
        _maxRetryInterval(300),
        _maxRetryAttempts(3),
        _useServerTimestamp(false) {
        }
    // Sets the timestamp precision. If timestamp precision is set, but a point does not have a timestamp, timestamp is automatically assigned from the device clock.
    // If useServerTimestamp is set to true, timestamp is not sent, only precision is specified for the server.
    WriteOptions& writePrecision(WritePrecision precision) { _writePrecision = precision; return *this; }
    // Returns the  write precision
    WritePrecision getWritePrecision() const { return _writePrecision; }
    // Sets the number of points that will be written to the databases at once. Points are added one by one and when number reaches batch size there are sent to server.
    WriteOptions& batchSize(uint16_t batchSize) { _batchSize = batchSize; return *this; }
    // Returns the count of points that will be written to the databases at once
    uint16_t getBatchSize() const { return _batchSize; }
    // Sets the size of the write buffer to control maximum number of record to keep in case of write failures.
    // When the max size is reached, oldest records are overwritten.
    WriteOptions& bufferSize(uint16_t bufferSize) { _bufferSize = bufferSize; return *this; }
    // Returns the size of the write buffer
    uint16_t getBufferSize() const { return _bufferSize; }
    // Sets the interval, in seconds, after which points will be written to the db. 
    WriteOptions& flushInterval(uint16_t flushIntervalSec) { _flushInterval = flushIntervalSec; return *this; }
    // Returns the interval, in seconds, after whitch points will be written to the db
    uint16_t getFlushInterval() const { return _flushInterval; }
    // Sets the default retry interval in sec. This is used in case of network failure or if server is bussy and doesn't specify retry interval.  
    // Setting to zero disables retrying.
    WriteOptions& retryInterval(uint16_t retryIntervalSec) { _retryInterval = retryIntervalSec; return *this; }
    // Returns the default retry interval in sec
    uint16_t getRetryInterval() const  { return _retryInterval; }
    // Sets the maximum retry interval in sec.
    WriteOptions& maxRetryInterval(uint16_t maxRetryIntervalSec) { _maxRetryInterval = maxRetryIntervalSec; return *this; }
    // Returns the maximum retry interval, in sec
    uint16_t getMaxRetryInterval() const { return _maxRetryInterval; }
    // Sets the maximum number of retry attempts of failed writes.
    WriteOptions& maxRetryAttempts(uint16_t maxRetryAttempts) { _maxRetryAttempts = maxRetryAttempts; return *this; }
    // Returns maximum number of retry attempts of failed writes.
    uint16_t getMaxRetryAttempts() const { return _maxRetryAttempts; }
    // Sets default tags list. Default tags are added to every written point. 
    WriteOptions& defaultTags(const String &tags) {  _defaultTags = tags; return *this; }
    // Adds new default tag. Default tags are added to every writen point. 
    // There cannot be the same tag in the default tags and in the tags included with a point.
    WriteOptions& addDefaultTag(const String &name, const String &value);
    // Clears default tag list
    WriteOptions& clearDefaultTags() { _defaultTags = (char *)nullptr; return *this; }
    // Returns the default tag list in line protocol format
    String getDefaultTags() const { return _defaultTags; } 
    // If timestamp precision is set and useServerTimestamp  is true, timestamp from point is not sent, or assigned.
    WriteOptions& useServerTimestamp(bool useServerTimestamp) { _useServerTimestamp = useServerTimestamp; return *this; }
    // Returns true if server should assign timestamp if point doesn't contain a timestamp
    bool isUseServerTimestamp() const { return _useServerTimestamp; }
};

/**
 * HTTPOptions holds HTTP related options
 */
class HTTPOptions {
private:    
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
    // Set true if HTTP connection should be kept open. Usable for frequent writes.
    HTTPOptions& connectionReuse(bool connectionReuse) { _connectionReuse = connectionReuse; return *this; }
    // Sets timeout after which HTTP stops reading
    HTTPOptions& httpReadTimeout(int httpReadTimeoutMs) { _httpReadTimeout = httpReadTimeoutMs; return *this; }
    // Returns true if HTTP connection should be kept open
    bool isConnectionReuse() const { return _connectionReuse; }
    // Returns timeout after which HTTP stops reading
    int getHttpReadTimeout() const { return _httpReadTimeout; }

};

#endif //_OPTIONS_H_
