#pragma once
#include <unordered_map>
#include <string>
#include <vector>
typedef unsigned long long ull;
std::unordered_map<ull, std::vector<std::pair<int, std::string>>> create_fingerprint(char *filepath);