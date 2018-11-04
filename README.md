# ESP8266_Influx_DB

Library for NodeMcu / ESP8266 (and Arduino?) for sending measurements to an Influx database.

## Initialization
```cpp
 #define INFLUXDB_HOST "192.168.0.32"
 #define INFLUXDB_PORT "1337"
 #define INFLUXDB_DATABASE "test"
 //if used with authentication
 #define INFLUXDB_USER "user"
 #define INFLUXDB_PASS "password"

 // connect to WiFi

 Influxdb influx(INFLUXDB_HOST); // port defaults to 8086
 // or to use a custom port
 Influxdb influx(INFLUXDB_HOST, INFLUXDB_PORT);

 // set the target database
 influx.setDb(INFLUXDB_DATABASE);
 // or use a db with auth
 influx.setDbAuth(INFLUXDB_DATABASE, INFLUXDB_USER, INFLUXDB_PASS) // with authentication
```

## Sending a single measurement
**Using an InfluxData object:**
```cpp
// create a measurement object
InfluxData measurement ("temperature");
measurement.addTag("device", d2);
measurement.addTag("sensor", "dht11");
measurement.addValue("value", 24.0);

// write it into db
influx.write(measurement);
```

**Using raw-data**
```cpp
 influx.write("temperature,device=d2,sensor=dht11 value=24.0")
```

## Write multiple data points at once
Batching measurements and send them with a single request will result in a much higher performance.
```cpp

InfluxData measurement1 = readTemperature()
influx.prepare(measurement1)

InfluxData measurement2 = readLight()
influx.prepare(measurement2)

InfluxData measurement3 = readVoltage()
influx.prepare(measurement3)

// writes all prepared measurements with a single request into db.
boolean success = influx.write();
```

## Http client error codes
Internally `ESP8266HTTPClient` is used.
```C
/// HTTP client errors
#define HTTPC_ERROR_CONNECTION_REFUSED  (-1)
#define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
#define HTTPC_ERROR_NOT_CONNECTED       (-4)
#define HTTPC_ERROR_CONNECTION_LOST     (-5)
#define HTTPC_ERROR_NO_STREAM           (-6)
#define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
#define HTTPC_ERROR_TOO_LESS_RAM        (-8)
#define HTTPC_ERROR_ENCODING            (-9)
#define HTTPC_ERROR_STREAM_WRITE        (-10)
#define HTTPC_ERROR_READ_TIMEOUT        (-11)
...
```
See [list of error codes](https://github.com/esp8266/Arduino/blob/cc0bfa04d401810ed3f5d7d01be6e88b9011997f/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L44-L55) and [list of http status codes](https://github.com/esp8266/Arduino/blob/cc0bfa04d401810ed3f5d7d01be6e88b9011997f/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.h#L60-L120).

## Documentation
For the documentation see [html/class_influxdb.html](html/class_influxdb.html) (only works locally).
