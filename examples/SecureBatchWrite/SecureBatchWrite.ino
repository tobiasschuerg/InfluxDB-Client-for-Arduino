/**
 * Secure Write Example code for InfluxDBClient library for Arduino
 * Enter WiFi and InfluxDB parameters below
 *
 * Demonstrates connection to any InfluxDB instance accesible via:
 *  - unsecured http://...
 *  - secure https://... (appropriate certificate is required)
 *  - InfluxDB 2 Cloud at https://cloud2.influxdata.com/ (certificate is preconfigured)
 * Measures signal level of all visible WiFi networks including signal level of the actually connected one
 * This example demonstrates time handling, how to write measures with different priorities, batching and retry
 * Data can be immediately seen in a InfluxDB 2 Cloud UI - measurements wifi_status and wifi_networks
 **/

#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#define WIFI_AUTH_OPEN ENC_TYPE_NONE
#endif

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID "SSID"
// WiFi password
#define WIFI_PASSWORD "PASSWORD"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "server-url"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "server token"
// InfluxDB v2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
#define INFLUXDB_ORG "org id"
// InfluxDB v2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
#define INFLUXDB_BUCKET "bucket name"
// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time:   "PST8PDT"
//  Eastern:        "EST5EDT"
//  Japanesse:      "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
// NTP servers the for time synchronization.
// For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
#define NTP_SERVER1  "pool.ntp.org"
#define NTP_SERVER2  "time.nis.gov"
#define WRITE_PRECISION WritePrecision::S
#define MAX_BATCH_SIZE 10
#define WRITE_BUFFER_SIZE 30

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
// InfluxDB client instance without preconfigured InfluxCloud certificate for insecure connection 
//InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

// Data point
Point sensorStatus("wifi_status");

// Number for loops to sync time using NTP
int iterations = 0;

void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Add tags
  sensorStatus.addTag("device", DEVICE);
  sensorStatus.addTag("SSID", WiFi.SSID());

  // Alternatively, set insecure connection to skip server certificate validation 
  //client.setInsecure();

  // Accurate time is necessary for certificate validation and writing in batches
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  // Enable messages batching and retry buffer
  client.setWriteOptions(WriteOptions().writePrecision(WRITE_PRECISION).batchSize(MAX_BATCH_SIZE).bufferSize(WRITE_BUFFER_SIZE));
}

void loop() {
  // Sync time for batching once per hour
  if (iterations++ >= 360) {
    timeSync(TZ_INFO, NTP_SERVER1, NTP_SERVER2);
    iterations = 0;
  }

  // Report networks (low priority data) just in case we successfully wrote the previous batch
  if (client.isBufferEmpty()) {
    // Report all the detected wifi networks
    int networks = WiFi.scanNetworks();
    // Set identical time for the whole network scan
    time_t tnow = time(nullptr);
    for (int i = 0; i < networks; i++) {
      Point sensorNetworks("wifi_networks");
      sensorNetworks.addTag("device", DEVICE);
      sensorNetworks.addTag("SSID", WiFi.SSID(i));
      sensorNetworks.addTag("channel", String(WiFi.channel(i)));
      sensorNetworks.addTag("open", String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN));
      sensorNetworks.addField("rssi", WiFi.RSSI(i));
      sensorNetworks.setTime(tnow);  //set the time

      // Print what are we exactly writing
      Serial.print("Writing: ");
      Serial.println(client.pointToLineProtocol(sensorNetworks));

      // Write point into buffer - low priority measures
      client.writePoint(sensorNetworks);
    }
  } else
    Serial.println("Wifi networks reporting skipped due to communication issues");

  // Report RSSI of currently connected network
  sensorStatus.setTime(time(nullptr));
  sensorStatus.addField("rssi", WiFi.RSSI());

  // Print what are we exactly writing
  Serial.print("Writing: ");
  Serial.println(client.pointToLineProtocol(sensorStatus));

  // Write point into buffer - high priority measure
  client.writePoint(sensorStatus);

  // Clear fields for next usage. Tags remain the same.
  sensorStatus.clearFields();

  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
  }

  // End of the iteration - force write of all the values into InfluxDB as single transaction
  Serial.println("Flushing data into InfluxDB");
  if (!client.flushBuffer()) {
    Serial.print("InfluxDB flush failed: ");
    Serial.println(client.getLastErrorMessage());
    Serial.print("Full buffer: ");
    Serial.println(client.isBufferFull() ? "Yes" : "No");
  }

  // Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}