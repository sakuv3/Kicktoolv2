// Create express app
var express = require("express")
const bodyParser = require('body-parser');
var app = express()
const cors = require("cors");
//DB
var sqlite3 = require('sqlite3').verbose();
const DBSOURCE = "players.db"
// Server port
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

    db.run("CREATE TABLE IF NOT EXISTS prison ("
        +"name TEXT NOT NULL,"
        +"ip TEXT PRIMARY KEY NOT NULL,"
        + "host INT NOT NULL);");

    db.run(`INSERT INTO prison ('name', 'ip', 'host') VALUES(?, ?, ?)`,
        [req.body.data.name, req.body.data.ip, req.body.data.host],
        function (err) {
            if (err) {
                res.status(400).json({ "error": res.message })
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
        + "host INT NOT NULL);");

    db.run(`INSERT INTO history ('name', 'ip', 'host') VALUES(?, ?, ?)`,
        [req.body.data.name, req.body.data.ip, req.body.data.host],
        function (err) {
            if (err) {
                res.status(400).json({ "error": res.message })
                return;
            }
            res.status(200).json({ "inserted": req.body })
        });
})

// delets player with given ip
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

// delets prisoneer with given ip
app.delete('/api/prison/:ip/', (req, res) => {
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
    console.log("Server running on port %PORT%".replace("%PORT%", HTTP_PORT))
});