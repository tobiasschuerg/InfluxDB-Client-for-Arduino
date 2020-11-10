const express = require('express');
const readline = require('readline');
var os = require('os');

const app = express();
const mgmtApp = express();
const port = 999;
const mgmtPort = 998;
var pointsdb = []; 
var lastUserAgent = '';
var chunked = false;
var delay = 0;
var permanentError = 0;
const prefix = '';
var server = undefined;

mgmtApp.use (function(req, res, next) {
    var data='';
    req.setEncoding('utf8');
    req.on('data', function(chunk) { 
       data += chunk;
    });

    req.on('end', function() {
        req.body = data;
        next();
    });
});

mgmtApp.get('/start', (req,res) => {
    if(server === undefined) {
        console.log('Starting server');
        server = app.listen(port);
        server.on('close',function() {
            pointsdb = [];
            server = undefined;
            console.log('Server closed');
        });
        res.status(201).send(`Listening on http://${server.address().address}:${server.address().port}`);
    } else {
        res.status(204).end();
    };
});

mgmtApp.get('/stop', (req,res) => {
    if(server === undefined) {
        res.status(404).end();
    } else {
        console.log('Shutting down server');
        server.close();
        res.status(200).end();
    };
});
mgmtApp.get('/status', (req,res) => {
    if(server === undefined) {
        res.status(404).send("stopped");
    } else {
        res.status(200).send("running");
    }
});

mgmtApp.post('/log', (req,res) => {
    console.log('===' + req.body + '=====');
    res.status(204).end();
})


app.use (function(req, res, next) {
    var data='';
    req.setEncoding('utf8');
    req.on('data', function(chunk) { 
       data += chunk;
    });

    req.on('end', function() {
        req.body = data;
        next();
    });
});
app.get(prefix + '/test/user-agent', (req,res) => {
    res.status(200).send(lastUserAgent);
})
app.get(prefix + '/ready', (req,res) => {
    lastUserAgent = req.get('User-Agent');
    res.status(200).send("<html><body><h1>OK</h1></body></html>");
})
app.get(prefix + '/health', (req,res) => {
    lastUserAgent = req.get('User-Agent');
    res.status(200).send("<html><body><h1>OK</h1></body></html>");
})

app.get(prefix + '/ping', (req,res) => {
    lastUserAgent = req.get('User-Agent');
    if(req.query['verbose'] == 'true') {
        res.status(200).send("<html><body><h1>OK</h1></body></html>");
    } else {
        res.status(204).end();
    }
})


app.post(prefix + '/api/v2/write', (req,res) => {
    chunked = false;
    if(checkWriteParams(req, res) && handleAuthentication(req, res)) {
        //console.log('Write');
        //console.log(req.body);
        var points = parsePoints(req.body);
        if(permanentError > 0) {
            console.log('Pernament error ' + permanentError);
            res.status(permanentError).send("Internal server error"); 
        }
        if(Array.isArray(points) && points.length > 0) {
            var point = points[0];
            if(point.tags.hasOwnProperty('direction')) {
                if(permanentError > 0) {
                    if(point.tags.direction == 'permanent-unset' ) {
                        permanentError = 0;
                    }
                } else {
                    switch(point.tags.direction) {
                        case '429-1':
                            console.log('Limit exceeded');
                            res.set("Retry-After","10");
                            console.log('Retry-After 10');
                            res.status(429).send("Limit exceeded"); 
                            break;
                        case '429-2':
                            console.log('Limit exceeded');
                            console.log('Retry-After default');
                            res.status(429).send("Limit exceeded"); 
                            break;
                        case '503-1':
                            res.set("Retry-After","10");
                            console.log('Server overloaded');
                            console.log('Retry-After 10');
                            res.status(503).send("Server overloaded"); 
                            break;
                        case '503-2':
                            console.log('Server overloaded');
                            console.log('Retry-After default');
                            res.status(503).send("Server overloaded"); 
                            break;
                        case 'delete-all':
                            pointsdb = [];
                            res.status(204).end(); 
                            break; 
                        case 'status':
                            points = [];
                            const code = parseInt(point.tags['x-code']);
                            console.log("Set code: " + code);
                            res.status(code).send("bad request");
                            break;
                        case 'chunked':
                            chunked = true;
                            console.log("Set chunked = true");
                            break;
                        case 'timeout':
                            delay = parseInt(point.tags.timeout)*1000;
                            console.log("Set delay: " + delay);
                            break;
                        case 'permanent-set':
                            permanentError = parseInt(point.tags['x-code']);
                            console.log("Set permanentError: " + permanentError);
                            res.status(permanentError).send("bad request");
                            break;
                    }
                    
                }
                points.shift();
            }
            console.log("write " + points.length + ' points');
            points.forEach((item, index) => {
                pointsdb.push(item);
            })
            if(res.statusCode < 299) {
                res.status(204).end();  
            }
        } else {
            res.status(204).end();
        }
    }
    if(res.statusCode != 204) {
        console.log('Responded with ' + res.statusCode);
    }
})

