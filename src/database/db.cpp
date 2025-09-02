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

    sqlite3_finalize(stmt);
}

void DB::add_hashes(int song_id, song_hash_map &hashes)
{
    std::string sql = "INSERT INTO fingerprints (hash, song_id, time) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_exec(db, "BEGIN TRANSACTION;", NULL, NULL, NULL);

    for (const auto &entry : hashes)
    {
        ll hash = entry.first;
        double time = entry.second.first;

        sqlite3_bind_int64(stmt, 1, hash);
        sqlite3_bind_int(stmt, 2, song_id);
        sqlite3_bind_double(stmt, 3, time);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
        {
            std::cerr << "Error inserting hash: " << sqlite3_errmsg(db) << "\n";
        }

        sqlite3_reset(stmt);
    }

    sqlite3_exec(db, "END TRANSACTION;", NULL, NULL, NULL);

    sqlite3_finalize(stmt);
}

matches_hash_map DB::get_matching_hashes(song_hash_map &fp)
{
    matches_hash_map matches;

    std::string sql = "SELECT song_id, time FROM fingerprints WHERE hash IN (";
    for (size_t i = 0; i < fp.size(); i++)
    {
        sql += (i == fp.size() - 1) ? "?" : "?, ";
    }
    sql += ");";

    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return matches;
    }

    int index = 1;
    for (auto &entry : fp)
        sqlite3_bind_int64(stmt, index++, entry.first);

    rc = sqlite3_step(stmt);

    while (rc == SQLITE_ROW)
    {
        int song_id = sqlite3_column_int(stmt, 0);
        double time = sqlite3_column_double(stmt, 1);
        ll hash = sqlite3_column_int64(stmt, 0);

        matches[song_id].push_back({time * 1000, fp[hash].first * 1000});

        rc = sqlite3_step(stmt);
    }

    if (rc != SQLITE_DONE)
    {
        std::cerr << "Error querying hashes: " << sqlite3_errmsg(db) << "\n";
        sqlite3_finalize(stmt);
        return matches;
    }

    sqlite3_finalize(stmt);
    return matches;
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

Song DB::get_song(int song_id)
{
    std::string sql = "SELECT title, author FROM songs WHERE id = ?;";
    sqlite3_stmt *stmt = nullptr;
    Song song(-1, "", "");

    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << "\n";
        return song;
    }

    sqlite3_bind_int(stmt, 1, song_id);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        const unsigned char *title = sqlite3_column_text(stmt, 0);
        const unsigned char *author = sqlite3_column_text(stmt, 1);
        song = Song(song_id, std::string(reinterpret_cast<const char *>(title)), std::string(reinterpret_cast<const char *>(author)));
    }
    else
    {
        std::cerr << "No song found with ID: " << song_id << "\n";
    }

    sqlite3_finalize(stmt);
    return song;
}

void DB::init_db(sqlite3 **database)
{
    // std::cout << "Initializing database...\n";
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
    // std::cout << "Database initialized successfully.\n\n";
}

void DB::delete_all()
{
    const char *drop_fingerprints = "DROP TABLE IF EXISTS fingerprints;";
    const char *drop_songs = "DROP TABLE IF EXISTS songs;";
    char *err = 0;

    int rc = sqlite3_exec(db, drop_fingerprints, nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error dropping fingerprints table: " << err << "\n";
        return;
    }

    rc = sqlite3_exec(db, drop_songs, nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error dropping songs table: " << err << "\n";
        return;
    }
}
