#include "create.h"
#include <preprocessing/wav_reader.h>
#include <database/db.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <complex.h>
#include <unordered_map>
#include <string>
#include <cassert>
#include <chrono>

typedef std::complex<double> cd;

const int window_size = 1024;
const int overlap = 512;
const int num_bands = 6;
const int bands[] = {10, 20, 40, 80, 160, 500};
const double PI = acos(-1);

void fft(std::vector<cd> &a);

ll get_hash(short fr1, short fr2, short delta);

std::pair<int, double> get_band_max(std::vector<double> &magnitudes, int band)
{
    int band_start = (band == 0) ? 1 : (bands[band - 1] + 1);
    int band_end = bands[band];

    double max = -1e4;
    int pos_max = 0;

    for (int i = band_start; i <= band_end; i++)
    {
        if (magnitudes[i] > max)
        {
            max = magnitudes[i];
            pos_max = i;
        }
    }

    return {pos_max, max};
}

double get_max_average(std::vector<std::pair<int, double>> &maxes)
{
    double sum = 0;

    for (int i = 0; i < maxes.size(); i++)
        sum += maxes[i].second;

    return sum / (double)maxes.size();
}

song_hash_map create_fingerprint(std::vector<short> samples, int song_id)
{
    double song_length = (double)samples.size() / 11025.0;

    int num_windows = samples.size() / (window_size - overlap) - 1;

    std::vector<double> hamming;
    for (int i = 0; i < window_size; i++)
        hamming.push_back(0.54 - 0.46 * cos(2.0 * PI * i / (window_size - 1)));

    std::vector<std::vector<double>> spectrogram;
    for (int i = 0; i < num_windows; i++)
    {
        std::vector<cd> current_window;
        int start = i * (window_size - overlap);

        for (int j = start; j < start + window_size; j++)
            current_window.push_back(cd(1.0 * samples[j] * hamming[j - start], 0));

        fft(current_window);

        std::vector<double> magnitudes;
        for (int i = 0; i <= window_size / 2; i++)
            magnitudes.push_back(std::abs(current_window[i]));
        spectrogram.push_back(magnitudes);
    }
    hamming = {};
    samples = {};

    std::vector<Peak> peaks;
    double slice_duration = song_length / (double)spectrogram.size();
    for (int slice = 0; slice < spectrogram.size(); slice++)
    {
        std::vector<std::pair<int, double>> maxes;
        for (int current_band = 0; current_band < num_bands; current_band++)
            maxes.push_back(get_band_max(spectrogram[slice], current_band));

        double avg_maxes = get_max_average(maxes);

        for (auto mx : maxes)
        {
            if (mx.second < avg_maxes)
                continue;
            peaks.push_back(Peak(1.0 * slice * slice_duration, (short)mx.first));
        }
    }
    spectrogram = {};

    const int max_delta = 6;
    song_hash_map hashes;

    for (int root = 0; root < peaks.size(); root++)
        for (int j = 1; j <= max_delta && root + j < peaks.size(); j++)
        {
            int target = root + j;
            short delta_ms = (peaks[target].time - peaks[root].time) * 1000;
            hashes[get_hash(peaks[root].frequency, peaks[target].frequency, delta_ms)] = {peaks[root].time, song_id};
        }

    return hashes;
}

ll get_hash(short fr1, short fr2, short delta)
{
    return ((ll)fr1 << 32) + ((ll)fr2 << 16ll) + (ll)delta;
}

void fft(std::vector<cd> &a)
{
    int n = a.size();
    if (n == 1)
        return;

    std::vector<cd> a0(n / 2), a1(n / 2);
    for (int i = 0; 2 * i < n; i++)
    {
        a0[i] = a[2 * i];
        a1[i] = a[2 * i + 1];
    }
    fft(a0);
    fft(a1);

    double ang = 2 * PI / n;
    cd w(1), wn(cos(ang), sin(ang));
    for (int i = 0; 2 * i < n; i++)
    {
        a[i] = a0[i] + w * a1[i];
        a[i + n / 2] = a0[i] - w * a1[i];
        w *= wn;
    }
}

void process_song(Song &song, DB &db)
{
    if (song.filepath == "")
    {
        std::cerr << "Filepath for song " << song.title << " is empty!\n";
        return;
    }

    std::cout << "Processing song: " << song.title << " by " << song.author << "\n";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    db.add_song(song);
    std::vector<short> song_samples = read_wav((char *)song.filepath.c_str());
    song_hash_map fingerprint = create_fingerprint(song_samples, song.id);
    db.add_hashes(song.id, fingerprint);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Added " << fingerprint.size() << " hashes for song ID: " << song.id << "\n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds\n\n";
}