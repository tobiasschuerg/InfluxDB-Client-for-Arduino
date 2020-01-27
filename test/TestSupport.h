#ifndef _TEST_SUPPORT_H_
#define _TEST_SUPPORT_H_

#define TEST_INIT(name) int temp = failures; const char *testName = name; do { Serial.println(testName)
#define TEST_END()  } while(0); Serial.printf("%s %s\n",testName,failures == temp?"SUCCEEDED":"FAILED")
#define TEST_ASSERT(a) if(testAssert(__LINE__, (a))) break
#define TEST_ASSERTM(a,m) if(testAssertm(__LINE__, (a),(m))) break

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

int countLines(String &str) {
  return countParts(str, '\n');
}

String *getLines(String &str, int &count) {
  return getParts(str, '\n', count);
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

#endif //_TEST_SUPPORT_H_