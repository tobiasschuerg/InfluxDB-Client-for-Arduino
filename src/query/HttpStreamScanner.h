/**
 * 
 * HttpStreamScanner.h:  Scannes HttpClient stream for lines. Supports chunking.
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
#ifndef _HTTP_STREAM_SCANNER_
#define _HTTP_STREAM_SCANNER_

#if defined(ESP8266)
# include <ESP8266HTTPClient.h>
#elif defined(ESP32)
# include <HTTPClient.h>
#endif //ESP8266

/** 
 * HttpStreamScanner parses response stream from  HTTPClient for lines.
 * By repeatedly calling next() it searches for new line.
 * If next() returns false, it can mean end of stream or an error.
 * Check getError() for nonzero if an error occured
 */ 
class HttpStreamScanner {
public:
    HttpStreamScanner(HTTPClient *client, bool chunked);
    bool next();
    void close();
    const String &getLine() const { return _line; }
    int getError() const { return _error; }
    int getLinesNum() const {return _linesNum; }
private:
    HTTPClient *_client;
    Stream *_stream = nullptr;
    int _len;
    String _line;
    int _linesNum= 0;
    int _read = 0;
    bool _chunked;
    bool _chunkHeader;
    int _chunkLen = 0;
    String _lastChunkLine;
    int _error = 0;
};

#endif //#_HTTP_STREAM_SCANNER_