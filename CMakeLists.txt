cmake_minimum_required(VERSION 3.1)
project(sc2bot)

# Build with c++14 support.
set(CMAKE_CXX_STANDARD 14)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_subdirectory(s2client-api)
add_subdirectory(src)
