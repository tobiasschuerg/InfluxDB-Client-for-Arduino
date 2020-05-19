/**
 * Query Example code for InfluxDBClient library for Arduino
 * Enter WiFi and InfluxDB parameters below
 *
 * Demonstrates connection to any InfluxDB instance accesible via:
 *  - unsecured http://...
 *  - secure https://... (appropriate certificate is required)
 *  - InfluxDB 2 Cloud at https://cloud2.influxdata.com/ (certificate is preconfigured)
 * This example demonstrates querying basic statistic parameters of WiFi signal level measured and stored in BasicWrite and SecureWrite examples
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
#include <vector>
using namespace std;

// WiFi AP SSID
#define WIFI_SSID "SSID"
// WiFi password
#define WIFI_PASSWORD "PASSWORD"
// InfluxDB v2 server url, e.g. https://eu-central-1-1.aws.cloud2.influxdata.com (Use: InfluxDB UI -> Load Data -> Client Libraries)
#define INFLUXDB_URL "server-url"
// InfluxDB v2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
#define INFLUXDB_TOKEN "server token"
// InfluxDB v2 organization name or id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
#define INFLUXDB_ORG "org name/id"
// InfluxDB v2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
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

void timeSync() {
  // Synchronize UTC time with NTP servers
  // Accurate time is necessary for certificate validaton
  configTime(0, 0, "pool.ntp.org", "time.nis.gov");
  // Set timezone
  setenv("TZ", TZ_INFO, 1);

  // Wait till time is synced
  Serial.print("Syncing time");
  int i = 0;
  while (time(nullptr) < 1000000000ul && i < 100) {
    Serial.print(".");
    delay(100);
    i++;
  }
  Serial.println();

  // Show time
  time_t tnow = time(nullptr);
  Serial.print("Synchronized time: ");
  Serial.println(String(ctime(&tnow)));
}

void setup() {
  Serial.begin(115200);

  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  // Sync time for certificate validation
  timeSync();

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
  printQuery("max");
  // Get mean RSSI
  printQuery("mean");
  // Get min RSSI
  printQuery("min");

  //Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}

// printQuery queries db for aggregated RSSI value computed by given InfluxDB selector function (max, mean, min)
// Prints composed query, raw query result and parsed values
void printQuery(String selectorFunction) {
  // Construct a Flux query
  // Query will find RSSI for last 24 hours for each connected WiFi network with this device computed by given selector function
  String query = "from(bucket: \"" INFLUXDB_BUCKET "\") |> range(start: -24h) |> filter(fn: (r) => r._measurement == \"wifi_status\" and r._field == \"rssi\"";
  query += "and r.device == \"" DEVICE "\")";
  query += "|> " + selectorFunction + "()";

  Serial.print("Querying with: ");
  Serial.println(query);

  String resultSet = client.query(query);
  if (resultSet == "") {
    if (client.wasLastQuerySuccessful()) {
      Serial.println("Empty results set");
    } else {
      Serial.print("InfluxDB query failed: ");
      Serial.println(client.getLastErrorMessage());
    }
    return;
  }

  Serial.println("Query raw result:");
  Serial.println(resultSet);

  // Result set in CSV table, with header
  // First parse retuned string to lines
  vector<String> lines;
  getLines(resultSet, lines);
  // If result contains some lines it has some data
  if (lines.size() > 0) {
    // First parse header line to arrays of columns
    vector<String> columnNames;
    getColumns(lines[0], columnNames);
    // Find index of column named '_value', which contains value
    int valueColumnIndex = findItem(columnNames, "_value");
    // Find index of column named 'SSID', which is our tag with WiFi name
    int ssidColumnIndex = findItem(columnNames, "SSID");
    // If result set contains all we need
    if (valueColumnIndex != -1 && ssidColumnIndex != -1) {
      // Iterate over lines with data to get values
      Serial.print(selectorFunction);
      Serial.println("(RSSI):");
      bool first = true;
      for (String &line : lines) {
        // skip first line with header
        if (first) {
          first = false;
          continue;
        }
        // Parse next line into values
        vector<String> columnValues;
        getColumns(line, columnValues);
        Serial.print("  ");
        Serial.print(columnValues[ssidColumnIndex]);
        Serial.print(":");
        Serial.println(columnValues[valueColumnIndex]);
      }
    }
  }
}

// String utils

// findItem finds index of item in array of lenght len
// Returns index of string item in the array if found, otherwise -1
int findItem(vector<String> &array, String item) {
  vector<String>::iterator it = find(array.begin(), array.end(), item);
  if (it != array.end()) {
    return distance(array.begin(), it);
  }
  return -1;
}

// getParts function  splits string str by separator char
// Splited parts are added to the given array parts
// Handles escaped comma, not escaped double quotes
void getParts(String &str, char separator, vector<String> &parts) {
  int i, from = 0;
  while (from < str.length()) {
    i = str.indexOf(separator, from);
    String part;
    if (i >= 0) {
      part = str.substring(from, i);
    } else {
      part = str.substring(from);
    }
    part.trim();
    if (part.length() > 0) {
      //check escaped comma
      if (separator == ',' && part.startsWith("\"") && i >= 0) {
        //find closing double quote
        int x = str.indexOf('"', i + 1);
        if (x > 0) {
          part = str.substring(from + 1, x);
          // move pointer to next comma
          i = str.indexOf(separator, x + 1);
        }
      }
      parts.push_back(part);
    }
    if (i == -1) {
      break;
    } else {
      from = i + 1;
    }
  }
}

// getLines splits string str containing multiple lines into arrays of lines
void getLines(String &str, vector<String> &parts) {
  getParts(str, '\n', parts);
}

// getColumns splits string str containing comma separated strings into arrays of such strings
void getColumns(String &str, vector<String> &parts) {
  getParts(str, ',', parts);
}