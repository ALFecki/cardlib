cmake_minimum_required(VERSION 3.17)
project(cardlib)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(libs/bee2)

set(ID_CARD ${PROJECT_SOURCE_DIR}/libs/libid_card.a)
set(JSONCPP ${PROJECT_SOURCE_DIR}/libs/jsoncpp/lib/libjsoncpp.a)

set(SRC
        src/apducmd.cpp
        src/bpace.cpp
        src/logger.cpp
        src/pcsc.cpp
        src/certHat.cpp
        src/cardsecure.cpp
        src/utils.cpp)


include_directories(include libs libs/bee2/include libs/jsoncpp/include)
add_library(cardlib ${SRC})
add_executable(cardlib-test main.cpp ${SRC})

target_link_libraries(cardlib PUBLIC bee2 pcsclite ${ID_CARD} ${JSONCPP})
target_link_libraries(cardlib-test PUBLIC bee2 ${cardlib} pcsclite ${ID_CARD} ${JSONCPP})