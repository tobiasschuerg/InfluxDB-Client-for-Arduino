/**
 * 
 * Point.cpp: Point for write into InfluxDB server
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

#include "Point.h"
#include "util/helpers.h"

Point::Point(const String & measurement)
{
  _data = std::make_shared<Data>(escapeKey(measurement, false));
}

Point::~Point() {
}

Point::Data::Data(char * measurement) {
  this->measurement = measurement;
  timestamp = nullptr;
  tsWritePrecision = WritePrecision::NoTime;
}

Point::Data::~Data() {
  delete [] measurement;
  delete [] timestamp;
}

Point::Point(const Point &other) {
  *this = other;
}

Point& Point::operator=(const Point &other) {
  if(this != &other) {
    this->_data = other._data;
  }
  return *this;
}

void Point::addTag(const String &name, String value) {
  if(_data->tags.length() > 0) {
      _data->tags += ',';
  }
  char *s = escapeKey(name);
  _data->tags += s;
  delete [] s;
  _data->tags += '=';
  s = escapeKey(value);
  _data->tags += s;
  delete [] s;
}

void Point::addField(const String &name, long long value) {
  char buff[23];
  snprintf(buff, 50, "%lld", value);
  putField(name, String(buff)+"i");
}

void Point::addField(const String &name, unsigned long long value) {
  char buff[23];
  snprintf(buff, 50, "%llu", value);
  putField(name, String(buff)+"i");
}

void Point::addField(const String &name, const char *value) { 
    putField(name, escapeValue(value)); 
}

void Point::addField(const String &name, const __FlashStringHelper *pstr) {
    addField(name, String(pstr));
}

void Point::addField(const String &name, float value, int decimalPlaces) { 
    if(!isnan(value)) putField(name, String(value, decimalPlaces)); 
}

void Point::addField(const String &name, double value, int decimalPlaces) {
    if(!isnan(value)) putField(name, String(value, decimalPlaces)); 
}

void Point::addField(const String &name, char value) { 
    addField(name, String(value).c_str()); 
}

void Point::addField(const String &name, unsigned char value) {
    putField(name, String(value)+"i"); 
}

void Point::addField(const String &name, int value) { 
    putField(name, String(value)+"i"); 
}

void Point::addField(const String &name, unsigned int value) { 
    putField(name, String(value)+"i"); 
}

void Point::addField(const String &name, long value)  { 
    putField(name, String(value)+"i"); 
}

void Point::addField(const String &name, unsigned long value) { 
    putField(name, String(value)+"i"); 
}

void Point::addField(const String &name, bool value)  { 
    putField(name, bool2string(value)); 
}

void Point::addField(const String &name, const String &value)  { 
    addField(name, value.c_str()); 
}

void Point::putField(const String &name, const String &value) {
    if(_data->fields.length() > 0) {
        _data->fields += ',';
    }
    char *s = escapeKey(name);
    _data->fields += s;
    delete [] s;
    _data->fields += '=';
    _data->fields += value;
}

String Point::toLineProtocol(const String &includeTags) const {
    return createLineProtocol(includeTags);
}

String Point::createLineProtocol(const String &incTags, bool excludeTimestamp) const {
    String line;
    line.reserve(strLen(_data->measurement) + 1 + incTags.length() + 1 + _data->tags.length() + 1 + _data->fields.length() + 1 + strLen(_data->timestamp));
    line += _data->measurement;
    if(incTags.length()>0) {
        line += ",";
        line += incTags;
    }
    if(hasTags()) {
        line += ",";
        line += _data->tags;
    }
    if(hasFields()) {
        line += " ";
        line += _data->fields;
    }
    if(hasTime() && !excludeTimestamp) {
        line += " ";
        line += _data->timestamp;
    }
    return line;
 }

void Point::setTime(WritePrecision precision) {
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
            setTime((char *)nullptr);
            break;
    }
    _data->tsWritePrecision = precision;
}

void  Point::setTime(unsigned long long timestamp) {
    setTime(timeStampToString(timestamp));
}

void Point::setTime(const String &timestamp) {
    setTime(cloneStr(timestamp.c_str()));
}

void Point::setTime(const char *timestamp) {
    setTime(cloneStr(timestamp));    
}

void Point::setTime(char *timestamp) {
    delete [] _data->timestamp;
    _data->timestamp = timestamp;
}

void  Point::clearFields() {
    _data->fields = (char *)nullptr;
    delete [] _data->timestamp;
    _data->timestamp = nullptr;
}

void Point:: clearTags() {
    _data->tags = (char *)nullptr;
}
