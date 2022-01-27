/**
 * 
 * Prams.h: InfluxDB flux query param
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
#ifndef _QUERY_PARAM_H_
#define _QUERY_PARAM_H_

#include "FluxTypes.h"
#include <memory>
#include <vector>

typedef std::vector<FluxBase *> ParamsList;
/**
 * QueryParams holds parameters for Flux query.
 * Parameters are accessed via the 'params.' prefix: e.g. params.start
 *
 */ 
class QueryParams {
  public:
    // Constructor
    QueryParams();
    // Copy constructor
    QueryParams(const QueryParams& other);
    // Assignment operator
    QueryParams &operator=(const QueryParams &other);
    // Descructor
    ~QueryParams();
    // Adds param with a float value. A number of decimal places can be optionally set.
    QueryParams &add(const String &name, float value, int decimalPlaces = 2);
    // Adds param with a double value. A number of decimal places can be optionally set.
    QueryParams &add(const String &name, double value, int decimalPlaces = 2); 
    // Adds param with a character value.
    QueryParams &add(const String &name, char value);
    // Adds param with an unsigned character value. It is added as a number.
    QueryParams &add(const String &name, unsigned char value);
    // Adds param with an integer value.
    QueryParams &add(const String &name, int value);
    // Adds param with an unsigned integer value.
    QueryParams &add(const String &name, unsigned int value);
    // Adds param with a long value.
    QueryParams &add(const String &name, long value);
    // Adds param with an unsigned long value.
    QueryParams &add(const String &name, unsigned long value);
    // Adds param with a boolen value.
    QueryParams &add(const String &name, bool value);
    // Adds param with a String value.
    QueryParams &add(const String &name, const String &value);
    // Adds param with a progmem string value.
    QueryParams &add(const String &name, const __FlashStringHelper *pstr);
    // Adds param with a long long value. It is converted to long.
    QueryParams &add(const String &name, long long value);
    // Adds param with an unsigned long long value. It is converted to long.
    QueryParams &add(const String &name, unsigned long long value);
    // Adds param with a string value.
    QueryParams &add(const String &name, const char *value);
    // Adds param as a string representation of date-time value in UTC timezone, e.g. "2022-01-12T23:12:30.124Z"
    // Micros is fractional part of seconds up to microseconds precision. E.g. if millisecond are provided
    // it must be converted to microseconds: micros = 1000*millis
    QueryParams &add(const String &name, struct tm tm, unsigned long micros = 0);
    // Removed a param
    void remove(const String &name);
    // Returns i-th param
    FluxBase *get(int i);
    // Returns params count
    int size();
    // Returns JSON representation of i-th param
    char *jsonString(int i);
  private:
    QueryParams &add(FluxBase *value);
    std::shared_ptr<ParamsList> _data;
};

#endif //_QUERY_PARAM_H_
