# InfluxDB 2 mock server

Mock server which simulates InfluxDB 1 and 2 write and query API.

First time, run: `yarn install` to download dependencies.

Run server: `yarn start`:

In query, it returns all written points, unless deleted. The results set had simple cvs form: measurement,tags, fields.

1st point in a batch if it has tag with name `direction` controls advanced behavior with value: 
 - `429-1` - reply with 429 status code and add Reply-After header with value 30
 - `429-2` - reply with 429 status
 - `503-1` - reply with 503 status code and add Reply-After header with value 10
 - `503-2` - reply with 503 status
 - `delete-all` - deletes all written points
