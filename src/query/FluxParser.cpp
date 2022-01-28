/**
 * 
 * FluxParser.cpp: InfluxDB flux query result parser
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

#include "FluxParser.h"
// Uncomment bellow in case of a problem and rebuild sketch
//#define INFLUXDB_CLIENT_DEBUG_ENABLE
#include "util/debug.h"

FluxQueryResult::FluxQueryResult(CsvReader *reader) {
    _data = std::make_shared<Data>(reader);
}

FluxQueryResult::FluxQueryResult(const String &error):FluxQueryResult((CsvReader *)nullptr) {
    _data->_error = error;
}

FluxQueryResult::FluxQueryResult(const FluxQueryResult &other) {
    _data = other._data;
}
FluxQueryResult &FluxQueryResult::operator=(const FluxQueryResult &other) {
    if(this != &other) {
        _data = other._data;
    }
    return *this;
}

FluxQueryResult::~FluxQueryResult() {
}

int FluxQueryResult::getColumnIndex(const String &columnName) {
    int i = -1;
    std::vector<String>::iterator it = find(_data->_columnNames.begin(), _data->_columnNames.end(), columnName);
    if (it != _data->_columnNames.end()) {
        i = distance(_data->_columnNames.begin(), it);
    }
    return i;
}

FluxValue FluxQueryResult::getValueByIndex(int index) {
    FluxValue ret;
    if(index >= 0 && index < (int)_data->_columnValues.size()) {
        ret = _data->_columnValues[index];
    }
    return ret;
}

FluxValue FluxQueryResult::getValueByName(const String &columnName) {
    FluxValue ret;
    int i = getColumnIndex(columnName);
    if(i > -1) {
        ret = getValueByIndex(i);
    }
    return ret;
}

void FluxQueryResult::close() {
    clearValues();
    clearColumns();
    if(_data->_reader) {
        _data->_reader->close();
    }
}

void FluxQueryResult::clearValues() {
    std::for_each(_data->_columnValues.begin(), _data->_columnValues.end(), [](FluxValue &value){ value = nullptr; });
    _data->_columnValues.clear();
}

void FluxQueryResult::clearColumns() {
    std::for_each(_data->_columnNames.begin(), _data->_columnNames.end(), [](String &value){ value = (const char *)nullptr; });
    _data->_columnNames.clear();

    std::for_each(_data->_columnDatatypes.begin(), _data->_columnDatatypes.end(), [](String &value){ value = (const char *)nullptr; });
    _data->_columnDatatypes.clear();
}

FluxQueryResult::Data::Data(CsvReader *reader):_reader(reader) {}

FluxQueryResult::Data::~Data() { 
    delete _reader;
}

enum ParsingState {
    ParsingStateNormal = 0, 
	ParsingStateNameRow,
	ParsingStateError
};

bool FluxQueryResult::next() {
    if(!_data->_reader) {
        return false;
    }
    ParsingState parsingState = ParsingStateNormal;
    _data->_tableChanged = false;
    clearValues();
    _data->_error = "";
readRow:
    bool stat = _data->_reader->next();
    if(!stat) {
        if(_data->_reader->getError()< 0) {
            _data->_error = HTTPClient::errorToString(_data->_reader->getError());
            INFLUXDB_CLIENT_DEBUG("Error '%s'\n", _data->_error.c_str());
        }
        return false;
    }
    std::vector<String> vals = _data->_reader->getRow();
    INFLUXDB_CLIENT_DEBUG("[D] FluxQueryResult: vals.size %d\n", vals.size());
    if(vals.size() < 2) {
        goto readRow;
    }
    if(vals[0] == "") {
		if (parsingState == ParsingStateError) {
			String message ;
			if (vals.size() > 1 && vals[1].length() > 0) {
				message = vals[1];
			} else {
				message = F("Unknown query error");
			}
			String reference = "";
            if (vals.size() > 2 && vals[2].length() > 0) {
				reference = "," + vals[2];
			}
			_data->_error =  message + reference;
            INFLUXDB_CLIENT_DEBUG("Error '%s'\n", _data->_error.c_str());
			return false;
		} else if (parsingState == ParsingStateNameRow) {
			if (vals[1] == "error") {
				parsingState = ParsingStateError;
			} else {
                if (vals.size()-1 != _data->_columnDatatypes.size()) {
                   _data->_error = String(F("Parsing error, header has different number of columns than table: ")) + String(vals.size()-1) + " vs " + String(_data->_columnDatatypes.size());
                   INFLUXDB_CLIENT_DEBUG("Error '%s'\n", _data->_error.c_str());
			       return false;
                } else {
                    for(unsigned int i=1;i < vals.size(); i++) {
                        _data->_columnNames.push_back(vals[i]);
                    }
                }
				parsingState = ParsingStateNormal;
			}
			goto readRow;
		}
		if(_data->_columnDatatypes.size() == 0) {
			_data->_error = F("Parsing error, datatype annotation not found");
            INFLUXDB_CLIENT_DEBUG("Error '%s'\n", _data->_error.c_str());
			return false;
		}
		if (vals.size()-1 != _data->_columnNames.size()) {
			_data->_error = String(F("Parsing error, row has different number of columns than table: ")) + String(vals.size()-1) + " vs " + String(_data->_columnNames.size());
            INFLUXDB_CLIENT_DEBUG("Error '%s'\n", _data->_error.c_str());
			return false;
		}
		for(unsigned int i=1;i < vals.size(); i++) {
            FluxBase *v  = nullptr;
            if(vals[i].length() > 0) {
                v = convertValue(vals[i], _data->_columnDatatypes[i-1]);
                if(!v) {
                    _data->_error = String(F("Unsupported datatype: ")) + _data->_columnDatatypes[i-1];
                    INFLUXDB_CLIENT_DEBUG("Error '%s'\n", _data->_error.c_str());
                    return false;
                }
            }  
            FluxValue val(v);
            _data->_columnValues.push_back(val);
		}
    } else if(vals[0] == "#datatype") {
		_data->_tablePosition++;
        clearColumns();
        _data->_tableChanged = true;
		for(unsigned int i=1;i < vals.size(); i++) {
			_data->_columnDatatypes.push_back(vals[i]);
		}
		parsingState = ParsingStateNameRow;
		goto readRow;
	} else {
        goto readRow;
    }
	return true;
}

FluxDateTime *FluxQueryResult::convertRfc3339(String &value, const char *type) {
    tm t = {0,0,0,0,0,0,0,0,0};
    // has the time part
    int zet = value.indexOf('Z');
    unsigned long fracts = 0;
    if(value.indexOf('T') > 0 && zet > 0) { //Full datetime string - 2020-05-22T11:25:22.037735433Z
        int f = sscanf(value.c_str(),"%d-%d-%dT%d:%d:%d", &t.tm_year,&t.tm_mon,&t.tm_mday, &t.tm_hour,&t.tm_min,&t.tm_sec);
        if(f != 6) {
            return nullptr;
        }
        t.tm_year -= 1900; //adjust to years after 1900
        t.tm_mon -= 1; //adjust to range 0-11
        int dot = value.indexOf('.');
        
        if(dot > 0) {
            int tail = zet;
            int len = zet-dot-1;
            if (len > 6) {
                tail = dot + 7; 
                len = 6;
            }
            String secParts = value.substring(dot+1, tail);
            fracts = strtoul((const char *) secParts.c_str(), NULL, 10);
            if(len < 6) {
                fracts *= 10^(6-len); 
            }
        }
    } else {
        int f = sscanf(value.c_str(),"%d-%d-%d", &t.tm_year,&t.tm_mon,&t.tm_mday);
        if(f != 3) {
            return nullptr;
        }
        t.tm_year -= 1900; //adjust to years after 1900
        t.tm_mon -= 1; //adjust to range 0-11
    }
    return new FluxDateTime(value, type, t, fracts);
}

FluxBase *FluxQueryResult::convertValue(String &value, String &dataType) {
    FluxBase *ret = nullptr;
    if(dataType.equals(FluxDatatypeDatetimeRFC3339) || dataType.equals(FluxDatatypeDatetimeRFC3339Nano)) {
        const char *type = FluxDatatypeDatetimeRFC3339;
        if(dataType.equals(FluxDatatypeDatetimeRFC3339Nano)) {
            type = FluxDatatypeDatetimeRFC3339Nano;
        }
        ret = convertRfc3339(value, type);
        if (!ret) {
            _data->_error = String(F("Invalid value for '")) + dataType + F("': ") + value;
        }
    } else if(dataType.equals(FluxDatatypeDouble)) {
        double val = strtod((const char *) value.c_str(), NULL);
        ret = new FluxDouble(value, val);
    } else if(dataType.equals(FluxDatatypeBool)) {
        bool val = value.equalsIgnoreCase("true");
        ret = new FluxBool(value, val);
    } else if(dataType.equals(FluxDatatypeLong)) {
        long l = strtol((const char *) value.c_str(), NULL, 10);
        ret = new FluxLong(value, l);
    } else if(dataType.equals(FluxDatatypeUnsignedLong)) {
        unsigned long ul = strtoul((const char *) value.c_str(), NULL, 10);
        ret = new FluxUnsignedLong(value, ul);
    } else if(dataType.equals(FluxBinaryDataTypeBase64)) {
        ret = new FluxString(value, FluxBinaryDataTypeBase64);
    } else if(dataType.equals(FluxDatatypeDuration)) {
        ret = new FluxString(value, FluxDatatypeDuration);
    } else if(dataType.equals(FluxDatatypeString)) {
        ret = new FluxString(value, FluxDatatypeString);
    }
    return ret;
}

