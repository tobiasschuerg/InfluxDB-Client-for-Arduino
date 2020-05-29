#ifndef _TEST_SUPPORT_H_
#define _TEST_SUPPORT_H_

#define TEST_INIT(name) int temp = failures; const char *testName = name; do { Serial.println(testName)
#define TEST_END()  } while(0); Serial.printf("%s %s\n",testName,failures == temp?"SUCCEEDED":"FAILED")
#define TEST_ASSERT(a) if(testAssert(__LINE__, (a))) break
#define TEST_ASSERTM(a,m) if(testAssertm(__LINE__, (a),(m))) break

#include "query/FluxParser.h"

int failures = 0;

void printFreeHeap() {
  Serial.print("Free heap: ");  
  Serial.println(ESP.getFreeHeap());
}

bool deleteAll(String url) {
  String deleteUrl = url + "/api/v2/delete";
  HTTPClient http;
  int code = 0;
  if(http.begin(deleteUrl)) {
    code = http.POST("");
    http.end();
  }
  return code == 204;
}

bool serverLog(String url,String mess) {
  String logUrl = url + "/log";
  HTTPClient http;
  int code = 0;
  if(http.begin(logUrl)) {
    code = http.POST(mess);
    http.end();
  }
  return code == 204;
}


int countParts(String &str, char separator) {
  int lines = 0;
  int i,from = 0;
  while((i = str.indexOf(separator, from)) >= 0) {
    ++lines;
    from = i+1;
  }
  // try last part
  if(from < str.length() && str.substring(from).length()>0) {
    ++lines;
  }
  return lines;
}

String *getParts(String &str, char separator, int &count) {
  count = countParts(str, separator);
  String *ret = new String[count];
  int i,from = 0,p=0;
  while((i = str.indexOf(separator, from)) >= 0) {
    ret[p++] = str.substring(from,i);
    from = i+1;
  }
  // try last part
  if(from < str.length() && str.substring(from).length()>0) {
    ret[p] = str.substring(from);
  }
  return ret;
}

int countLines(FluxQueryResult flux) {
  int lines = 0;
  while(flux.next()) {
    lines++;
  }
  flux.close();
  return lines;
}

std::vector<String> getLines(FluxQueryResult flux) {
  std::vector<String> lines;
  while(flux.next()) {
    String line;
    int i=0;
    for(auto &val: flux.getValues()) {
      if(i>0) line += ",";
      line += val.getRawValue();
      i++;
    }
    lines.push_back(line);
  }
  flux.close();
  return lines;
}


bool testAssertm(int line, bool state,String message) {
  if(!state) {
    ++failures;
    Serial.printf("Assert failure line %d%s%s\n", line, message.length()>0?": ":"",message.c_str());
    return true;
  }
  return false;
}

bool testAssert(int line, bool state) {
  return testAssertm(line, state, "");
}

// Waits for server in desired state (up - true, down - false)
bool waitServer(InfluxDBClient &client, bool state) {
  int c = 0;
  bool res = false;
  while((res = client.validateConnection()) != state && c++ < 30) {
      Serial.printf("  Server is not %s\n", state?"up":"down");
      delay(1000);
  }
  return res == state;
}

bool compareTm(tm &tm1, tm &tm2) {
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    return difftime(t1, t2) == 0;
} 

#endif //_TEST_SUPPORT_H_