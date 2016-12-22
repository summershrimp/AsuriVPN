cmake_minimum_required(VERSION 3.6)
project(AsuriVPN)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
set(SOURCE_FILES src/main.c include/utils.h src/utils.c src/devices.c include/devices.h src/analyze.c include/analyze.h)
add_executable(AsuriVPN ${SOURCE_FILES})