app.post(prefix + '/write', (req,res) => {
    if(checkWriteParamsV1(req, res) ) {
        var points = parsePoints(req.body);
        if(Array.isArray(points) && points.length > 0) {
            var point = points[0];
            if(point.tags.hasOwnProperty('direction')) {
                switch(point.tags.direction) {
                    case 'delete-all':
                        pointsdb = [];
                        res.status(204).end(); 
                        break; 
                    case '400':
                        points = [];
                        res.status(400).send("bad request");
                        break;
                    case '500':
                        points = [];
                        res.status(500).send("internal server error");
                        break;
                }
                points.shift();
            }
            console.log("write " + points.length + ' points');
            points.forEach((item, index) => {
                pointsdb.push(item);
            })
            if(res.statusCode < 299) {
                res.status(204).end();  
            }
        } else {
            res.status(204).end();
        }
    }
    if(res.statusCode != 204) {
        console.log('Responded with ' + res.statusCode);
    }
})

app.post(prefix + '/api/v2/delete', (req,res) => {
    console.log('Deleteting points');
    pointsdb = [];
    res.status(204).end(); 
});

var queryRes = {
    "singleTable":`#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,double,string,string,string,string
,result,table,_start,_stop,_time,_value,_field,_measurement,a,b
,,0,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T10:34:08.135814545Z,1.4,f,test,1,adsfasdf
,,1,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T22:08:44.850214724Z,6.6,f,test,3,adsfasdf
\r 
`,
    "nil-value": `#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,double,string,string,string,string
,result,table,_start,_stop,_time,_value,_field,_measurement,a,b
,,0,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T10:34:08.135814545Z,,f,test,1,adsfasdf
,,0,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T22:08:44.850214724Z,6.6,f,test,,adsfasdf
,,0,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T22:11:32.225467895Z,1122.45,f,test,3,
\r
`,
"multiTables":`#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,unsignedLong,string,string,string,string
,result,table,_start,_stop,_time,_value,_field,_measurement,a,b
,_result,0,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T10:34:08.135814545Z,14,f,test,1,adsfasdf
,_result,0,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T22:08:44.850214724Z,66,f,test,1,adsfasdf
\r
#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,long,string,string,string,string
,result,table,_start,_stop,_time,_value,_field,_measurement,a,b
,_result1,1,2020-02-16T22:19:49.747562847Z,2020-02-17T22:19:49.747562847Z,2020-02-17T10:34:08.135814545Z,-4,i,test,1,adsfasdf
,_result1,1,2020-02-16T22:19:49.747562847Z,2020-02-17T22:19:49.747562847Z,2020-02-16T22:08:44.850214724Z,-1,i,test,1,adsfasdf
\r
#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,boolean,string,string,string,string
,result,table,_start,_stop,_time,_value,_field,_measurement,a,b
,_result2,2,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T10:34:08.135814545Z,false,b,test,0,brtfgh
,_result2,2,2020-02-17T22:19:49.747562847Z,2020-02-18T22:19:49.747562847Z,2020-02-18T22:08:44.969100374Z,true,b,test,0,brtfgh
\r
#datatype,string,long,dateTime:RFC3339Nano,dateTime:RFC3339Nano,dateTime:RFC3339Nano,duration,string,string,string,base64Binary
,result,table,_start,_stop,_time,_value,_field,_measurement,a,b
,_result3,3,2020-02-10T22:19:49.747562847Z,2020-02-12T22:19:49.747562847Z,2020-02-11T10:34:08.135814545Z,1d2h3m4s,d,test,0,eHh4eHhjY2NjY2NkZGRkZA==
,_result3,3,2020-02-10T22:19:49.747562847Z,2020-02-12T22:19:49.747562847Z,2020-02-12T22:08:44.969100374Z,22h52s,d,test,0,ZGF0YWluYmFzZTY0
\r
`,
"diffNum-data":`#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,long,string,duration,base64Binary,dateTime:RFC3339
,result,table,_start,_stop,_time,deviceId,sensor,elapsed,note,start
,,0,2020-04-28T12:36:50.990018157Z,2020-04-28T12:51:50.990018157Z,2020-04-28T12:38:11.480545389Z,1467463,BME280,1m1s,ZGF0YWluYmFzZTY0,2020-04-27T00:00:00Z,2345234
\r
`,
"diffNum-type-vs-header":`#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,long,string,duration,base64Binary,dateTime:RFC3339
,result,table,_start,_stop,_time,deviceId,sensor,elapsed,note
,,0,2020-04-28T12:36:50.990018157Z,2020-04-28T12:51:50.990018157Z,2020-04-28T12:38:11.480545389Z,1467463,BME280,1m1s,ZGF0YWluYmFzZTY0,2020-04-27T00:00:00Z
\r
`,
"flux-error":`{"code":"invalid","message":"compilation failed: loc 4:17-4:86: expected an operator between two expressions"}`,
"invalid-datatype":`#datatype,string,long,dateTime:RFC3339,dateTime:RFC3339,dateTime:RFC3339,int,string,duration,base64Binary,dateTime:RFC3339
,result,table,_start,_stop,_time,deviceId,sensor,elapsed,note,start
,,0,2020-04-28T12:36:50.990018157Z,2020-04-28T12:51:50.990018157Z,2020-04-28T12:38:11.480545389Z,1467463,BME280,1m1s,ZGF0YWluYmFzZTY0,2020-04-27T00:00:00Z
,,0,2020-04-28T12:36:50.990018157Z,2020-04-28T12:51:50.990018157Z,2020-04-28T12:39:36.330153686Z,1467463,BME280,1h20m30.13245s,eHh4eHhjY2NjY2NkZGRkZA==,2020-04-28T00:00:00Z
\r
`,
"missing-datatype":`,result,table,_start,_stop,_time,_value,_field,_measurement,a,b
,_result3,3,2020-02-10T22:19:49.747562847Z,2020-02-12T22:19:49.747562847Z,2020-02-11T10:34:08.135814545Z,1d2h3m4s,d,test,0,eHh4eHhjY2NjY2NkZGRkZA==
,_result3,3,2020-02-10T22:19:49.747562847Z,2020-02-12T22:19:49.747562847Z,2020-02-12T22:08:44.969100374Z,22h52s,d,test,0,eHh4eHhjY2NjY2NkZGRkZA==
\r
`,
"error-it-row-full":`#datatype,string,string
,error,reference
,failed to create physical plan: invalid time bounds from procedure from: bounds contain zero time,897
\r
`,
"error-it-row-no-reference":`#datatype,string,string
,error,reference
,failed to create physical plan: invalid time bounds from procedure from: bounds contain zero time,
\r
`,
"error-it-row-no-message":`#datatype,string,string
,error,reference
,,
\r
`,
"empty":``
};

