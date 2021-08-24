#include "TestBase.h"

#if defined(ESP32)
#include <WiFi.h>
String chipId = String((unsigned long)ESP.getEfuseMac());
String deviceName = "ESP32";
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
String chipId = String(ESP.getChipId());
String deviceName = "ESP8266";
#endif

const char * TestBase::managementUrl;
const char * TestBase::apiUrl;
const char * TestBase::e2eApiUrl;
const char * TestBase::orgName;
const char * TestBase::bucketName;
const char * TestBase::dbName;
const char * TestBase::token;

int TestBase::failures = 0;

void TestBase::setup(const char * mgmtUrl, const char * apiUrl, const char *e2eApiUrl, const char * orgName, const char * bucketName, const char * dbName, const char * token) {
    TestBase::managementUrl = mgmtUrl;
    TestBase::apiUrl = apiUrl;
    TestBase::e2eApiUrl = e2eApiUrl;
    TestBase::orgName = orgName;
    TestBase::bucketName = bucketName;
    TestBase::dbName = dbName;
    TestBase::token = token;
}

Point *TestBase::createPoint(String measurement) {
    Point *point = new Point(measurement);
    point->addTag("SSID", WiFi.SSID());
    point->addTag("device_name", deviceName);
    point->addTag("device_id", chipId);
    point->addField("temperature", random(-20, 40) * 1.1f);
    point->addField("humidity", random(10, 90));
    point->addField("code", random(10, 90));
    point->addField("door", random(0, 10) > 5);
    point->addField("status", random(0, 10) > 5 ? "ok" : "failed");
    return point;
}

bool testAssertm(int line, bool state,String message) {
  if(!state) {
    ++TestBase::failures;
    Serial.printf("Assert failure line %d%s%s\n", line, message.length()>0?": ":"",message.c_str());
    return true;
  }
  return false;
}

bool testAssert(int line, bool state) {
  return testAssertm(line, state, "");
}