#pragma once
#include <sqlite3.h>
#include <string>

struct Song
{
    int id;
    std::string title;
    std::string author;
    Song(int i, std::string t, std::string a) : id(i), title(t), author(a) {}
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
    void diplay_songs();
    ~DB();

private:
    void init_db(sqlite3 **db);
    sqlite3 *db;
};
