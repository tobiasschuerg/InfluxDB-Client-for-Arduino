# ESP8266_Influx_DB

Library for NodeMcu / ESP8266 (and Arduino?) for sending measurements to an Influx database.

## Initialization
```cpp
 #define INFLUXDB_HOST "192.168.0.32"
 #define INFLUXDB_PORT "1337"
 #define INFLUXDB_DATABASE "test"

 // TODO: connect to WiFi

 Influxdb influx(INFLUXDB_HOST); // port defaults to 8086
 // or
 Influxdb influx(INFLUXDB_HOST, INFLUXDB_PORT);

 // set the target database
 influx.setDb(INFLUXDB_DATABASE);
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

## Documentation
For the documentation see [html/class_influxdb.html](html/class_influxdb.html) (only works locally).
