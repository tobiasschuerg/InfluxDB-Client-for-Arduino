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
#include <AHttpClient.h>
#include <AWifi.h>
#include <time.h>

#include "TestSupport.h"
#include "util/helpers.h"



bool isNetworkAvailable() {
#ifdef INFLUXDB_CLIENT_HAVE_WIFI
  return WiFi.status() == WL_CONNECTED;
#else
  return false;
#endif
}

int httpPOST(const String &url, String mess) {
  int code = 0;
#ifdef INFLUXDB_CLIENT_NET_ESP
  HTTPClient httpClient;
  httpClient.setReuse(false);
  WiFiClient client;
  if(httpClient.begin(client, url)) {
    code = httpClient.POST(mess);
    httpClient.end();
  }
#else
  WiFiClient wifiClient;
  String host, path, user, pass;
  int port;
  parseURL(url, host, port, path, user, pass);
  HttpClient httpClient(wifiClient, host, port);
  code = httpClient.post(path, "text", mess);
  code = httpClient.responseStatusCode();
  httpClient.responseBody();
  httpClient.stop();
#endif
  return code;
}

int httpGET(const String &url) {
  int code = 0;
#ifdef INFLUXDB_CLIENT_NET_ESP
  HTTPClient httpClient;
  httpClient.setReuse(false);
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
#else
  WiFiClient wifiClient;
  String host, path, user, pass;
  int port;
  parseURL(url, host, port, path, user, pass);
  HttpClient httpClient(wifiClient, host, port);
  httpClient.get(path);
  code = httpClient.responseStatusCode();
  httpClient.responseBody();
  httpClient.stop();
#endif  
  return code;
}

bool deleteAll(const String &url) {
  if(isNetworkAvailable()) {
    return httpPOST(url + "/api/v2/delete","") == 204;
  }
  return false;
}

bool serverLog(const String &url, String mess) {
  if(isNetworkAvailable()) {
    return httpPOST(url + "/log", mess) == 204;
  } 
  return false;
}

bool isServerUp(const String &url) {
  if(isNetworkAvailable()) {
    return httpGET(url + "/status") == 200;
  }
  return false;
}


int countParts(const String &str, char separator) {
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

String *getParts(const String &str, char separator, int &count) {
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
///#if defined(ESP8266) || defined(ESP32)  
    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);
    return difftime(t1, t2) == 0;
// #else
//   return tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon && tm1.tm_wday == tm2.tm_wday && tm1.tm_hour == tm2.tm_hour
//   && tm1.tm_min == tm2.tm_min && tm1.tm_sec == tm2.tm_sec;
// #endif    
} 

bool waitServer(const String &url, bool state) {
    int i = 0;
    while(isServerUp(url) != state && i<10 ) {
        if(!i) {
            Serial.println(state?"[TD] Starting server":"[TD] Shuting down server");
            httpGET(url + (state?"/start":"/stop"));
        }
        delay(500);
        i++;
    }
    return isServerUp(url) == state;
}

#ifndef INFLUXDB_CLIENT_NET_ESP
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__
#endif //INFLUXDB_CLIENT_NET_ESP

int getFreeHeap() {
#ifdef INFLUXDB_CLIENT_NET_ESP
  return  ESP.getFreeHeap();
#else //Code for AVR a SAM D 
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
#endif  //ESP
}