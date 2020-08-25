/**
 * 
 * FluxTypes.cpp: InfluxDB flux query types support
 * 
 * MIT License
 * 
 * Copyright (c) 2018-2020 InfluxData
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

#include "FluxTypes.h"

const char	*FluxDatatypeString     = "string";
const char	*FluxDatatypeDouble     = "double";
const char	*FluxDatatypeBool       = "boolean";
const char	*FluxDatatypeLong       = "long";
const char	*FluxDatatypeUnsignedLong   = "unsignedLong";
const char	*FluxDatatypeDuration       = "duration";
const char	*FluxBinaryDataTypeBase64         = "base64Binary";
const char	*FluxDatatypeDatetimeRFC3339      = "dateTime:RFC3339";
const char	*FluxDatatypeDatetimeRFC3339Nano  = "dateTime:RFC3339Nano";

FluxBase::FluxBase(String rawValue) {
    _rawValue = rawValue;
}

FluxBase::~FluxBase() {
}


FluxLong::FluxLong(String rawValue, long value):FluxBase(rawValue),value(value) {

}

const char *FluxLong::getType() {
    return FluxDatatypeLong;
}

FluxUnsignedLong::FluxUnsignedLong(String rawValue, unsigned long value):FluxBase(rawValue),value(value) {
}

const char *FluxUnsignedLong::getType() {
    return FluxDatatypeUnsignedLong;
}

FluxDouble::FluxDouble(String rawValue, double value):FluxBase(rawValue),value(value) {
   
}

const char *FluxDouble::getType() {
    return FluxDatatypeDouble;
}

FluxBool::FluxBool(String rawValue, bool value):FluxBase(rawValue),value(value) {   
}

const char *FluxBool::getType() {
    return FluxDatatypeBool;
}


FluxDateTime::FluxDateTime(String rawValue, const char *type, struct tm value, unsigned long microseconds):FluxBase(rawValue),_type(type),value(value), microseconds(microseconds) {

}

const char *FluxDateTime::getType() {
    return _type;
}

String FluxDateTime::format(String formatString) {
  int len = formatString.length() + 20; //+20 for safety
  char *buff = new char[len];
  strftime(buff,len, formatString.c_str(),&value);
  String str = buff;
  delete [] buff;
  return str;
}

FluxString::FluxString(String rawValue, const char *type):FluxBase(rawValue),_type(type),value(_rawValue) {

}

const char *FluxString::getType() {
    return _type;
}


FluxValue::FluxValue() {}

FluxValue::FluxValue(FluxBase *fluxValue):_data(fluxValue) {
    
}

FluxValue::FluxValue(const FluxValue &other) {
    _data = other._data;
}

FluxValue& FluxValue::operator=(const FluxValue& other) {
    if(this != &other) {
        _data = other._data;
    }
    return *this;
}

// Type accessor. If value is different type zero value for given time is returned.
String FluxValue::getString() {
    if(_data && (_data->getType() == FluxDatatypeString ||_data->getType() == FluxDatatypeDuration || _data->getType() == FluxBinaryDataTypeBase64)) {
            FluxString *s = (FluxString *)_data.get();
            return s->value;
        }
    return "";
}

long FluxValue::getLong() {
    if(_data && _data->getType() == FluxDatatypeLong) {
        FluxLong *l = (FluxLong *)_data.get();
        return l->value;
    }
    return 0;
}

unsigned long FluxValue::getUnsignedLong() {
    if(_data && _data->getType() == FluxDatatypeUnsignedLong) {
        FluxUnsignedLong *l = (FluxUnsignedLong *)_data.get();
        return l->value;
    }
    return 0;

}
FluxDateTime FluxValue::getDateTime() {
    if(_data && (_data->getType() == FluxDatatypeDatetimeRFC3339 ||_data->getType() == FluxDatatypeDatetimeRFC3339Nano)) {
        FluxDateTime *d = (FluxDateTime *)_data.get();
        return *d;
    }
    return FluxDateTime("",FluxDatatypeDatetimeRFC3339, {0,0,0,0,0,0,0,0,0}, 0 );
}

bool FluxValue::getBool() {
    if(_data && _data->getType() == FluxDatatypeBool) {
        FluxBool *b = (FluxBool *)_data.get();
        return b->value;
    }
    return false;
}

double FluxValue::getDouble() {
    if(_data && _data->getType() == FluxDatatypeDouble) {
        FluxDouble *d = (FluxDouble *)_data.get();
        return d->value;
    }
    return 0.0;
}

// returns string representation of non-string values
String FluxValue::getRawValue() {
    if(_data) {
        return _data->getRawValue();
    }
    return "";
}

bool FluxValue::isNull() {
    return _data == nullptr;
}