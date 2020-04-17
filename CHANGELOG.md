# Changelog
## Version 3.x.x
 - [FIX] Compilation error on fields order (#43)
 - [FIX] Invalid precision constant for microseconds (#49)

## Version 3.1.1
- [Updated] CA Certificate for SSL (#38)

## Version 3.1.0
 - [NEW] Added User-agent header
 - [FIX] status code check when pinging an InfluxDB version 1.x instance

## Version 3.0.0
 - New API with similar keywords as other official InfluxDB clients
 - Richer set of data types for fields and timestamp methods
 - Advanced features, such as implicit batching, automatic retrying on server backpressure and connection failure, along with secured communication over TLS supported for both devices and authentication
 - Special characters escaping
 - Backward support for original API of V1/V2