// Create express app
var express = require("express")
const bodyParser = require('body-parser');
var app = express()
const cors = require("cors");
const cmd = require('windosu') // executing cmds

// VUEJS
const vuePort = 44444
var vueServer = require("http").Server(app);
var io = require("socket.io")(vueServer, {
    cors: {
        origin: "*",
        methods: ["GET", "POST"]
    }
});
let vueSocket = undefined;
let vuesocket = io.on('connection', function (socket) {
    console.log("vuejs client connected");

    socket.on('disconnect', () => {
        console.log('vuejs client disconnected');
    });
    return socket;
});

vueServer.listen(vuePort, function () {
    console.log('Server listening for vueJS client on port ' + vuePort);
});

//C++
const cppPort = 33333
var net = require('net');
var cppServer = net.createServer(function (sock) {
    console.log('C++ client connected');

    // c++ sends struct{char name[32], ip[16];} -> arrives here as single string 'data'
    sock.on('data', function (data) {
        var msg = data.toString();
        //console.log(msg); //struct is now singlestr, can be multiple player sections at once
        vuesocket.emit("newPlayer", { "msg": msg }, Uint8Array.from([1]));
        var i;
        var n = msg.length;
        if (n % 54 != 0) {
            console.log("ERROR: msg length: " + n + "\n");
        }
        //get type of message {'del' or 'add' }
        var index = msg.indexOf("\u0000");
        var type = msg.slice(0, index);
        //console.log("TYPE: " +type)

        // extract name of player
        nameoffset = msg.slice(4);
        index = nameoffset.indexOf("\u0000");
        var name = nameoffset.slice(0, index);


        //get ip
        var ip = msg.slice(36, 52);
        index = ip.indexOf("\u0000");
        ip = ip.slice(0, index);

        // is player host?
        var isHost = msg.slice(52, 53);

        if (type == "add")
            vuesocket.emit("addPlayer", { "name": name, "ip": ip, "isHost": isHost }, Uint8Array.from([1]));
        else
            vuesocket.emit("delPlayer", { "name": name, "ip": ip, "isHost": isHost }, Uint8Array.from([1]));
        //console.log(type +": " +name +", ip: " +ip +", isHost: " +isHost);
    });

    sock.on('error', function (data) {
        console.log('C++ client CLOSED');
    });
});

cppServer.listen(cppPort, function () {
    console.log('Server listening for C++   client on port ' + cppPort);
});

//DB
var sqlite3 = require('sqlite3').verbose();
const DBSOURCE = "players.db"

// DB server port
var HTTP_PORT = 8000 

let db = new sqlite3.Database(DBSOURCE, (err) => {
    if (err) {
        // Cannot open database
        console.error(err.message)
        throw err
    } else {
        console.log('Connected to %s', DBSOURCE)
    }
});

app.use(bodyParser.json());
app.use(cors()); //for configuring Cross-Origin Resource Sharing (CORS)

// Root endpoint
app.get("/", (req, res, next) => {
    res.send("<h1>MW2 Kicktool</h1>");
});

// Get list of players in current session
app.get('/api/players/', (req, res, next) => {
    var sql = "SELECT * FROM players"
    var params = []
    return db.all(sql, params, (err, rows) => {
        if (err) {
            res.status(400).json({ "error": err.message });
            return;
        }
        res.status(200).send(rows);
    });
});

// Get list of kicked players
app.get('/api/history/', (req, res, next) => {
    var sql = "SELECT * FROM history"
    var params = []
    return db.all(sql, params, (err, rows) => {
        if (err) {
            res.status(400).json({ "error": err.message });
            return;
        }
        res.status(200).json(rows)
    });


});
// Get list of kicked players
app.get('/api/prison/', (req, res, next) => {
    var sql = "SELECT * FROM prison"
    var params = []
    return db.all(sql, params, (err, rows) => {
        if (err) {
            res.status(400).json({ "error": err.message });
            return;
        }
        res.status(200).json(rows)
    });

});

// move player into prison
app.post('/api/prison/', (req, res) => {
    const ip = req.body.data.ip
    const str = 'route add ' + ip + ' mask 255.255.255.255 1.3.3.7 if 1'
    cmd.exec(str)

    db.run("CREATE TABLE IF NOT EXISTS prison ("
        +"name TEXT NOT NULL,"
        +"ip TEXT PRIMARY KEY NOT NULL,"
        + "isHost TEXT NOT NULL);");
    var host = false;
    if (req.body.data.host == '1')
        host = true;
    db.run(`INSERT INTO prison ('name', 'ip', 'isHost') VALUES(?, ?, ?)`,
        [req.body.data.name, req.body.data.ip, host],
        function (err) {
            if (err) {
                console.log("ERR: " +err);
                res.status(400).json({ "error": err })
                return;
            }
            res.status(200).json({ "inserted": req.body })
        });
})

// move player to history
app.post('/api/history/', (req, res) => {

    db.run("CREATE TABLE IF NOT EXISTS history ("
        + "name TEXT NOT NULL,"
        + "ip TEXT PRIMARY KEY NOT NULL,"
        + "isHost TEXT NOT NULL);");

    db.run(`INSERT INTO history ('name', 'ip', 'isHost') VALUES(?, ?, ?)`,
        [req.body.data.name, req.body.data.ip, req.body.data.host],
        function (err) {
            if (err) {
                res.status(400).json({ "error": res.message })
                return;
            }
            res.status(200).json({ "inserted": req.body })
        });
})

// delets player with given ip and blocks his ip
app.delete('/api/players/:ip/', (req, res) => {
    db.run(
        'DELETE FROM players WHERE ip =(?)', req.params.ip,
        function (err, result) {
            if (err) {
                res.status(400).json({ "error": res.message })
                return;
            }
            res.status(200).json({ "deleted": req.params.ip })
        });
})

// delets player from history with given ip
app.delete('/api/history/:ip/', (req, res) => {
    db.run(
        'DELETE FROM history WHERE ip =(?)', req.params.ip,
        function (err, result) {
            if (err) {
                res.status(400).json({ "error": res.message })
                return;
            }
            res.status(200).json({ "deleted": req.params.ip })
        });
})

// frees prisoneer with given ip and unblocks his ip
app.delete('/api/prison/:ip/', (req, res) => {
    const ip = req.params.ip
    const str = 'route delete ' + ip + ' mask 255.255.255.255 1.3.3.7 if 1'
    cmd.exec(str)
    db.run(
        'DELETE FROM prison WHERE ip =(?)', req.params.ip,
        function (err, result) {
            if (err) {
                res.status(400).json({ "error": res.message })
                return;
            }
            res.status(200).json({ "deleted": req.params.ip })
        });
})

// Default response for any other request
app.use(function (req, res) {
    res.status(404);
});

// Start server
app.listen(HTTP_PORT, () => {
    console.log('Server listening for HTTP-requests on port ' + HTTP_PORT);
});