function sleep(milliseconds) {
    const date = Date.now();
    let currentDate = null;
    do {
      currentDate = Date.now();
    } while (currentDate - date < milliseconds);
  }

app.post(prefix+'/api/v2/query', (req,res) => {
    //console.log("Query with: " + req.body);
    if(checkQueryParams(req, res) && handleAuthentication(req, res)) {
        var queryObj = JSON.parse(req.body);
        var data = '';
        var status = 200;
        if (queryObj["query"].startsWith('testquery-')) {
            var qi =  queryObj["query"].substring(10) ;
            console.log('query: ' + qi + ' dataset');
            if(qi.endsWith('error')) {
                status = 400;
            }
            data = queryRes[qi];
        } else if(pointsdb.length > 0) {
            console.log('query: ' + pointsdb.length + ' points');
            data = convertToCSV(pointsdb);
        }
        if(data.length > 0) {
            if(delay) {
                sleep(delay);
                delay = 0;
            }
            //console.log(data);

            if(chunked) {
                var i = data.length/3;
                res.set("Transfer-Encoding","chunked");
                res.status(status);
                res.write(data.substring(0, i+1));
                res.write(data.substring(i+1, 2*i+1));
                res.write(data.substring(2*i+1));
                res.end();
                chunked = false;
            } else {
                res.status(status).send(data);
            }
        } else {
            res.status(status).end();
        }
    }
});

function parsePoints(data) {
    var lines = data.split("\n");
    var points = [];
    lines.forEach((line, index) => {
        var parts = line.split(" ");
        if (parts.length > 1) {
            var measTags = parts[0];
            var fields = parts[1].split(",");
            var point = {};
            var keys = measTags.split(",");
            point.measurement = keys[0];
            point.tags = {};
            point.fields = {};
            if (keys.length > 1) {
                for (var i = 1; i < keys.length; i++) {
                    var keyval = keys[i].split("=");
                    point.tags[keyval[0]] = keyval[1];
                }
            }
            for (var i = 0; i < fields.length; i++) {
                var keyval = fields[i].split("=");
                var value = keyval[1];
                if (typeof value === 'string' && value.endsWith("i")) {
                    value = value.substring(0, value.length - 1);
                }
                point.fields[keyval[0]] = value;
            }
            if(parts.length>2) {
                point.timestamp = parts[2];
            }
            points.push(point);
        }
    });
    if (points.length > 0) {
        return points;
    }
    else {
       return data;
    }
}

