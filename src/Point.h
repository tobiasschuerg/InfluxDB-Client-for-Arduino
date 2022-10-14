/**
 * 
 * Point.h: Point for write into InfluxDB server
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
#ifndef _POINT_H_
#define _POINT_H_

#include <Arduino.h>
#include "WritePrecision.h"
#include "util/helpers.h"
#include <memory>

/**
 * Class Point represents InfluxDB point in line protocol.
 * It defines data to be written to InfluxDB.
 */
class Point {
friend class InfluxDBClient;
  public:
    Point(const String &measurement);
    Point(const Point &other);
    Point& operator=(const Point &other);
    virtual ~Point();
    // Adds string tag 
    void addTag(const String &name, String value);
    // Add field with various types
    void addField(const String &name, float value, int decimalPlaces = 2);
    void addField(const String &name, double value, int decimalPlaces = 2);
    void addField(const String &name, char value);
    void addField(const String &name, unsigned char value);
    void addField(const String &name, int value);
    void addField(const String &name, unsigned int value);
    void addField(const String &name, long value);
    void addField(const String &name, unsigned long value);
    void addField(const String &name, bool value);
    void addField(const String &name, const String &value);
    void addField(const String &name, const __FlashStringHelper *pstr);
    void addField(const String &name, long long value);
    void addField(const String &name, unsigned long long value);
    void addField(const String &name, const char *value);
    // Set timestamp to `now()` and store it in specified precision, nanoseconds by default. Date and time must be already set. See `configTime` in the device API
    void setTime(WritePrecision writePrecision = WritePrecision::NS);
    // Set timestamp in offset since epoch (1.1.1970). Correct precision must be set InfluxDBClient::setWriteOptions.
    void setTime(unsigned long long timestamp);
    // Set timestamp in offset since epoch (1.1.1970 00:00:00). Correct precision must be set InfluxDBClient::setWriteOptions.
    void setTime(const String &timestamp);
    // Set timestamp in offset since epoch (1.1.1970 00:00:00). Correct precision must be set InfluxDBClient::setWriteOptions.
    void setTime(const char *timestamp);
    // Clear all fields. Usefull for reusing point  
    void clearFields();
    // Clear tags
    void clearTags();
    // True if a point contains at least one field. Points without a field cannot be written to db
    bool hasFields() const { return _data->fields.length() > 0; }
    // True if a point contains at least one tag
    bool hasTags() const   { return _data->tags.length() > 0; }
    // True if a point contains timestamp
    bool hasTime() const   { return strLen(_data->timestamp) > 0; }
    // Creates line protocol with optionally added tags
    String toLineProtocol(const String &includeTags = "") const;
    // returns current timestamp
    String getTime() const { return _data->timestamp; } 
  protected:
    class Data {
      public:
        Data(char *measurement);
        ~Data();
        char *measurement;
        String tags;
        String fields;
        char *timestamp;
        WritePrecision tsWritePrecision;
    };
    std::shared_ptr<Data> _data;
  protected:    
    // method for formating field into line protocol
    void putField(const String &name, const String &value);
    // set timestamp
    void setTime(char *timestamp);
    // Creates line protocol string
    String createLineProtocol(const String &incTags, bool excludeTimestamp = false) const;
};
#endif //_POINT_H_
