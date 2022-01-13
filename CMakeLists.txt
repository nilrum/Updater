cmake_minimum_required(VERSION 3.20)
project(Updater)

set(CMAKE_CXX_STANDARD 17)

include(C:/CMakeProjects/Fetch.cmake)
set(NAME_UPDATER VersionControl)

add_executable(${NAME_UPDATER} main.cpp)
target_include_directories(${NAME_UPDATER} PUBLIC ${FETCH_DIR} )
target_link_libraries(${NAME_UPDATER} PUBLIC Property cpr::cpr)