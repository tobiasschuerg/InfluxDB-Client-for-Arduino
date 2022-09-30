/**
 * 
 * TestSupport.h: Supporting functions for tests
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

#ifndef _TEST_SUPPORT_H_
#define _TEST_SUPPORT_H_

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include "query/FluxParser.h"

void printFreeHeap();

int httpPOST(const String &url, String mess);

int httpGET(const String &url);

bool deleteAll(const String &url) ;

bool serverLog(const String &url, String mess);

bool isServerUp(const String &url);


int countParts(const String &str, char separator);

String *getParts(const String &str, char separator, int &count);

int countLines(FluxQueryResult flux) ;

std::vector<String> getLines(FluxQueryResult flux);


bool compareTm(tm &tm1, tm &tm2);
// Waits for server in desired state (up - true, down - false)
bool waitServer(const String &url, bool state);

#endif //_TEST_SUPPORT_H_
