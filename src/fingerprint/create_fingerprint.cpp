#include <preprocessing/wav_reader.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <complex.h>

const int window_size = 1024;
const int overlap = 32;
using cd = std::complex<double>;
const double PI = acos(-1);

void fft(std::vector<cd> &a);
void process_window(std::vector<short> window);

void create_fingerprint(char *filepath)
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
        for (auto &fr : current_window)
            magnitudes.push_back(std::abs(fr));

        spectrogram.push_back(magnitudes);
    }

    std::ofstream out("out.txt");
    for (auto it : spectrogram)
    {
        for (int i = 0; i < it.size(); i++)
        {
            out << it[i];
            if (i + 1 != it.size())
                out << ", ";
        }
        out << '\n';
    }

    return;
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