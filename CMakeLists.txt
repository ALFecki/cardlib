cmake_minimum_required(VERSION 3.17)
project(cardlib)

set(PCSC_DIR "/usr/lib/x86_64-linux-gnu")

set(CMAKE_CXX_STANDARD 20)

set(LIBBEE2 ${PROJECT_SOURCE_DIR}/libs/libbee2_static.a)

set(SRC
        src/apducmd.cpp
        src/bpace.cpp
        src/logger.cpp
        src/pcsc.cpp)

include_directories(include libs)

add_library(cardlib ${SRC})
add_executable(cardlib-test main.cpp ${SRC})

target_link_libraries(cardlib PUBLIC ${LIBBEE2} libpcsclite)
target_link_libraries(cardlib-test PUBLIC ${LIBBEE2} ${cardlib} libpcsclite)