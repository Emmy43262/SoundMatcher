#include <fstream>
#include <iostream>
#include "src/fingerprint/create_fingerprint.h"

int main()
{
    char path[] = "../audio/crab.wav";
    create_fingerprint(path);

    return 0;
}
