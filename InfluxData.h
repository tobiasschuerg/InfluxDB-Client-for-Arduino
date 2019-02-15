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
  void setTime(long int value)
  {
    _time = " " + String(value) + "000000000";
  }

  String toString() const { return _measurement + _tags + _values + _time; }

 private:
  String _measurement;
  String _tags;
  String _values;
  String _time;
};
