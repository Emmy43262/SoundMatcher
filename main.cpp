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

    Song match_song = mydb->get_song(match.first);
    std::cout << "Best match: " << match_song.title << " by " << match_song.author << " with score " << match.second << ".\n\n";

    return 0;
}