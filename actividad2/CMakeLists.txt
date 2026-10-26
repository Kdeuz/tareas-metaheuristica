cmake_minimum_required(VERSION 3.20)
project(Metaheuristica)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable
add_executable(app src/main.cpp)

# Include directories
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
