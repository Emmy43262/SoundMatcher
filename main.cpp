#include "src/database/db.h"
#include "src/fingerprint/create.h"
#include "src/fingerprint/match.h"

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sqlite3.h>

// void add_to_db(db_hash_map &db, song_hash_map &fp);
// std::unordered_map<int, int> match_fingerprint(db_hash_map &db, song_hash_map &fp);

int main()
{

    DB *mydb = new DB();

    Song song1(0, "Crab Rave", "Noisestorm", "../audio/crab.wav");
    Song song2(0, "Fh4 theme song", "Don't know", "../audio/fh4.wav");
    process_song(song1, *mydb);
    process_song(song2, *mydb);

    std::pair<int, int> match = match_audio(std::string("../audio/sample_crab.wav"), mydb);
    std::cout << "Best match: Song ID " << match.first << " with score " << match.second << "\n";

    return 0;

    /*db_hash_map db;

    char path2[] = "../audio/fh4.wav";
    char path3[] = "../audio/sample_crab.wav";

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
    }*/

    return 0;
}