cmake_minimum_required(VERSION 3.28)

project(CSTMTests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_COMPILE_WARNINGS_AS_ERROR ON)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PUBLIC
        Main.cpp
        CompileAll.cpp
        Result.cpp
        Concepts.cpp
        String.cpp
        StringView.cpp)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ../CSTM/)

target_link_libraries(${PROJECT_NAME} PRIVATE CSTM)
