#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <assert.h>

short read_sample(std::ifstream *file, int channels)
{
    int sample = 0;
    short channel_sample = 0;

    for (int i = 0; i < channels; i++)
    {
        if (file->read((char *)&channel_sample, 2).eof())
        {
            throw("EOF reachedd\n");
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
    std::string resample_command = ffmpeg_path + " -loglevel error -y -i  " + file_string + " -ar 11025 -ac 1 -f wav -sample_fmt s16 -write_bext 0 " + "temp.wav";

    int resample_error = system(resample_command.c_str());
    if (resample_error != 0)
    {
        throw("Error with resampling " + resample_error);
    }

    std::ifstream audio_file("temp.wav", std::ios::binary);

    audio_file.ignore(16); // skip intro

    int size_header, sample_rate, byte_rate, data_size; // TODO create a header struct
    short type, channels, block_align, bits_per_sample;
    audio_file.read((char *)&size_header, 4);
    audio_file.read((char *)&type, 2);
    audio_file.read((char *)&channels, 2);
    audio_file.read((char *)&sample_rate, 4);
    audio_file.read((char *)&byte_rate, 4);
    audio_file.read((char *)&block_align, 2);
    audio_file.read((char *)&bits_per_sample, 2);

    char buf[8];
    audio_file.read(buf, 4); // get next title
    buf[4] = (char)NULL;
    if (strcmp(buf, "LIST") == 0)
    {
        int list_size = 0;
        audio_file.read((char *)&list_size, 4);
        audio_file.ignore(list_size + 4); // include next id
    }
    audio_file.read((char *)&data_size, 4);

    int bytes_per_channel = bits_per_sample >> 3;
    int total_samples = data_size / bytes_per_channel / channels;

    std::vector<short> samples;

    for (int i = 0; i < total_samples; i++)
    {
        samples.push_back(read_sample(&audio_file, channels));
    }

    audio_file.close();
    return samples;
}