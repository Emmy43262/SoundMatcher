#include <iostream>
#include <fstream>

short read_sample(std::ifstream *file, int channels)
{
    int sample = 0;
    short channel_sample = 0;

    for (int i = 0; i < channels; i++)
    {
        if (file->read((char *)&channel_sample, 2).eof())
        {
            std::cout << "EOF reachedd\n";
            return 0;
        }
        sample += channel_sample;
    }

    return (short)(sample / channels);
}

void read_wav(char *filepath)
{
    std::ifstream audio_file(filepath, std::ios::binary);

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
    audio_file.read((char *)&data_size, 4);

    std::cout << data_size << '\n';

    int bytes_per_sample = bits_per_sample >> 3;
    int samples_per_iteration = 100; // sliding window size
    int num_iterations = data_size / bytes_per_sample / samples_per_iteration / channels;

    for (int i = 0; i < num_iterations; i++)
    {
        for (int j = 0; j < samples_per_iteration; j++)
        {
            short sample = read_sample(&audio_file, channels);
        }
    }

    audio_file.close();
}