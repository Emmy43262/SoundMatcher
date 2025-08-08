#include <iostream>
#include <fstream>

void read_wav(char *filepath)
{
    std::ifstream audio_file(filepath);

    char buf[32];
    audio_file.read(buf, 16); // skip intro

    int size_header, sample_rate, byte_rate, data_size; // TODO create a header struct
    short type, channels, block_align, bits_per_sample;
    audio_file.read((char *)&size_header, 4);
    audio_file.read((char *)&type, 2);
    audio_file.read((char *)&channels, 2);
    audio_file.read((char *)&sample_rate, 4);
    audio_file.read((char *)&byte_rate, 4);
    audio_file.read((char *)&block_align, 2);
    audio_file.read((char *)&bits_per_sample, 2);

    audio_file.read(buf, 4); // skip id
    std::cout << buf << '\n';
    audio_file.read((char *)&data_size, 4);

    if (channels > 2)
    {
        std::cout << "Too many channels. Parsing stopped.";
        return;
    }

    int bytes_per_sample = bits_per_sample >> 3;
    int samples_per_iteration = 10; // sliding window size
    int num_iterations = data_size / bytes_per_sample / samples_per_iteration / channels;

    for (int i = 0; i < num_iterations; i++)
    {
        short Max = 0, Min = 0;
        for (int j = 0; j < samples_per_iteration * channels; j++)
        {
            short sample, s2;

            if (audio_file.read((char *)&sample, 2).eof())
            {
                std::cout << "EOF reached\n";
                return;
            }

            if (channels == 2) // discard a channel(for now)
                audio_file.read((char *)&sample, 2);

            if (j == 0)
            {
                Max = sample;
                Min = sample;
            }

            Max = std::max(sample, Max);
            Min = std::min(sample, Min);
        }
        std::cout << "Iteration: " << i << " Min: " << Min << " Max: " << Max << '\n';
    }

    audio_file.close();
}