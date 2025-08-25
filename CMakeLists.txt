cmake_minimum_required(VERSION 4.1)
project(SoundMatcher LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)


include_directories(src)
add_executable(SoundMatcher main.cpp src/preprocessing/wav_reader.cpp src/fingerprint/create_fingerprint.cpp)


