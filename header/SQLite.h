#include <string>
#include <iostream>
#include <player.h>
#include <winsqlite/winsqlite3.h>

using namespace std;

class SQLite
{
private:
    // Pointer to SQLite connection
    sqlite3* db;

    // Save any error messages
    char* zErrMsg = 0;

    // Save the result of opening the file
    int rc;

    // Save any SQL
    string sql;
    string players, history, prison;

    // Compiled SQLite Statement
    sqlite3_stmt* statement;

    const char* data = "Callback function called";

    // Create a callback function  
    static int callback(void* data, int argc, char** argv, char** azColName) {

        //fprintf(stderr, "%s: \n", (const char*)data);

        // int argc: holds the number of results
        // (array) azColName: holds each column returned
        // (array) argv: holds each value

        for (int i = 0; i < argc; i++) {

            // Show column name, value, and newline
            cout << azColName[i] << ": " << argv[i] << endl;
        }

        // Insert a newline
        cout << endl;

        // Return successful
        return 0;
    }

    int checkDBErrors() {

        if (rc != SQLITE_OK) {
            // Show an error message
            cout << "DB Error: " << sqlite3_errmsg(db) << endl;

            closeDB();
            return 1;
        }
        return 0;
    }

public:
    SQLite() {
        rc = sqlite3_open_v2("../website/players.db", &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, NULL);
        checkDBErrors();
        createTables();
        clearPlayers();
    }

    int createTables() {
        // Save SQL to create a table
        players = "CREATE TABLE IF NOT EXISTS players ("  \
            "name TEXT NOT NULL," \
            "ip TEXT PRIMARY KEY NOT NULL," \
            "host INT NOT NULL);";

        history = "CREATE TABLE IF NOT EXISTS prison ("  \
            "name TEXT NOT NULL," \
            "ip TEXT PRIMARY KEY NOT NULL," \
            "host INT NOT NULL);";

        prison = "CREATE TABLE IF NOT EXISTS history ("  \
            "name TEXT NOT NULL," \
            "ip TEXT PRIMARY KEY NOT NULL," \
            "host INT NOT NULL);";

        // Run the SQL
        rc = sqlite3_exec(db, players.c_str(), callback, 0, &zErrMsg);
        if (checkDBErrors() != 0)
            return 1;
        rc = sqlite3_exec(db, history.c_str(), callback, 0, &zErrMsg);
        if (checkDBErrors() != 0)
            return 1;
        rc = sqlite3_exec(db, prison.c_str(), callback, 0, &zErrMsg);
        if (checkDBErrors() != 0)
            return 1;
        return 0;
    }

    int insertTable(Player player, std::string table) {

        string host = "0";
        if (player.host)
            host = "1";

        // Build a string using asprintf()
        string query = "INSERT INTO " + table + " ('name', 'ip', 'host') VALUES(";
        query += "'" + (string)player.name + "', ";
        query += "'" + (string)player.ip + "', ";
        query += host.c_str();
        query += ");";

        const char* ex = query.c_str();

        rc = sqlite3_exec(db, ex, callback, 0, &zErrMsg);
        checkDBErrors();
        return 0;
    }
    int clearPlayers() {
        const char* query = "DELETE FROM players";
        rc = sqlite3_exec(db, query, callback, 0, &zErrMsg);
        checkDBErrors();

        return 0;
    }

    int deletePlayer(string ip) {

        string query = "DELETE FROM 'players' WHERE ip = '" + ip + "';";
        const char* exe = query.c_str();

        rc = sqlite3_exec(db, exe, callback, 0, &zErrMsg);
        checkDBErrors();
        return 0;
    }

    int showTable() {

        // Save SQL insert data
        sql = "SELECT * FROM 'players';";

        // Run the SQL
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);
        checkDBErrors();
        return 0;
    }

    void closeDB() {
        // Close the SQL connection
        sqlite3_close(db);
    }
};