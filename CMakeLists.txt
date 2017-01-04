cmake_minimum_required(VERSION 3.6)
project(AsuriVPN)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)
set(SOURCE_FILES src/main.c include/utils.h src/utils.c src/device.c include/device.h src/analyze.c include/analyze.h src/server.c include/server.h src/client.c include/client.h src/event.c include/event.h src/config.c include/config.h include/common.h src/ipcfg.cpp include/ipcfg.h include/protocal.h)
add_executable(AsuriVPN ${SOURCE_FILES})