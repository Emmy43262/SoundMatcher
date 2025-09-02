#include "src/database/db.h"
#include "src/fingerprint/create.h"
#include "src/fingerprint/match.h"

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sqlite3.h>

// -a "song_path.wav" Add song
// -m "audio_path.wav" Match audio
// -l Display known songs
// -clear Delete the database

int main(int argc, char *argv[])
{
    if (argc == 1 || argc > 3)
    {
        std::cerr << "Usage:\n"
                  << " -a \"song_path.wav\" to add a song\n"
                  << " -m \"audio_path.wav\" to match audio\n"
                  << " -l to list known songs\n"
                  << " -clear clear the database\n";
        return 1;
    }

    if (std::string(argv[1]) == "-a")
    {
        std::ifstream file_check(argv[2]);
        if (!file_check.good())
        {
            std::cerr << "File " << argv[2] << " does not exist or is not accessible.\n";
            return 1;
        }
        file_check.close();

        DB *mydb = new DB();

        std::string title, author;
        std::cout << "Enter song title: ";
        std::getline(std::cin, title);
        std::cout << "Enter song author: ";
        std::getline(std::cin, author);

        Song new_song(0, title, author, std::string(argv[2]));
        process_song(new_song, *mydb);

        delete mydb;
        return 0;
    }
    else if (std::string(argv[1]) == "-m")
    {
        std::ifstream file_check(argv[2]);
        if (!file_check.good())
        {
            std::cerr << "File " << argv[2] << " does not exist or is not accessible.\n";
            return 1;
        }
        file_check.close();

        DB *mydb = new DB();
        std::pair<int, int> match = match_audio(std::string(argv[2]), mydb);

        if (match.first == -1)
        {
            std::cout << "No match found.\n";
            delete mydb;
            return 0;
        }

        Song match_song = mydb->get_song(match.first);
        std::cout << "Best match: " << match_song.title << " by " << match_song.author << " with score " << match.second << ".\n";

        delete mydb;
        return 0;
    }
    else if (std::string(argv[1]) == "-l")
    {
        DB *mydb = new DB();
        mydb->diplay_songs();
        delete mydb;
        return 0;
    }
    else if (std::string(argv[1]) == "-clear")
    {
        std::cout << "Clearing database...\n";
        DB *mydb = new DB();
        mydb->delete_all();
        return 0;
    }
    else
    {
        std::cerr << "Unknown option: " << argv[1] << "\n";
        return 1;
    }

    return 0;
}