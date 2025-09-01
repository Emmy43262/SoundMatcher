#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "src/fingerprint/create_fingerprint.h"

void add_to_db(db_hash_map &db, song_hash_map &fp);
std::unordered_map<int, int> match_fingerprint(db_hash_map &db, song_hash_map &fp);

int main()
{
    db_hash_map db;

    char path1[] = "../audio/crab.wav";
    char path2[] = "../audio/fh4.wav";
    char path3[] = "../audio/sample_crab.wav";

    song_hash_map fp1 = create_fingerprint(path1, 1);
    add_to_db(db, fp1);
    fp1 = {};
    std::cout << "done1\n";

    song_hash_map fp2 = create_fingerprint(path2, 2);
    add_to_db(db, fp2);
    fp2 = {};
    std::cout << "done2\n";

    song_hash_map fp3 = create_fingerprint(path3, -1);
    std::cout << "done3\n";

    std::unordered_map<int, int> match_score = match_fingerprint(db, fp3);
    for (auto it : match_score)
    {
        std::cout << "Song " << it.first << " has score " << it.second << "\n";
    }

    /*std::unordered_map<int, int> match_count;

    std::ofstream song1("song1.txt");
    std::ofstream song2("song2.txt");

    std::unordered_map<int, std::unordered_map<int, int>> offset_counts; // song_id -> offset -> count

    for (auto &sample_entry : fp3)
    {
        ull h = sample_entry.first;

        for (int delta = -1; delta <= 1; delta++)
        { // check nearby hashes too
            ull hash = h + delta;
            if (hash >> 32 != h >> 32) // check if first 4 bytes are the same
                continue;

            if (db.find(hash) != db.end())
            {
                for (auto &sample_occurrence : sample_entry.second)
                {
                    int sample_time = sample_occurrence.first;

                    for (auto &db_occurrence : db[hash])
                    {
                        int db_time = db_occurrence.first;
                        int song_id = db_occurrence.second;
                        int time_offset = db_time - sample_time;

                        offset_counts[song_id][time_offset]++;

                        if (song_id == 1)
                            song1 << db_time << " " << sample_time << "\n";
                        else if (song_id == 2)
                            song2 << db_time << " " << sample_time << "\n";
                    }
                }
            }
        }
    }

    song1.close();
    song2.close();

    for (auto &song_data : offset_counts)
    {
        int song_id = song_data.first;
        int max_count = 0;
        int best_offset = 0;

        for (auto &offset_data : song_data.second)
        {
            if (offset_data.second > max_count)
            {
                max_count = offset_data.second;
                best_offset = offset_data.first;
            }
        }

        match_count[song_id] = max_count;
        std::cout << "Song " << song_id << " has best offset " << best_offset
                  << " with " << max_count << " matches\n";
    }

    /*for (auto sample : fp3)
    {
        ull h = sample.first;

        for (ull hash = h; hash <= h; hash++) // check nearby hashes too
        {
            // check if first 4 bytes are the same
            if (hash >> 32 != h >> 32)
                continue;
            if (db.find(hash) == db.end())
                continue;
            for (auto match : db[hash])
            {
                for (auto it2 : sample.second)
                    if (match.second == 1)
                    {
                        song1 << match.first << " " << it2.first << "\n";
                    }
                    else if (match.second == 2)
                    {
                        song2 << match.first << " " << it2.first << "\n";
                    }
                match_count[match.second] += sample.second.size();
            }
        }
    }

    song1.close();
    song2.close();
    std::cout << "done 4\n";*/

    return 0;
}

std::unordered_map<int, int> match_fingerprint(db_hash_map &db, song_hash_map &fp)
{
    std::unordered_map<int, std::vector<std::pair<int, int>>> mp; // song_id -> vector of (db_time, sample_time)

    for (auto sample : fp)
    {
        ull h = sample.first;

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
        ull hash = it.first;

        ans++;
        db[hash].push_back({it.second.first, it.second.second});
    }

    std::cout << "Added " << ans << " entries to db\n";
}