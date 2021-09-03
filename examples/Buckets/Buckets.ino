/**
 * Buckets management Example code for InfluxDBClient library for Arduino
 * Enter WiFi and InfluxDB parameters below
 *
 * This example supports only InfluxDB running from unsecure (http://...)
 * For secure (https://...) or Influx Cloud 2 connection check SecureWrite example to 
 * see how connect using secured connection (https)
 **/

#if defined(ESP32)
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32"
#elif defined(ESP8266)
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
#define DEVICE "ESP8266"
#endif

#include <InfluxDbClient.h>

// WiFi AP SSID
#define WIFI_SSID "ssid"
// WiFi password
#define WIFI_PASSWORD "password"
// InfluxDB  server url. Don't use localhost, always server name or ip address.
// E.g. http://192.168.1.48:8086 (In InfluxDB 2 UI -> Load Data -> Client Libraries), 
#define INFLUXDB_URL "influxdb-url"
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
// This token must have all buckets permission
#define INFLUXDB_TOKEN "toked-id"
// InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
#define INFLUXDB_ORG "org"
// Bucket name that doesn't exist in the db yet
#define INFLUXDB_BUCKET "test-bucket"

void setup() {
  Serial.begin(74880);

  // Connect WiFi
  Serial.println("Connecting to " WIFI_SSID);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
}

// Creates client, bucket, writes data, verifies data and deletes bucket
void testClient() {
  // InfluxDB client instance
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
    return;
  }

  // Get dedicated client for buckets management
  BucketsClient buckets = client.getBucketsClient();
  
  // Verify bucket does not exist, or delete it
  if(buckets.checkBucketExists(INFLUXDB_BUCKET)) {
    Serial.println("Bucket " INFLUXDB_BUCKET " already exists, deleting" );
    // get reference
    Bucket b = buckets.findBucket(INFLUXDB_BUCKET);
    // Delete bucket
    buckets.deleteBucket(b.getID());
  } 

  // create a bucket with retention policy one month. Leave out or set zero to infinity
  uint32_t monthSec = 30*24*3600;
  Bucket b = buckets.createBucket(INFLUXDB_BUCKET, monthSec);
  if(!b) {
    // some error occurred
    Serial.print("Bucket creating error: ");
    Serial.println(buckets.getLastErrorMessage());
    return;
  }
  Serial.print("Created bucket: ");
  Serial.println(b.toString());
  
  int numPoints = 10;
  // Write some points
  for(int i=0;i<numPoints;i++) {
    Point point("test");
    point.addTag("device_name", DEVICE);
    point.addField("temperature", random(-20, 40) * 1.1f);
    point.addField("humidity", random(10, 90));
    if(!client.writePoint(point)) {
      Serial.print("Write error: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
  // verify written points
  String query= "from(bucket: \"" INFLUXDB_BUCKET "\") |> range(start: -1h) |> pivot(rowKey:[\"_time\"],columnKey: [\"_field\"],valueColumn: \"_value\") |> count(column: \"humidity\")";
  FluxQueryResult result = client.query(query);
  // We expect one row
  if(result.next()) { 
    // Get count value
    FluxValue val = result.getValueByName("humidity");
    if(val.getLong() != numPoints) {
      Serial.print("Test failure, expected ");
      Serial.print(numPoints);
      Serial.print(" got ");
      Serial.println(val.getLong());
    } else {
      Serial.println("Test successfull");
    }
    // Advance to the end
    result.next();
  } else {
    Serial.print("Query error: ");
    Serial.println(result.getError());
  };
  result.close();
      
  buckets.deleteBucket(b.getID());
}

void loop() {
  // Lets do an E2E test
  // call a client test
  testClient();
  
  Serial.println("Stopping");
  // Stop here, don't loop
  while(1) delay(1);
}
