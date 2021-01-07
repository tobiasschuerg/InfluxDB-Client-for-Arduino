/**
 * QueryAggregated Example code for InfluxDBClient library for Arduino.
 * 
 * This example demonstrates querying basic aggregated statistic parameters of WiFi signal level measured and stored in BasicWrite and SecureWrite examples.
 * 
 * Demonstrates connection to any InfluxDB instance accesible via:
 *  - unsecured http://...
 *  - secure https://... (appropriate certificate is required)
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


void loop() {
  // Get max RSSI
  printAgregateResult("max");
  // Get mean RSSI
  printAgregateResult("mean");
  // Get min RSSI
  printAgregateResult("min");

  //Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}

// printAgregateResult queries db for aggregated RSSI value computed by given InfluxDB selector function (max, mean, min)
// Prints composed query and the result values.
void printAgregateResult(String selectorFunction) {
  // Construct a Flux query
  // Query will find RSSI for last hour for each connected WiFi network with this device computed by given selector function
  String query = "from(bucket: \"" INFLUXDB_BUCKET "\") |> range(start: -1h) |> filter(fn: (r) => r._measurement == \"wifi_status\" and r._field == \"rssi\"";
  query += " and r.device == \""  DEVICE  "\")";
  query += "|> " + selectorFunction + "()";

  // Print ouput header
  Serial.print("==== ");
  Serial.print(selectorFunction);
  Serial.println(" ====");

  // Print composed query
  Serial.print("Querying with: ");
  Serial.println(query);

  // Send query to the server and get result
  FluxQueryResult result = client.query(query);

  // Iterate over rows. Even there is just one row, next() must be called at least once.
  while (result.next()) {
    // Get converted value for flux result column 'SSID'
    String ssid = result.getValueByName("SSID").getString();
    Serial.print("SSID '");
    Serial.print(ssid);

    Serial.print("' with RSSI ");
    // Get converted value for flux result column '_value' where there is RSSI value
    // RSSI is integer value and so on min and max selected results, 
    // whereas mean is computed and the result type is double.
    if(selectorFunction == "mean") {
      double value = result.getValueByName("_value").getDouble();
      Serial.print(value, 1);
      // computed value has not got a _time column, so omitting getting time here
    } else {
      long value = result.getValueByName("_value").getLong();
      Serial.print(value);

      // Get converted value for the _time column
      FluxDateTime time = result.getValueByName("_time").getDateTime();

      // Format date-time for printing
      // Format string according to http://www.cplusplus.com/reference/ctime/strftime/
      String timeStr = time.format("%F %T");

      Serial.print(" at ");
      Serial.print(timeStr);
    }

    
    Serial.println();
  }

  // Check if there was an error
  if(result.getError() != "") {
    Serial.print("Query result error: ");
    Serial.println(result.getError());
  }

  // Close the result
  result.close();
}

