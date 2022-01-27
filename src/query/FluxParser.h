/**
 * 
 * FLuxParser.h: InfluxDB flux query result parser
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
#ifndef _FLUX_PARSER_H_
#define _FLUX_PARSER_H_

#include <vector>
#include "CsvReader.h"
#include "FluxTypes.h"


/**
 * FluxQueryResult represents result from InfluxDB flux query.
 * It parses stream from server, line by line, so it allows to read a huge responses.
 * 
 * Browsing thought the result is done by repeatedly calling the next() method, until it returns false.
 * Unsuccesful reading is distinqushed by non empty value from getError().
 * 
 * As a flux query result can contain several tables differing by grouping key, use hasTableChanged() to
 * know when there is a new table.
 * 
 * Single values are returned using getValueByIndex() or getValueByName() methods.
 * All row values are retreived by getValues().
 * 
 * Always call close() at the of reading.
 * 
 * FluxQueryResult supports passing by value.
 */
class FluxQueryResult {
public:
    // Constructor for reading result
    FluxQueryResult(CsvReader *reader);
    // Constructor for error result
    FluxQueryResult(const String &error);
    // Copy constructor
    FluxQueryResult(const FluxQueryResult &other);
    // Assignment operator
    FluxQueryResult &operator=(const FluxQueryResult &other);
    // Advances to next values row in the result set.
    // Returns true on successful reading new row, false means end of the result set 
    // or an error. Call getError() and check non empty value
    bool next();
    // Returns index of the column, or -1 if not found
    int getColumnIndex(const String &columnName);
    // Returns a converted value by index, or nullptr in case of missing value or wrong index
    FluxValue getValueByIndex(int index);
    // Returns a result value by column name, or nullptr in case of missing value or wrong column name
    FluxValue getValueByName(const String &columnName);
    // Returns flux datatypes of all columns
    std::vector<String> getColumnsDatatype() { return _data->_columnDatatypes; }
    // Returns names of all columns
    std::vector<String> getColumnsName()  { return  _data->_columnNames; }
    // Returns all values from current row
    std::vector<FluxValue> getValues() { return  _data->_columnValues; }
    // Returns true if new table was encountered
    bool hasTableChanged() const { return  _data->_tableChanged; }
    // Returns current table position in the results set
    int getTablePosition() const { return _data->_tablePosition; }
    // Returns an error found during parsing if any, othewise empty string
    String getError() { return  _data->_error; }
    // Releases all resources and closes server reponse. It must be always called at end of reading.
    void close();
    // Descructor
    ~FluxQueryResult();
protected:
    FluxBase *convertValue(String &value, String &dataType);
    static FluxDateTime *convertRfc3339(String &value, const char *type);
    void clearValues();
    void clearColumns();
private:
    class Data {
    public:
        Data(CsvReader *reader);
        ~Data();
        CsvReader *_reader;
        int _tablePosition = -1;
        bool _tableChanged = false;
        std::vector<String> _columnDatatypes;
        std::vector<String> _columnNames;
        std::vector<FluxValue> _columnValues;
        String _error;
    };
    std::shared_ptr<Data> _data;
};

#endif //#_FLUX_PARSER_H_