const AuthToken = "Token 1234567890";
function handleAuthentication(req, res) {
    var auth = req.get('Authorization');
    if(auth && auth != AuthToken) {
        res.status(401).send(`{"code":"unauthorized","message":"unauthorized access"}`);
        return false;
    } else {
        return true;
    }
}

const AllowedPrecisions = ['ns','us','ms','s'];
function checkWriteParams(req, res) {
    var org = req.query['org'];
    var bucket = req.query['bucket'];
    var precision = req.query['precision'];
    if(org != 'my-org') {
        res.status(404).send(`{"code":"not found","message":"organization name \"${org}\" not found"}`);
        return false;
    } else if(bucket != 'my-bucket') {
        res.status(404).send(`{"code":"not found","message":"bucket \"${bucket}\" not found"}`);
        return false;
    } else if(typeof precision !== 'undefined' && AllowedPrecisions.indexOf(precision)==-1) {
        res.status(400).send(`{"code":"bad request ","message":"precision \"${precision}\" is not valid"}`);
        return false;
    } else {
        return true;
    }
}

const AllowedPrecisionsV1 = ['ns','u','ms','s'];
function checkWriteParamsV1(req, res) {
    var db = req.query['db'];
    var user = req.query['u'];
    var pass = req.query['p'];
    var precision = req.query['precision'];
    if(db != 'my-db') {
        res.status(404).send(`{"code":"not found","message":"database \"${db}\" not found"}`);
        return false;
    } else if(typeof precision !== 'undefined' && AllowedPrecisionsV1.indexOf(precision)==-1) {
        res.status(400).send(`precision \"${precision}\" is not valid`);
        return false;
    } else if (user !== 'user' && pass != 'my secret pass') {
        res.status(401).send("unauthorized")
    } else {
        return true;
    }
}

function checkQueryParams(req, res) {
    var org = req.query['org'];
    if(org && org !== 'my-org') {
        res.status(404).send(`{"code":"not found","message":"organization name \"${org}\" not found"}`);
        return false;
    } else {
        return true;
    }
}

function objectToCSV(obj, type, level) {
    var line = '';
    if(level == 1) line = type==0?'#datatype,':',';
    var i = 0;
    for (var index in obj) {
        if (i>0) line += ',';
        if(typeof obj[index] == 'object') {
            line += objectToCSV(obj[index], type, level+1);
        } else if(type == 0) { //datatype header
            line += 'string';
        } else if(type == 1) {
            line += index;
        } else {
            line += obj[index];
        }
        i++;
    }
    return line;
}

function convertToCSV(objArray) {
    var array = typeof objArray != 'object' ? JSON.parse(objArray) : objArray;
    var str = '';

    if(array.length > 0) {
        str = objectToCSV(array[0], 0, 1) + '\r\n';
        str += objectToCSV(array[0], 1, 1) + '\r\n';
    }

    for (var i = 0; i < array.length; i++) {
        var line = '';
        line = objectToCSV(array[i], 2, 1);
        
        str += line + '\r\n';
    }

    return str;
}

var mgmtServer = mgmtApp.listen(mgmtPort)

var rl = readline.createInterface(process.stdin, process.stdout);

rl.on('line', function(line) {
    rl.close();
}).on('close',function(){
    if(server !== undefined) {
        server.close();
    }
    mgmtServer.close();
    process.exit(0);
});


var ifaces = os.networkInterfaces();

console.log("Available interfaces:")
Object.keys(ifaces).forEach(function (ifname) {
  var alias = 0;

  ifaces[ifname].forEach(function (iface) {
    if ('IPv4' !== iface.family || iface.internal !== false) {
      // skip over internal (i.e. 127.0.0.1) and non-ipv4 addresses
      return;
    }

    if (alias >= 1) {
      // this single interface has multiple ipv4 addresses
      console.log('  ', ifname + ':' + alias, iface.address);
    } else {
      // this interface has only one ipv4 adress
      console.log('  ', ifname, iface.address);
    }
    ++alias;
  });
});
console.log(`Listening on http://${mgmtServer.address().address}:${mgmtServer.address().port}`)
console.log(`Press Enter to exit`)
