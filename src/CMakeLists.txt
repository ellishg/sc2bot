include_directories("../include")
include_directories("../s2client-api/include")
include_directories("../s2client-api/common")
include_directories(SYSTEM "${PROJECT_BINARY_DIR}/s2client-api/generated")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/s2client-api/contrib/SDL-mirror/include")

add_executable(sc2bot main.cpp)
target_link_libraries(sc2bot sc2api sc2lib sc2utils)
