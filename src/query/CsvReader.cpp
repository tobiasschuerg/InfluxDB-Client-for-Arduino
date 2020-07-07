/**
 * 
 * CsvReader.cpp: Simple Csv parser for comma separated values, with double quotes suppport
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
#include "CsvReader.h"

CsvReader::CsvReader(HttpStreamScanner *scanner) {
    _scanner = scanner;
}

CsvReader::~CsvReader() {
    delete _scanner;
}

std::vector<String> CsvReader::getRow() {
    return _row;
};

void CsvReader::close() {
    clearRow();
    _scanner->close();
}

void CsvReader::clearRow() {
    std::for_each(_row.begin(), _row.end(), [](String &value){ value = (const char *)nullptr; });
    _row.clear();
}

enum class CsvParsingState {
    UnquotedField,
    QuotedField,
    QuotedQuote
};

bool CsvReader::next() {
     clearRow();
     bool status = _scanner->next();
     if(!status) {
          _error =  _scanner->getError();
         return false;
     }
    String line = _scanner->getLine();
    CsvParsingState state = CsvParsingState::UnquotedField;
    std::vector<String> fields {""};
    size_t i = 0; // index of the current field
    for (char c : line) {
        switch (state) {
            case CsvParsingState::UnquotedField:
                switch (c) {
                    case ',': // end of field
                              fields.push_back(""); i++;
                              break;
                    case '"': state = CsvParsingState::QuotedField;
                              break;
                    default:  fields[i] += c;
                              break; 
                }
                break;
            case CsvParsingState::QuotedField:
                switch (c) {
                    case '"': state = CsvParsingState::QuotedQuote;
                              break;
                    default:  fields[i] += c;
                              break; 
                }
                break;
            case CsvParsingState::QuotedQuote:
                switch (c) {
                    case ',': // , after closing quote
                              fields.push_back(""); i++;
                              state = CsvParsingState::UnquotedField;
                              break;
                    case '"': // "" -> "
                              fields[i] += '"';
                              state = CsvParsingState::QuotedField;
                              break;
                    default:  // end of quote
                              state = CsvParsingState::UnquotedField;
                              break; 
                }
                break;
        }
    }
    _row = fields;
    return true;
}
