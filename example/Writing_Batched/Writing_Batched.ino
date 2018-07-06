
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <InfluxDb.h>

#define INFLUXDB_HOST "192.168.0.32"
#define WIFI_SSID "xxx"
#define WIFI_PASS "xxx"

ESP8266WiFiMulti WiFiMulti;
Influxdb influx(INFLUXDB_HOST);

void setup() {
  Serial.begin(9600);
  Serial.println(" ### Hello ###");

  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WIFI");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  influx.setDb("test");

  Serial.println("Setup done");
}


int loopCount = 0;

void loop() {
  loopCount++;

  InfluxData m1 = measure(1);
  influx.prepare(m1);

  InfluxData m2 = measure(2);
  influx.prepare(m2);

  InfluxData m3 = measure(3);
  influx.prepare(m3);

  // only with this call all prepared measurements are sent
  influx.write();

  delay(5000);
}

/**
   Just create a random measurement.
*/
InfluxData measure(int number) {
  InfluxData row("temperature");
  row.addTag("device", "alpha");
  row.addTag("sensor", "one");
  row.addTag("mode", "pwm");
  row.addValue("number", number);
  row.addValue("loopCount", loopCount);
  row.addValue("value", random(10, 40));
  return row;
}
