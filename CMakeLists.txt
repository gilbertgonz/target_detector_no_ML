cmake_minimum_required(VERSION 3.5)
project(track_target)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Source files
file(GLOB CPP_SOURCES *.cpp)

# Add executables
add_executable(main main.cpp ${CPP_SOURCES})

# Specify include dir
#include_directories(src/include)

# Link libraries
target_link_libraries(main ${OpenCV_LIBS})
