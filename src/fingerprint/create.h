#pragma once
#include <fingerprint/commons.h>
#include <database/db.h>
#include <unordered_map>
#include <string>
#include <vector>

struct Peak
{
    double time;
    int frequency;
    Peak(double t, int f) : time(t), frequency(f) {}
};

db_hash_map create_fingerprint(std::vector<short>, int song_id);
void process_song(Song &song, DB &db);
ll get_hash(short fr1, short fr2, short delta);