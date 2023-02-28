/**
 * 
 * Options.cpp: InfluxDB Client write options and HTTP options
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
#include <Arduino.h>
#include "Options.h"
#include "util/helpers.h"

WriteOptions& WriteOptions::addDefaultTag(const String &name, const String &value) { 
    if(_defaultTags.length() > 0) {
        _defaultTags += ',';
    }
    char *s = escapeKey(name);
    _defaultTags += s;
    delete [] s;
    s = escapeKey(value);
    _defaultTags += '=';
    _defaultTags += s;
    delete [] s;
    return *this; 
}

void WriteOptions::printTo(Print &dest) const {
    dest.println("WriteOptions:");
    dest.print("\t_precision: "); dest.println((uint8_t)_writePrecision);
    dest.print("\t_batchSize: "); dest.println(_batchSize);
    dest.print("\t_bufferSize: "); dest.println(_bufferSize);
    dest.print("\t_flushInterval: "); dest.println(_flushInterval);
    dest.print("\t_retryInterval: "); dest.println(_retryInterval);
    dest.print("\t_maxRetryInterval: "); dest.println(_maxRetryInterval);
    dest.print("\t_maxRetryAttempts: "); dest.println(_maxRetryAttempts);
    dest.print("\t_defaultTags: "); dest.println(_defaultTags);
    dest.print("\t_useServerTimestamp: "); dest.println(_useServerTimestamp);
}
