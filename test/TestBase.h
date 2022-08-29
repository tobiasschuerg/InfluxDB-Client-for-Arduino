/**
 * 
 * AbstractTest.h: Test base for tests for InfluxDB client for Arduino
 * 
 * MIT License
 * 
 * Copyright (c) 2021 InfluxData
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
#ifndef _ABSTRACT_TEST_H_
#define _ABSTRACT_TEST_H_

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include "TestSupport.h"

#define TEST_INIT(name)  int temp = failures;\
 const char *testName = name; \
 serverLog(TestBase::managementUrl, name); \
  do { \
  Serial.println(testName)
#define TEST_END()  } while(0); \
 end: Serial.printf("%s %s\n",testName,failures == temp?"SUCCEEDED":"FAILED")
#define TEST_ASSERT(a) if(testAssert(__LINE__, (a))) goto end
#define TEST_ASSERTM(a,m) if(testAssertm(__LINE__, (a),(m))) goto end

class TestBase {
public:
    static const char * managementUrl;
    static const char * apiUrl;
    static const char * e2eApiUrl;
    static const char * orgName;
    static const char * bucketName;
    static const char * dbName;
    static const char * token;
    static int failures;
public:
    static void setup(const char * mgmtUrl, const char * apiUrl, const char *e2eApiUrl, const char * orgName, const char * bucketName, const char * dbName, const char * token);
    static Point *createPoint(const String &measurement);
};

bool testAssertm(int line, bool state,const String &message);
bool testAssert(int line, bool state);

#endif //_E2E_TEST_H_
