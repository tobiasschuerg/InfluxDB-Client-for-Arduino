# Changelog
## Version 3.3.0 (in progress)
 - [NEW] Added possibility skip server certification validation (`setInsecure()` method)
 - [NEW] Added possibility to query flux on InfuxDB 1.8 using V1 approach
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