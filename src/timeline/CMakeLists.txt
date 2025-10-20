# OMNISIGHT Timeline Threading™ Module
# CMake build configuration

cmake_minimum_required(VERSION 3.16)

# Timeline module library
add_library(omnisight_timeline STATIC
    timeline.c
    trajectory_predictor.c
    event_predictor.c
)

target_include_directories(omnisight_timeline PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/perception
)

target_link_libraries(omnisight_timeline
    m  # Math library
)

# Compiler flags for optimization
target_compile_options(omnisight_timeline PRIVATE
    -Wall
    -Wextra
    -O3
    -ffast-math
    -march=armv8-a  # ARTPEC-8 architecture
)

# Install headers
install(FILES
    timeline.h
    trajectory_predictor.h
    event_predictor.h
    DESTINATION include/omnisight/timeline
)

# Install library
install(TARGETS omnisight_timeline
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
