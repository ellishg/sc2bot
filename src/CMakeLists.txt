include_directories("../include")
include_directories("common")
include_directories("../s2client-api/include")
include_directories(SYSTEM "${PROJECT_BINARY_DIR}/s2client-api/generated")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/s2client-api/contrib/SDL-mirror/include")

file(GLOB sc2bot_SRC "*.cpp" "Bots/*.cpp" "Strategies/Zerg/*.cpp")
file(GLOB common_SRC "common/*.cpp")

# Setup MSVC parallelized builds
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

add_executable(sc2bot ${sc2bot_SRC} ${common_SRC})
target_link_libraries(sc2bot sc2api sc2lib sc2utils)
