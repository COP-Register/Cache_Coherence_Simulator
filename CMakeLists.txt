cmake_minimum_required(VERSION 3.28)
project(Cache_Coherence_Simulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(Cache_Coherence_Simulator main.cpp
        CPU.cpp
        CPU.h
        Processor.cpp
        Processor.h
        Cache_Controller.cpp
        Cache_Controller.h
        Cache.cpp
        Cache.h
        Cache_Line.cpp
        Cache_Line.h
        Word.cpp
        Word.h
        Selection.cpp
        Selection.h)
