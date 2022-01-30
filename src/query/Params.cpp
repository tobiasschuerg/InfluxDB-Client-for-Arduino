/**
 * 
 * Params.cpp: InfluxDB flux query param
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

#include "Params.h"
#include <algorithm>

QueryParams::QueryParams() {
  _data = std::make_shared<ParamsList>();
}


QueryParams::QueryParams(const QueryParams& other) {
  _data = other._data;
}

 QueryParams::~QueryParams() {
   if(_data) {
    std::for_each(_data->begin(), _data->end(), [](FluxBase *&value){ delete value; });
    _data->clear();
   }
 }

QueryParams &QueryParams::operator=(const QueryParams &other) {
  if(this != &other) {
    _data = other._data;
  }
  return *this;
}

QueryParams &QueryParams::add(const String &name, float value, int decimalPlaces) {
  return add(name, (double)value, decimalPlaces);
}

QueryParams &QueryParams::add(const String &name, double value, int decimalPlaces) {
  return add(new FluxDouble(name, value, decimalPlaces));
}

QueryParams &QueryParams::add(const String &name, char value) {
  String s(value);
  return add(name, s);
}

QueryParams &QueryParams::add(const String &name, unsigned char value) {
  return add(name, (unsigned long)value);
}

QueryParams &QueryParams::add(const String &name, int value) {
  return add(name,(long)value);
}

QueryParams &QueryParams::add(const String &name, unsigned int value) {
  return add(name,(unsigned long)value);
}

QueryParams &QueryParams::add(const String &name, long value) {
    return add(new FluxLong(name, value));
}

QueryParams &QueryParams::add(const String &name, unsigned long value) {
  return add(new FluxUnsignedLong(name, value));
}

QueryParams &QueryParams::add(const String &name, bool value) {
  return add(new FluxBool(name, value));
}

QueryParams &QueryParams::add(const String &name, const String &value) {
  return add(name, value.c_str());
}

QueryParams &QueryParams::add(const String &name, const __FlashStringHelper *pstr) {
  return add(name, String(pstr));
}

QueryParams &QueryParams::add(const String &name, long long value)  {
  return add(name,(long)value);
}

QueryParams &QueryParams::add(const String &name, unsigned long long value) {
  return add(name,(unsigned long)value);
}

QueryParams &QueryParams::add(const String &name, const char *value) {
  return add(new FluxString(name, value, FluxDatatypeString));
}

QueryParams &QueryParams::add(const String &name, struct tm tm, unsigned long micros ) {
  return add(new FluxDateTime(name, FluxDatatypeDatetimeRFC3339Nano, tm, micros));
}

QueryParams &QueryParams::add(FluxBase *value) {
 if(_data) {
    _data->push_back(value);
  }
  return *this;
}

void QueryParams::remove(const String &name) {
  if(_data) {
    auto it = std::find_if(_data->begin(),_data->end(), [name](FluxBase *f){ return f->getRawValue() == name; } );
    if(it != _data->end()) {
      delete *it;
      _data->erase(it);
    }
  }
}
// Copy constructor
int QueryParams::size() {
  if(_data) {
    return _data->size();
  }
  return 0;
}

FluxBase *QueryParams::get(int i) {
  if(_data) {
    return _data->at(i);
  }
  return 0;
}

char *QueryParams::jsonString(int i) {
  if(_data) {
    return _data->at(i)->jsonString();
  }
  return nullptr;
}
