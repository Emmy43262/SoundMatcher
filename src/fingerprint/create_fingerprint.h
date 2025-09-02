#pragma once
#include <unordered_map>
#include <string>
#include <vector>
typedef long long ll;
typedef std::unordered_map<ll, std::vector<std::pair<double, int>>> db_hash_map;
typedef std::unordered_map<ll, std::pair<double, int>> song_hash_map;

struct Peak
{
    double time;
    int frequency;
    Peak(double t, int f) : time(t), frequency(f) {}
};

song_hash_map create_fingerprint(char *filepath, int song_id);
ll get_hash(short fr1, short fr2, short delta);