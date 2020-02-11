const express = require('express');
const readline = require('readline');
var os = require('os');

const app = express();
const port = 999;
var pointsdb = []; 

app.use (function(req, res, next) {
    var data='';
    req.setEncoding('utf8');
    req.on('data', function(chunk) { 
       data += chunk;
    });

    req.on('end', function() {
        req.body = parsePoints(data);
        next();
    });
});

app.get('/ready', (req,res) => {
    res.status(200).send("<html><body><h1>OK</h1></body></html>");
})

app.post('/api/v2/write', (req,res) => {
    if(checkWriteParams(req, res) && handleAuthentication(req, res)) {
        var points = req.body;
        if(Array.isArray(points) && points.length > 0) {
            var point = points[0];
            if(point.tags.hasOwnProperty('direction')) {
                switch(point.tags.direction) {
                    case '429-1':
                        res.set("Retry-After","30");
                        res.status(429).send("Limit exceeded"); 
                        break;
                    case '429-2':
                         res.status(429).send("Limit exceeded"); 
                        break;
                    case '503-1':
                        res.set("Retry-After","10");
                        res.status(503).send("Server overloaded"); 
                        break;
                    case '503-2':
                        console.log('Return');
                        res.status(503).send("Server overloaded"); 
                        break;
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

app.post('/api/v2/delete', (req,res) => {
    console.log('Deleteting points');
    pointsdb = [];
    res.status(204).end(); 
});

app.post('/api/v2/query', (req,res) => {
    if(checkQueryParams(req, res) && handleAuthentication(req, res)) {
        if(pointsdb.length > 0) {
            console.log('query: ' + pointsdb.length + ' points');
            res.status(200).send(convertToCSV(pointsdb));
        } else {
            res.status(200).end();
        }
    }
});

var rl = readline.createInterface(process.stdin, process.stdout);

rl.on('line', function(line) {
    rl.close();
}).on('close',function(){
    process.exit(0);
});

var server = app.listen(port)
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
console.log(`Listening on http://${server.address().address}:${server.address().port}`)
console.log(`Press Enter to exit`)


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
    if(auth != AuthToken) {
        res.status(401).send(`{"code":"unauthorized","message":"unauthorized access"}`);
        return false;
    } else {
        return true;
    }
}

function checkWriteParams(req, res) {
    var org = req.query['org'];
    var bucket = req.query['bucket'];
    if(org != 'my-org') {
        res.status(404).send(`{"code":"not found","message":"organization name \"${org}\" not found"}`);
        return false;
    } else if(bucket != 'my-bucket') {
        res.status(404).send(`{"code":"not found","message":"bucket \"${bucket}\" not found"}`);
        return false;
    } else {
        return true;
    }
}

function checkQueryParams(req, res) {
    var org = req.query['org'];
    if(org != 'my-org') {
        res.status(404).send(`{"code":"not found","message":"organization name \"${org}\" not found"}`);
        return false;
    } else {
        return true;
    }
}

function objectToCSV(obj, header) {
    var line = '';
    for (var index in obj) {
        if (line != '') line += ','
        if(typeof obj[index] == 'object') {
            line += objectToCSV(obj[index], header);
        } else if(header) {
            line += index;
        } else {
            line += obj[index];
        }
    }
    return line;
}

function convertToCSV(objArray) {
    var array = typeof objArray != 'object' ? JSON.parse(objArray) : objArray;
    var str = '';

    if(array.length > 0) {
        str = objectToCSV(array[0], true) + '\r\n';
    }

    for (var i = 0; i < array.length; i++) {
        var line = '';
        line = objectToCSV(array[i], false);
        
        str += line + '\r\n';
    }

    return str;
}