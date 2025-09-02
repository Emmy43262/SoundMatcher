#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "src/fingerprint/create_fingerprint.h"
#include <sqlite3.h>
#include "src/database/db.h"

void add_to_db(db_hash_map &db, song_hash_map &fp);
std::unordered_map<int, int> match_fingerprint(db_hash_map &db, song_hash_map &fp);

int main()
{

    DB *mydb = new DB();

    Song song1(-1, "Crab Rave", "Noisestorm");
    mydb->add_song(song1);

    mydb->diplay_songs();

    return 0;

    db_hash_map db;

    char path1[] = "../audio/crab.wav";
    char path2[] = "../audio/fh4.wav";
    char path3[] = "../audio/sample_crab.wav";

    song_hash_map fp1 = create_fingerprint(path1, 1);
    add_to_db(db, fp1);
    fp1 = {};

    song_hash_map fp2 = create_fingerprint(path2, 2);
    add_to_db(db, fp2);
    fp2 = {};

    song_hash_map fp3 = create_fingerprint(path3, -1);

    std::unordered_map<int, int> match_score = match_fingerprint(db, fp3);
    for (auto it : match_score)
    {
        std::cout << "Song " << it.first << " has score " << it.second << "\n";
    }

    return 0;
}

std::unordered_map<int, int> match_fingerprint(db_hash_map &db, song_hash_map &fp)
{
    std::unordered_map<int, std::vector<std::pair<int, int>>> mp; // song_id -> vector of (db_time, sample_time)

    for (auto sample : fp)
    {
        ll h = sample.first;

        for (auto match : db[h])
        {
            mp[match.second].push_back({match.first * 1000, sample.second.first * 1000});
        }
    }

    std::unordered_map<int, int> scores;

    for (auto song_matches : mp)
    {
        int song_id = song_matches.first;
        auto &matches = song_matches.second;

        int song_score = 0;

        for (int i = 0; i < matches.size(); i++)
            for (int j = i + 1; j < matches.size(); j++)
            {
                int db_delta = abs(matches[j].first - matches[i].first);
                int sample_delta = abs(matches[j].second - matches[i].second);
                if (abs(db_delta - sample_delta) <= 100)
                    song_score++;
            }
        scores[song_id] = song_score;
    }
    return scores;
}

void add_to_db(db_hash_map &db, song_hash_map &fp)
{
    int ans = 0;
    for (auto it : fp)
    {
        ll hash = it.first;

        ans++;
        db[hash].push_back({it.second.first, it.second.second});
    }

    std::cout << "Added " << ans << " entries to db\n";
}