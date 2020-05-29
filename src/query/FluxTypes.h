/**
 * 
 * FLuxTypes.h: InfluxDB flux types representation
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
#ifndef _FLUX_TYPES_H_
#define _FLUX_TYPES_H_

#include <Arduino.h>
#include <memory>

/** Supported flux types:
 *  - long - converts to long
 *  - unsignedLong - converts to unsigned long
 *  - double - converts to double
 *  - bool  - converts to bool
 *  - dateTime:RFC3339 - converts to FluxDataTime
 *  - dateTime:RFC3339Nano - converts to FluxDataTime
 *  other types defaults to String
 */

extern const char	*FluxDatatypeString;
extern const char	*FluxDatatypeDouble;
extern const char	*FluxDatatypeBool;
extern const char	*FluxDatatypeLong;
extern const char	*FluxDatatypeUnsignedLong;
extern const char	*FluxDatatypeDuration;
extern const char	*FluxBinaryDataTypeBase64;
extern const char	*FluxDatatypeDatetimeRFC3339;  
extern const char	*FluxDatatypeDatetimeRFC3339Nano;

// Base type for all specific flux types
class FluxBase {
protected:
    String _rawValue;
public:
    FluxBase(String rawValue);
    virtual ~FluxBase();
    String getRawValue() const { return _rawValue; }
    virtual const char *getType() = 0;
};

// Represents flux long
class FluxLong : public FluxBase {
public:
    FluxLong(String rawValue, long value);
    long value;
    virtual const char *getType() override;
};

// Represents flux unsignedLong
class FluxUnsignedLong : public FluxBase {
public:
    FluxUnsignedLong(String rawValue, unsigned long value);
    unsigned long value;
    virtual const char *getType() override;
};

// Represents flux double
class FluxDouble : public FluxBase {
public:
    FluxDouble(String rawValue, double value);
    double value;
    virtual const char *getType() override;
};

// Represents flux bool
class FluxBool : public FluxBase {
public:
    FluxBool(String rawValue, bool value);
    bool value;
    virtual const char *getType() override;
};

// Represents flux dateTime:RFC3339 and dateTime:RFC3339Nano
// Date and time are stored in classic struct tm.
// Fraction of second is stored in microseconds
// There are several classic functions for using struct tm: http://www.cplusplus.com/reference/ctime/ 
class FluxDateTime : public FluxBase {
protected:
    const char *_type;
public:    
    FluxDateTime(String rawValue, const char *type, struct tm value, unsigned long microseconds);
    // Struct tm for date and time
    struct tm value;
    // microseconds part
    unsigned long microseconds;
    // Formats the value part to string according to the given format. Microseconds are skipped.
    // Format string must be compatible with the http://www.cplusplus.com/reference/ctime/strftime/
    String format(String formatString);
    virtual const char *getType() override;
};

// Represents flux string, duration, base64binary
class FluxString : public FluxBase {
protected:
    const char *_type;
public:
    FluxString(String rawValue, const char *type);
    String value;
    virtual const char *getType() override;
};

/** 
 * FluxValue wraps a value from a flux query result column.
 * It provides getter methods for supported flux types:
 *  * getString() - string, base64binary or duration
 *  * getLong() - long
 *  * getUnsignedLong() - unsignedLong
 *  * getDateTime() - dateTime:RFC3339 or dateTime:RFC3339Nano
 *  * getBool() - bool
 *  * getDouble() - double
 * 
 * Calling improper type getter will result in zero (empty) value.
 * Check for null value usig isNull().
 * Use getRawValue() for getting original string form.
 * 
 **/

class FluxValue {
public:
    FluxValue();
    FluxValue(FluxBase *value);
    FluxValue(const FluxValue &other);
    FluxValue& operator=(const FluxValue& other);
    // Check if value represent null - not present - value.
    bool isNull();
    // Returns a value of string, base64binary or duration type column, or empty string if column is a different type.
    String getString();
    // Returns a value of long type column, or zero if column is a different type.
    long getLong();
    // Returns a value of unsigned long type column, or zero if column is a different type.
    unsigned long getUnsignedLong();
    // Returns a value of dateTime:RFC3339 or dateTime:RFC3339Nano, or zeroed FluxDateTime instance if column is a different type.
    FluxDateTime getDateTime();
    // Returns a value of bool type column, or false if column is a different type.
    bool getBool();
    // Returns a value of double type column, or 0.0 if column is a different type.
    double getDouble();
    // Returns a value in the original string form, as presented in the response.
    String getRawValue();
private:    
    std::shared_ptr<FluxBase> _data;
};

#endif //_FLUX_TYPES_H_