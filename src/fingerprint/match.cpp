#include <fingerprint/commons.h>
#include <fingerprint/create.h>
#include <preprocessing/wav_reader.h>
#include <database/db.h>

#include <iostream>
#include <string>
#include <chrono>

std::pair<int, int> match_audio(std::vector<short> &audio_data, DB *db);
std::unordered_map<int, int> compute_scores(matches_hash_map &matches);

std::pair<int, int> match_audio(std::string sample_path, DB *db) // song_id, matches
{

    if (sample_path == "")
    {
        std::cerr << "Filepath can't be empty!\n";
        return {-1, -1};
    }

    std::vector<short> song_samples = read_wav((char *)sample_path.c_str());
    return match_audio(song_samples, db);
}

std::pair<int, int> match_audio(std::vector<short> &audio_data, DB *db)
{
    std::cout << "Processing sample...\n";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    db_hash_map fingerprint = create_fingerprint(audio_data, -1);

    song_hash_map simplified_fp;
    for (auto it : fingerprint)
        simplified_fp[it.first] = it.second[0];

    matches_hash_map db_matches = db->get_matching_hashes(simplified_fp);

    std::unordered_map<int, int> scores = compute_scores(db_matches);

    int best_song = -1;
    int best_score = 0;
    for (auto it : scores)
    {
        if (it.second > best_score)
        {
            best_score = it.second;
            best_song = it.first;
        }
    }

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds\n\n";
    return {best_song, best_score};
}

std::unordered_map<int, int> compute_scores(matches_hash_map &matches)
{
    std::unordered_map<int, int> scores;

    for (auto song_matches : matches)
    {
        int song_id = song_matches.first;
        auto &match_times = song_matches.second;

        // std::cout << "For song ID " << song_id << ", found " << match_times.size() << " matching hashes.\n";

        int song_score = 0;

        /*for (int i = 0; i < match_times.size(); i++)
            for (int j = i + 1; j < match_times.size(); j++)
            {
                int db_delta = abs(match_times[j].first - match_times[i].first);
                int sample_delta = abs(match_times[j].second - match_times[i].second);
                if (abs(db_delta - sample_delta) <= 100)
                    song_score++;
            }*/

        std::unordered_map<int, int> histogram;
        histogram = {};

        for (auto &mt : match_times)
        {
            int delta = (mt.first - mt.second) / 200; // spectrogram bucket size
            histogram[delta]++;
            if (histogram[delta] > song_score)
            {
                song_score = histogram[delta];
            }
        }

        // for (auto it : histogram)
        //     std::cout << "Delta: " << it.first << ", Count: " << it.second << "\n";
        scores[song_id] = song_score;
    }
    for (auto it : scores)
        std::cout << "Song ID " << it.first << " has score " << it.second << "\n";
    return scores;
}