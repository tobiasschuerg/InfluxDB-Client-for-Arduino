/**
 * 
 * InfluxDb.h: InfluxDB Client for Arduino
 * 
 * MIT License
 * 
 * Copyright (c) 2018-2020 Tobias Schürg
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
#include "InfluxData.h"

class Influxdb : public InfluxDBClient {
 public:
  Influxdb(String host, uint16_t port = 8086);

  void setDb(String db);
  void setDbAuth(String db, String user, String pass);

  void setVersion(uint16_t version);
  void setBucket(String bucket);
  void setOrg(String org);
  void setToken(String token);
  void setPort(uint16_t port);
#if defined(ESP8266)
  void setFingerPrint(const char *fingerPrint);
#endif

  void prepare(InfluxData data);
  boolean write();

  boolean write(InfluxData data);
  boolean write(String data);

 private:
  uint16_t _preparedPoints;
  
  void begin();
};
