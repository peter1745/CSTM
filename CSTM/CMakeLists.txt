cmake_minimum_required(VERSION 3.28)

project(CSTM)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COMPILE_WARNINGS_AS_ERROR ON)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PUBLIC
        File.cpp
        Utf8String.cpp
        String.cpp
        StringView.cpp
        CodePointIterator.cpp)
