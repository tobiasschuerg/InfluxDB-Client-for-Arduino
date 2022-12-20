/**
 *
 * helpers.cpp: InfluxDB Client util functions
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
#include <stdarg.h>
#include "helpers.h"
#include "config.h"

void timeSync(const char *tzInfo, const char* ntpServer1, const char* ntpServer2, const char* ntpServer3) {
  // Accurate time is necessary for certificate validion
#ifdef INFLUXDB_CLIENT_NET_ESP
  configTzTime(tzInfo,ntpServer1, ntpServer2, ntpServer3);

  // Wait till time is synced
  Serial.print("Syncing time");
  int i = 0;
  while (time(nullptr) < 1000000000l && i < 40) {
    Serial.print(".");
    delay(500);
    i++;
  }
  Serial.println();

  // Show time
  time_t tnow = time(nullptr);
  Serial.print("Synchronized time: ");
  Serial.println(ctime(&tnow));
#else
  Serial.println("Time synchronization not supported");
#endif
}

unsigned long long getTimeStamp(struct timeval *tv, int secFracDigits) {
    unsigned long long tsVal = 0;
    switch(secFracDigits) {
        case 0:
            tsVal = tv->tv_sec;
            break;
        case 6:
            tsVal = tv->tv_sec * 1000000LL + tv->tv_usec;
            break;
        case 9:
            tsVal = tv->tv_sec * 1000000000LL + tv->tv_usec * 1000LL;
            break;
        case 3:
        default:
            tsVal = tv->tv_sec * 1000LL + tv->tv_usec / 1000LL;
            break;

    }
    return tsVal;
}

char *timeStampToString(unsigned long long timestamp, int extraCharsSpace) {
    //22 is max long long string length (18)
    char *buff = new char[22+extraCharsSpace+1];
    
#ifdef INFLUXDB_CLIENT_NET_ESP   
    snprintf(buff, 22, "%llu", timestamp);
#else
    unsigned long high = timestamp /1000000000ll;
    unsigned long low = timestamp % 1000000000ll;
    snprintf(buff, 22, "%u%09u", high, low);
#endif 
    return buff;
}

static char escapeChars[] = "=\r\n\t ,";

char *escapeKey(const String &key, bool escapeEqual) {
    char c,*ret,*d,*s = (char *)key.c_str();
    int n = 0;
    while ((c = *s++)) {
        if(strchr(escapeEqual?escapeChars:escapeChars+1, c)) {
            n++;
        }
    }
    ret = new char[key.length()+n+1];
    s = (char *)key.c_str();
    d = ret;
    while ((c = *s++)) {
       if (strchr(escapeEqual?escapeChars:escapeChars+1,c)) {
           *d++ = '\\';
      }
      *d++ = c;
   }
   *d = 0;
   return ret;
}

String escapeValue(const char *value) {
    String ret;
    int len = strlen_P(value);
    ret.reserve(len+7); //5 is estimate of max chars needs to escape,
    ret += '"';
    for(int i=0;i<len;i++)
    {
        switch (value[i])
        {
            case '\\':
            case '\"':
                ret += '\\';
                break;
        }

        ret += value[i];
    }
    ret += '"';
    return ret;
}


static char invalidChars[] = "$&+,/:;=?@ <>#%{}|\\^~[]`";

static char hex_digit(char c) {
    return "0123456789ABCDEF"[c & 0x0F];
}

String urlEncode(const char* src) {
    int n=0;
    char c,*s = (char *)src;
    while ((c = *s++)) {
        if(strchr(invalidChars, c)) {
            n++;
        }
    }
    String ret;
    ret.reserve(strlen(src)+2*n+1);
    s = (char *)src;
    while ((c = *s++)) {
       if (strchr(invalidChars,c)) {
           ret += '%';
           ret += hex_digit(c >> 4);
           ret += hex_digit(c);
      }
      else ret += c;
   }
   return ret;
}

bool isValidID(const char *idString) {
   if(strlen(idString) != 16) {
     return false;
   }
   for(int i=0;i<16;i++) {
     //0-9,a-f
     if(!((idString[i] >= '0' && idString[i] <= '9') || (idString[i] >= 'a' && idString[i] <= 'f'))) {
         return false;
     }
   }
   return true;
}

const char *bool2string(bool val) {
    return (val?"true":"false");
}

uint8_t getNumLength(long long l) {
    uint8_t c = 0;
    do {
        c++;
        l /=10;
    } while(l);
    return c;
}

char *cloneStr(const char *str) {
  char *ret = new char[strlen(str)+1];
  strcpy(ret, str);
  return ret;
}

size_t strLen(const char *str) {
    if(!str) {
        return 0;
    }
    return strlen(str);
}

size_t Serialprintf(const char *format, ...)
{
    char loc_buf[64];
    char * temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
    va_end(copy);
    if(len < 0) {
        va_end(arg);
        return 0;
    };
    if(len >= sizeof(loc_buf)){
        temp = (char*) malloc(len+1);
        if(temp == NULL) {
            va_end(arg);
            return 0;
        }
        len = vsnprintf(temp, len+1, format, arg);
    }
    va_end(arg);
    len = Serial.write((uint8_t*)temp, len);
    if(temp != loc_buf){
        free(temp);
    }
    return len;
}

// TODO allow setting external callback to get a time
bool getAccurateTime(timeval *tv) {
#ifdef INFLUXDB_CLIENT_NET_ESP 
    gettimeofday(tv, NULL);
    return true;
#else
    unsigned long us = micros();
    tv->tv_sec = 1670416906 + us/1000000ul;
    tv->tv_usec = us%1000000ul;
    return false;
#endif    
}

bool parseURL(String url, String &host, int &port, String &path,String &user, String &pass) {
    int index = url.indexOf(':');
     if(index < 0) {
        return false;
    }
    String protocol = url.substring(0, index);
    url.remove(0, (index + 3)); // remove http:// or https://

    if (protocol == "http") {
        // set default port for 'http'
        port = 80;
    } else if (protocol == "https") {
        // set default port for 'https'
        port = 443;
    } else {
        return false;
    }
    index = url.indexOf('/');
    host = url.substring(0, index);
    url.remove(0, index); // remove host 
    path = url;
    // check Authorization
    index = host.indexOf('@');
    if(index >= 0) {
        user = host.substring(0, index);
        host.remove(0, index + 1); // remove auth part including @
        index = user.indexOf(':');
        if(index >=0) {
            pass = user.substring(index+1);
            user.remove(index);
        }
    }
    // get port
    index = host.indexOf(':');
    if(index >= 0) {
        String portS = host;
        host = host.substring(0, index); // hostname
        portS.remove(0, (index + 1)); // remove hostname + :
        port = portS.toInt(); // get port
    }
    return true;
}
