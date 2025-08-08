cmake_minimum_required(VERSION 4.1)

project(SoundMatcher)

include_directories(src)

add_executable(main main.cpp src/preprocessing/wav_reader.cpp)