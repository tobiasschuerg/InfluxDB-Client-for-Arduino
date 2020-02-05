#ifndef _TEST_SUPPORT_H_
#define _TEST_SUPPORT_H_

#define TEST_INIT(name) int temp = failures; const char *testName = name; do { Serial.println(testName)
#define TEST_END()  } while(0); Serial.printf("%s %s\n",testName,failures == temp?"SUCCEEDED":"FAILED")
#define TEST_ASSERT(a) if(testAssert(__LINE__, (a))) break
#define TEST_ASSERTM(a,m) if(testAssertm(__LINE__, (a),(m))) break

#include <InfluxDBClient.h>

bool deleteAll(String url);

int countParts(String &str, char separator);

String *getParts(String &str, char separator, int &count);

int countLines(String &str);

String *getLines(String &str, int &count);


bool testAssertm(int line, bool state,String message);

bool testAssert(int line, bool state);

// Waits for server in desired state (up - true, down - false)
bool waitServer(InfluxDBClient &client, bool state);
// Sends Flux query and returns raw JSON formatted response
// Return raw query response in the form of CSV table. Empty string can mean that query hasn't found anything or an error. Check getLastStatusCode() for 200 
String queryFlux(String url, String token, String org, String &fluxQuery);

#endif //_TEST_SUPPORT_H_