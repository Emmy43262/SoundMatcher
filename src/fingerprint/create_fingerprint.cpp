#include <preprocessing/wav_reader.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <complex.h>
#include <unordered_map>
#include <string>

typedef unsigned long long ull;
typedef std::unordered_map<ull, std::vector<std::pair<int, std::string>>> hash_map;

const int window_size = 1024;
const int overlap = 32;
using cd = std::complex<double>;
const double PI = acos(-1);
const int num_bands = 6;
const int bands[] = {10, 20, 40, 80, 160, 512};

void fft(std::vector<cd> &a);

ull get_hash(short fr1, short fr2, short delta);

hash_map create_fingerprint(char *filepath)
{

    std::vector<short> samples = read_wav(filepath);

    int num_windows = 1 + (samples.size() - window_size) / (window_size - overlap); // ensure does not overflow

    std::vector<std::vector<double>> spectrogram;

    for (int i = 0; i < num_windows; i++)
    {
        std::vector<cd> current_window;
        for (int j = i * (window_size - overlap); j < i * (window_size - overlap) + window_size; j++)
        {
            current_window.push_back(cd((double)samples[j], 0));
        }
        fft(current_window);
        std::vector<double> magnitudes;
        for (int i = 0; i <= window_size / 2; i++)
            magnitudes.push_back(std::abs(current_window[i]));

        spectrogram.push_back(magnitudes);
    }

    std::ofstream out("out.txt");

    std::vector<std::vector<short>> peaks;
    for (auto it : spectrogram)
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
                if (it[i] > max)
                {
                    max = it[i];
                    pos_max = i;
                }
                it[i] = 0;
            }
            maxes.push_back({pos_max, max});
        }
        double avg_maxes = 0;
        for (auto mx : maxes)
        {
            avg_maxes += mx.second;
        }
        avg_maxes /= (double)num_bands;

        std::vector<short> peaks_slice;

        for (auto mx : maxes)
        {

            if (mx.second >= avg_maxes)
            {
                peaks_slice.push_back((short)mx.first);
                it[mx.first] = mx.second;
            }
            else
            {
                peaks_slice.push_back(0);
            }
        }

        peaks.push_back(std::vector<short>(peaks_slice));

        for (int i = 0; i < it.size(); i++)
        {
            out << it[i];
            if (i + 1 != it.size())
                out << ", ";
        }
        out << '\n';
    }
    out.close();

    const int w = 4, v_off = 2;
    std::unordered_map<ull, std::vector<std::pair<int, std::string>>> hashes;

    for (int col = 0; col < peaks.size(); col++)
    {
        for (int row = 0; row < peaks[col].size(); row++)
        {
            if (peaks[col][row] == 0)
                continue;

            for (int i = std::max(0, row - v_off); i < std::min((int)peaks[col].size(), row + v_off); i++)
            {
                for (int j = col + 1; j <= col + w && j <= peaks.size(); j++)
                {
                    if (peaks[i][j] == 0)
                        continue;

                    hashes[get_hash(peaks[col][row], peaks[i][j], i - col)].push_back({col, std::string(filepath)}); // todo Song ids
                }
            }
        }
    }

    return hashes;
}

ull get_hash(short fr1, short fr2, short delta)
{
    return fr1 << 32 + fr2 << 16 + delta;
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