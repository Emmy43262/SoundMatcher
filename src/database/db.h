#pragma once
#include <fingerprint/commons.h>
#include <sqlite3.h>
#include <string>

struct Song
{
    int id;
    std::string title;
    std::string author;
    std::string filepath;
    Song(int i, std::string t, std::string a) : id(i), title(t), author(a) {}
    Song(int i, std::string t, std::string a, std::string f) : id(i), title(t), author(a), filepath(f) {}
};

struct Hash
{
    long long hash;
    int song_id;
    double time;
    Hash(long long h, int s, double t) : hash(h), song_id(s), time(t) {}
};

class DB
{
public:
    DB();
    void add_song(Song &song);
    void add_hashes(int song_id, song_hash_map &hashes);
    matches_hash_map get_matching_hashes(song_hash_map &fp);
    void diplay_songs();
    ~DB();

private:
    void init_db(sqlite3 **db);
    sqlite3 *db;
};
