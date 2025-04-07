cmake_minimum_required(VERSION 3.30)
project(compiler)

set(CMAKE_CXX_STANDARD 14)

add_executable(compiler
        main.cpp
        lexicalAnalysis.cpp
        lexicalAnalysis.h
        grammaticalAnalysis.h
        SymbolTable.h
        QuadrupleGenerator.h
        Test/LanCherLogger.h
)

set(CMAKE_EXE_LINKER_FLAGS "-static")