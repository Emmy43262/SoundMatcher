#include "db.h"
#include <string>
#include <iostream>

DB::DB()
{
    init_db(&db);
}

DB::~DB()
{
    sqlite3_close(db);
}

void DB::add_song(Song &song)
{
    std::string sql = "INSERT INTO songs (title, author) VALUES (?, ?);";
    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, song.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, song.author.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE)
    {
        std::cerr << "Error inserting song: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return;
    }

    song.id = sqlite3_last_insert_rowid(db);
    std::cout << "Inserted song with ID: " << song.id << "\n";

    sqlite3_finalize(stmt);
}

void DB::diplay_songs()
{
    std::cout << "Displaying songs in the database:\n";
    const char *sql = "SELECT * FROM songs;";
    char *err = 0;

    auto callback = [](void *NotUsed, int argc, char **argv, char **colName) -> int
    {
        for (int i = 0; i < argc; i++)
        {
            std::cout << colName[i] << ": " << (argv[i] ? argv[i] : "NULL") << "\n";
        }
        std::cout << "\n";
        return 0;
    };

    sqlite3_exec(db, sql, callback, 0, &err);
    if (err)
    {
        std::cerr << "Error displaying songs: " << err << "\n";
        exit(1);
    }
}

void DB::init_db(sqlite3 **database)
{
    std::cout << "Initializing database...\n";
    int rc = sqlite3_open("sound_matcher.db", database);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(*database) << "\n";
        exit(1);
    }

    const char *song_table = "CREATE TABLE IF NOT EXISTS songs (id INTEGER PRIMARY KEY, title TEXT, author TEXT);";
    char *err = 0;
    rc = sqlite3_exec(*database, song_table, nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error creating songs table: " << err << "\n";
        exit(1);
    }

    const char *fingerprint_table = "CREATE TABLE IF NOT EXISTS fingerprints (hash BIGINT, song_id INTEGER, time REAL, FOREIGN KEY(song_id) REFERENCES songs(id) PRIMARY KEY(hash, song_id));";
    rc = sqlite3_exec(*database, fingerprint_table, nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error creating fingerprints table: " << err << "\n";
        exit(1);
    }
    std::cout << "Database initialized successfully.\n";
}
