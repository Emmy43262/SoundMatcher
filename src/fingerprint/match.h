#pragma once
#include <database/db.h>

std::pair<int, int> match_audio(std::string sample_path, DB *db);

std::pair<int, int> match_audio(std::vector<short> &audio_data, DB *db);