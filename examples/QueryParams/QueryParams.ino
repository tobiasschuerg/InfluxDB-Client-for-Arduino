/**
 * QueryParams Example code for InfluxDBClient library for Arduino.
  * 
 * This example demonstrates querying using parameters inserted into the Flux query. We select WiFi signal level values bellow a certain threshold. 
 * WiFi signal is measured and stored in BasicWrite and SecureWrite examples.
 * 
 * Demonstrates connection to any InfluxDB instance accesible via:
 *  - InfluxDB 2 Cloud at https://cloud2.influxdata.com/ (certificate is preconfigured)
 * 
 *  Enter WiFi and InfluxDB parameters below
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
#include <InfluxDbCloud.h>

// WiFi AP SSID
#define WIFI_SSID "SSID"
// WiFi password
#define WIFI_PASSWORD "PASSWORD"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
// InfluxDB 1.8+  (v2 compatibility API) server url, e.g. http://192.168.1.48:8086
#define INFLUXDB_URL "server-url"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
// InfluxDB 1.8+ (v2 compatibility API) use form user:password, eg. admin:adminpass
#define INFLUXDB_TOKEN "server token"
// InfluxDB v2 organization name or id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
// InfluxDB 1.8+ (v2 compatibility API) use any non empty string
#define INFLUXDB_ORG "org name/id"
// InfluxDB v2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
// InfluxDB 1.8+ (v2 compatibility API) use database name
#define INFLUXDB_BUCKET "bucket name"

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time: "PST8PDT"
//  Eastern: "EST5EDT"
//  Japanesse: "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"

// InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

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

  
  // Accurate time is necessary for certificate validation
  // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}


// Queries WiFi signal level values bellow a certain threshold using parameters inserted into the Flux query
// Prints composed query and the result values.
void loop() {
  // Prepare query parameters
  QueryParams params;
  params.add("bucket", INFLUXDB_BUCKET);
  params.add("since", "-5m");
  params.add("device", DEVICE);
  params.add("rssiTreshold", -50);
  
  // Construct a Flux query using parameters
  // Parameters are accessed via the 'params' Flux object
  // Flux only supports only string, float and int as parameters. Duration can be converted from string.
  // Query will find RSSI less than defined treshold
  String query = "from(bucket: params.bucket) |> range(start: duration(v: params.since)) \
    |> filter(fn: (r) => r._measurement == \"wifi_status\") \
    |> filter(fn: (r) => r._field == \"rssi\") \
    |> filter(fn: (r) => r.device == params.device) \
    |> filter(fn: (r) => r._value < params.rssiTreshold)";

  // Print ouput header
  // Print composed query
  Serial.print("Querying with: ");
  Serial.println(query);

  // Send query to the server and get result
  FluxQueryResult result = client.query(query, params);

  //Print header
  Serial.printf("%10s %20s %5s\n","Time","SSID","RSSI");

  for(int i=0;i<37;i++) {
    Serial.print('-');
  }
  Serial.println();

  // Iterate over rows. Even there is just one row, next() must be called at least once.
  int c = 0;
  while (result.next()) {
    // Get converted value for flux result column 'SSID'
    String ssid = result.getValueByName("SSID").getString();
   
    // Get converted value for flux result column '_value' where there is RSSI value
    long rssi = result.getValueByName("_value").getLong();

    // Get converted value for the _time column
    FluxDateTime time = result.getValueByName("_time").getDateTime();

    // Format date-time for printing
    // Format string according to http://www.cplusplus.com/reference/ctime/strftime/
    String timeStr = time.format("%F %T");
    // Print formatted row
    Serial.printf("%20s %10s %5d\n", timeStr.c_str(), ssid.c_str() ,rssi);
    c++;
  }
  if(!c) {
    Serial.println(" No data found");
  }

  // Check if there was an error
  if(result.getError() != "") {
    Serial.print("Query result error: ");
    Serial.println(result.getError());
  }

  // Close the result
  result.close();
  // Wait 15s
  delay(15000);
}
