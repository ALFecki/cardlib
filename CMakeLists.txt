cmake_minimum_required(VERSION 3.17)
project(cardlib)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/bee2)

set(SRC
        src/apducmd.cpp
        src/bpace.cpp
        src/logger.cpp
        src/pcsc.cpp
        src/certHat.cpp
        src/cardlib.cpp
        src/cardsecure.cpp)


include_directories(include libs libs/bee2/include)

add_library(cardlib ${SRC})
add_executable(cardlib-test main.cpp ${SRC})

target_link_libraries(cardlib PUBLIC bee2 pcsclite)
target_link_libraries(cardlib-test PUBLIC bee2 ${cardlib} pcsclite)