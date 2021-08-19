/**
 * 
 * TestSupport.cpp: Supporting functions for tests
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

#include <Arduino.h>
#if defined(ESP8266)
# include <ESP8266HTTPClient.h>
#elif defined(ESP32)
# include <HTTPClient.h>
#endif

#include "TestSupport.h"

static HTTPClient httpClient;

void printFreeHeap() {
  Serial.print("[TD] Free heap: ");  
  Serial.println(ESP.getFreeHeap());
}

int httpPOST(String url, String mess) {
  httpClient.setReuse(false);
  int code = 0;
  WiFiClient client;
  if(httpClient.begin(client, url)) {
    code = httpClient.POST(mess);
    httpClient.end();
  }
  return code;
}

int httpGET(String url) {
  httpClient.setReuse(false);
  int code = 0;
  WiFiClient client;
  if(httpClient.begin(client, url)) {
    code = httpClient.GET();
    if(code != 204) {
       //Serial.print("[TD] ");
       //String res = http.getString();
       //Serial.println(res);
    }
    httpClient.end();
  }
  return code;
}

bool deleteAll(String url) {
  return httpPOST(url + "/api/v2/delete","") == 204;
}

bool serverLog(String url, String mess) {
  return httpPOST(url + "/log", mess) == 204;
}

bool isServerUp(String url) {
    return httpGET(url + "/status") == 200;
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


bool compareTm(tm &tm1, tm &tm2) {
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    return difftime(t1, t2) == 0;
} 

bool waitServer(const char *url, bool state) {
    int i = 0;
    while(isServerUp(url) != state && i<10 ) {
        if(!i) {
            Serial.println(state?"[TD] Starting server":"[TD] Shuting down server");
            httpGET(String(url) + (state?"/start":"/stop"));
        }
        delay(500);
        i++;
    }
    return isServerUp(url) == state;
}