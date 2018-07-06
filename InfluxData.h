/**
    ESP8266 InfluxDb: InfluxData

    Purpose: Holds the data of a single measurement.

    @see
   https://docs.influxdata.com/influxdb/v1.5/concepts/glossary/#measurement

    @author Tobias Schürg
*/

class InfluxData {
 public:
  InfluxData(String measurement) : _measurement(measurement) {}

  void addTag(String key, String value) { _tags += "," + key + "=" + value; }
  void addValue(String key, float value) {
    _values = (_values == "") ? (" ") : (_values += ",");
    _values += key + "=" + String(value);
  }

  String toString() const { return _measurement + _tags + _values; }

 private:
  String _measurement;
  String _tags;
  String _values;
};
