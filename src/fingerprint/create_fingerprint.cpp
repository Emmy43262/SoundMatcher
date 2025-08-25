#include <preprocessing/wav_reader.h>
#include <vector>
#include <iostream>

const int window_size = 1024;
const int overlap = 32;

void process_window(std::vector<short> window);

void create_fingerprint(char *filepath)
{
    std::vector<short> samples = read_wav(filepath);

    int num_windows = 1 + (samples.size() - window_size) / (window_size - overlap); // ensure does not overflow

    std::cout << samples.size() << '\n';
    std::cout << num_windows << '\n';

    for (int i = 0; i < num_windows; i++)
    {
        std::vector<short> current_window;
        for (int j = i * (window_size - overlap); j < (i + 1) * (window_size - overlap); j++)
        {
            current_window.push_back(samples[j]);
        }

        process_window(current_window);
    }

    return;
}

void process_window(std::vector<short> window)
{

    return;
}