
#include "TestSupport.h"

extern int failures;

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

bool waitServer(InfluxDBClient &client, bool state) {
  int c = 0;
  bool res = false;
  while((res = client.validateConnection()) != state && c++ < 30) {
      Serial.printf("  Server is not %s\n", state?"up":"down");
      delay(1000);
  }
  return res == state;
}

String queryFlux(String url, String token, String org, String &fluxQuery) {
    HTTPClient httpClient;
    String queryUrl = url + "/api/v2/query?org=" + org;
    if(!httpClient.begin(queryUrl)) {
        return "";
    }
    httpClient.addHeader(F("Content-Type"), F("application/vnd.flux"));
    
    httpClient.addHeader(F("Authorization"), "Token " + token);

    int statusCode = httpClient.POST(fluxQuery);
    String queryResult;
    if(statusCode == 200) {
        queryResult = httpClient.getString();
        queryResult.trim();
    }

    httpClient.end();

    return queryResult;
}