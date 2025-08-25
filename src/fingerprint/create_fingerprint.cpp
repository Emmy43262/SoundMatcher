#include <preprocessing/wav_reader.h>
#include <vector>
#include <iostream>

void create_fingerprint(char *filepath)
{
    std::vector<short> samples = read_wav(filepath);

    return;
}