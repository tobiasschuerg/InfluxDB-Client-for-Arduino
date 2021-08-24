/**
 *  Sketch for running InfluxDBClient tests.
 *  For compiling in VSCode add path to workspace ("${workspaceFolder}\\**")
 *  Most of the tests require running mock server: cd test/server & node server.js. It will print ip adresses of available network interfaces.
 *  Modify INFLUXDB_CLIENT_TESTING_SERVER_HOST to set the correct mock server address.
 * 
 */

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#define INFLUXDB_CLIENT_TESTING_SERVER_HOST "192.168.88.142"
#define INFLUXDB_CLIENT_TESTING_ORG "my-org"
#define INFLUXDB_CLIENT_TESTING_BUC "my-bucket"
#define INFLUXDB_CLIENT_TESTING_DB "my-db"
#define INFLUXDB_CLIENT_TESTING_TOK "my-token"
#define INFLUXDB_CLIENT_TESTING_SSID "SSID"
#define INFLUXDB_CLIENT_TESTING_PASS "password"

#include "customSettings.h"

#define INFLUXDB_CLIENT_MANAGEMENT_URL "http://" INFLUXDB_CLIENT_TESTING_SERVER_HOST ":998"
#define INFLUXDB_CLIENT_TESTING_URL "http://" INFLUXDB_CLIENT_TESTING_SERVER_HOST ":999"
#define INFLUXDB_CLIENT_E2E_TESTING_URL "http://" INFLUXDB_CLIENT_TESTING_SERVER_HOST ":8086"

#include "TestSupport.h"
#include "Test.h"
#include "E2ETest.h"

void setup() {
#if defined(ESP8266)
    Serial.begin(74880);
#else
    Serial.begin(115200);
#endif    

    //Serial.setDebugOutput(true);
    randomSeed(123);

    Serial.println();

    initInet();

    Test::setup(INFLUXDB_CLIENT_MANAGEMENT_URL,INFLUXDB_CLIENT_TESTING_URL, INFLUXDB_CLIENT_E2E_TESTING_URL, INFLUXDB_CLIENT_TESTING_ORG, INFLUXDB_CLIENT_TESTING_BUC, INFLUXDB_CLIENT_TESTING_DB, INFLUXDB_CLIENT_TESTING_TOK );

    Serial.printf("Using server: %s\n", INFLUXDB_CLIENT_TESTING_URL);
}

void loop() {
    uint32_t startRAM = ESP.getFreeHeap();
    Serial.printf("Start RAM: %d\n", startRAM);
    time_t now = time(nullptr);
    Serial.print("Start time: ");
    Serial.println(ctime(&now));
    uint32_t start = millis();

    Test::run();
    E2ETest::run();

    now = time(nullptr);
    Serial.print("End time: ");
    Serial.print(ctime(&now));
    Serial.printf("  Took: %.1fs\n", (millis()-start)/1000.0f);
    uint32_t endRAM = ESP.getFreeHeap();
    Serial.printf("End RAM %d, diff: %d\n", endRAM, endRAM-startRAM);

    while(1) {
        delay(1000);
    }
}

void initInet() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);

    int i = 0,j = 0;
    bool wifiOk = false;
    while(!wifiOk && j<3) {
        Serial.print("Connecting to wifi ");
        WiFi.begin(INFLUXDB_CLIENT_TESTING_SSID, INFLUXDB_CLIENT_TESTING_PASS);
        while ((WiFi.status() != WL_CONNECTED) && (i < 30)) {
            Serial.print(".");
            delay(300);
            i++;
        }
        Serial.println();
        wifiOk = WiFi.status() == WL_CONNECTED;
        if(!wifiOk) {
            WiFi.disconnect();
        }
        j++;
    }
    if (!wifiOk) {
        Serial.println("Wifi connection failed");
        Serial.println("Restating");
        ESP.restart();
    } else {
        Serial.printf("Connected to: %s (%d)\n", WiFi.SSID().c_str(), WiFi.RSSI());
        Serial.print("Ip: ");
        Serial.println(WiFi.localIP());

        timeSync("CET-1CEST,M3.5.0,M10.5.0/3", "0.cz.pool.ntp.org", "1.cz.pool.ntp.org", "pool.ntp.org");

        deleteAll(INFLUXDB_CLIENT_TESTING_URL);
    }
}