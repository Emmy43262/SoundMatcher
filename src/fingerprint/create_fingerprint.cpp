#include <preprocessing/wav_reader.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <complex.h>
#include <unordered_map>
#include <string>
#include "create_fingerprint.h"
#include <cassert>

typedef unsigned long long ull;
typedef std::complex<double> cd;

const int window_size = 1024;
const int overlap = 512;
const double PI = acos(-1);
const int num_bands = 6;
const int bands[] = {10, 20, 40, 80, 160, 500};

void fft(std::vector<cd> &a);

ull get_hash(short fr1, short fr2, short delta);

song_hash_map create_fingerprint(char *filepath, int song_id)
{

    std::vector<short> samples = read_wav(filepath);
    double song_length = (double)samples.size() / 11025.0;

    int num_windows = samples.size() / (window_size - overlap); // ensure does not overflow

    std::vector<std::vector<double>> spectrogram;

    std::vector<double> hamming;
    for (int i = 0; i < window_size; i++)
        hamming.push_back(0.54 - 0.46 * cos(2.0 * PI * i / (window_size - 1)));

    for (int i = 0; i < num_windows; i++)
    {
        std::vector<cd> current_window;
        int start = i * (window_size - overlap);

        for (int j = start; j < start + window_size; j++)
            current_window.push_back(cd((double)samples[j] * hamming[j - start], 0));

        fft(current_window);
        std::vector<double> magnitudes;
        for (int i = 0; i <= window_size / 2; i++)
            magnitudes.push_back(std::abs(current_window[i]));

        spectrogram.push_back(magnitudes);
    }

    std::ofstream out("out.txt");

    // std::vector<std::vector<short>> peaks;
    std::vector<Peak> peaks;

    for (int slice = 0; slice < spectrogram.size(); slice++)
    {
        std::vector<std::pair<int, double>> maxes;
        for (int current_band = 0; current_band < num_bands; current_band++)
        {
            int band_start = (current_band == 0) ? 1 : (bands[current_band - 1] + 1);
            int band_end = bands[current_band];

            double max = -1e4;
            int pos_max = 0;

            for (int i = band_start; i <= band_end; i++)
            {
                if (spectrogram[slice][i] > max)
                {
                    max = spectrogram[slice][i];
                    pos_max = i;
                }
                spectrogram[slice][i] = 0;
            }
            maxes.push_back({pos_max, max});
        }

        double avg_maxes = 0;
        for (auto mx : maxes)
            avg_maxes += mx.second;
        avg_maxes /= (double)num_bands;

        for (auto mx : maxes)
        {
            if (mx.second >= avg_maxes)
            {
                peaks.push_back(Peak((double)slice / spectrogram.size() * song_length, (short)mx.first));

                spectrogram[slice][mx.first] = mx.second;
            }
        }

        // peaks.push_back(std::vector<short>(peaks_slice)); // peaks[i] = a peak slice

        // for (int i = 0; i < it.size(); i++)
        //{
        // out << it[i];
        // if (i + 1 != it.size())
        //     out << ", ";
        //}
        // out << '\n';
    }
    out.close();

    const int max_delta = 10;
    song_hash_map hashes;

    for (int i = 0; i < peaks.size(); i++)
        for (int j = 1; j <= max_delta && i + j < peaks.size(); j++)
        {
            short delta_ms = (peaks[i + j].time - peaks[i].time) * 1000.0;
            hashes[get_hash(peaks[i].frequency, peaks[i + j].frequency, delta_ms)] = {peaks[i].time, song_id};
        }

    /*for (int col = 0; col < peaks.size(); col++)
    {
        for (int row = 0; row < num_bands; row++)
        {
            if (peaks[col][row] == 0)
                continue;

            for (int i = std::max(0, row - v_off); i < std::min(num_bands, row + v_off); i++)
            {
                for (int j = col + 1; j <= col + w && j < peaks.size(); j++)
                {
                    if (peaks[j][i] == 0)
                        continue;

                    hashes[get_hash(peaks[col][row], peaks[j][i], j - col)].push_back({col, song_id});
                }
            }
        }
    }*/

    return hashes;
}

ull get_hash(short fr1, short fr2, short delta)
{
    return ((ull)fr1 << 32) + ((ull)fr2 << 16ll) + (ull)delta;
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