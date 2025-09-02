cmake_minimum_required(VERSION 4.1)
project(SoundMatcher LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)


add_executable(
    SoundMatcher 
    main.cpp 
    src/preprocessing/wav_reader.cpp 
    src/fingerprint/create_fingerprint.cpp
    src/database/db.cpp
    external/sqlite/sqlite3.c
)

target_include_directories(SoundMatcher PRIVATE src external/sqlite)