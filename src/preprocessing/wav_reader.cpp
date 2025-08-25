#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

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

std::vector<short> read_wav(char *filepath)
{

    // resample audio to 11025hz
    std::string file_string = std::string(filepath);
    std::string ffmpeg_path = "..\\bin\\ffmpeg.exe"; // not cross platform yet
    std::string resample_command = ffmpeg_path + " -loglevel error -y -i " + file_string + " -ar 11025 " + "temp.wav";

    int resample_error = system(resample_command.c_str());
    if (resample_error != 0)
    {
        throw("Error with resampling " + resample_error);
    }

    std::ifstream audio_file("temp.wav", std::ios::binary);

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

    int bytes_per_sample = bits_per_sample >> 3;
    int total_samples = data_size / bytes_per_sample / channels;

    std::vector<short> samples;

    for (int i = 0; i < total_samples; i++)
    {
        samples.push_back(read_sample(&audio_file, channels));
    }

    audio_file.close();
    return samples;
}