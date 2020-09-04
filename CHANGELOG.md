# Changelog
## Version 3.4.0 (in progres)
### Features
 - [#89](https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/pull/89) - ESP8266 only - Added Max Fragment Length Negotiation for TLS communicaton to reduce memory allocation. If server supports MFLN, it saves ~15kB. Standalone InfluxDB OSS server doesn't support MFLN, Cloud yes. To leverage MFLN for standalone OSS, a reverse proxy needs to be used. 
 - [#91](https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/pull/91) - Improved API for settings of write and HTTP options:
    - Introduced `WriteOptions` to wrap the write related options (write precision, batch-size, etc). It offers fluent style API allowing to change only the required options. `InfluxDBClient` has overloaded `setWriteOptions(const WriteOptions& writeOptions)` method.
    - Introduced `HTTPOptions` to wrap the HTTP related options (e.g. reusing connection). It offers fluent style API allowing to change only the required options. `InfluxDBClient` has `setHTTPOptions(const HTTPOptions& httpOptions)` method.
    - Added possibility to set HTTP response read timeout (part of the `HTTPOptions`).
    - Method `InfluxDBClient::void setWriteOptions(WritePrecision precision, uint16_t batchSize = 1, uint16_t bufferSize = 5, uint16_t flushInterval = 60, bool preserveConnection = true)` is deprecated and it will be removed in the next release.
 - [#92](https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/pull/92) - Write logic improvements 
   - Retry on failure logic unification with other InfluxDB clients (exponencial retry, max retry count 3, max retry interval)
   - Better write buffer memory management

### Documentation
 - [#87](https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/pull/87) - Fixed include file name in the Readme

### Fixes
 - [#90](https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino/pull/90) - Fixed boolean type recognition of InfluxDB Flux

## Version 3.3.0 (2020-07-07)
 - [NEW] Added possibility skip server certification validation (`setInsecure()` method)
 - [NEW] Added possibility to query flux on secured InfuxDB 1.8 using V1 approach
 - [NEW] `validateConnection()` can be used also for the [forward compatibility](https://docs.influxdata.com/influxdb/latest/tools/api/#influxdb-2-0-api-compatibility-endpoints) connection to InfluxDB 1.8
 - [FIX] More precice default timestamp generating, up to microseconds
 - [FIX] Debug compilation error
 - [FIX] SecureBatchWrite compile error
 
## Version 3.2.0 (2020-06-09)
- [NEW] Added possibility to read data from InfluxDB using Flux queries
- [NEW] `timeSync` utility function for synchronous time synchronization using NTP 
- [FIX] Properly initialize member variable (#59)
- [FIX] ASCII chars & compilation warning fix (#60)
- [Update] ESP8266 SDK 2.7+ required

## Version 3.1.3 (2020-04-27)
 - [FIX] SecureWrite crash (#54)
 
## Version 3.1.2 (2020-04-18)
 - [FIX] Compilation error on fields order (#43)
 - [FIX] Invalid precision constant for microseconds (#49)
 - [FIX] Write error in case point has no tags (#50)

## Version 3.1.1 (2020-04-06)
 - [Updated] CA Certificate for SSL (#38)

## Version 3.1.0 (2020-03-12)
 - [NEW] Added User-agent header
 - [FIX] status code check when pinging an InfluxDB version 1.x instance

## Version 3.0.0 (2020-02-11)
 - New API with similar keywords as other official InfluxDB clients
 - Richer set of data types for fields and timestamp methods
 - Advanced features, such as implicit batching, automatic retrying on server backpressure and connection failure, along with secured communication over TLS supported for both devices and authentication
 - Special characters escaping
 - Backward support for original API of V1/V2