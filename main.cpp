#include <fstream>
#include <iostream>
#include "src/preprocessing/wav_reader.h"

int main()
{
    char path[] = "../audio/crab.wav";
    read_wav(path);

    return 0;
}
