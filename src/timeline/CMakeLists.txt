# OMNISIGHT Timeline Threading™ Module
# CMake build configuration - Stub implementation

cmake_minimum_required(VERSION 3.16)

# Timeline module library - using stub implementation
add_library(omnisight_timeline STATIC
    timeline_stub.c
)

target_include_directories(omnisight_timeline PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/perception
)

target_link_libraries(omnisight_timeline
    m  # Math library
)

# Compiler flags (architecture-agnostic for stub build)
target_compile_options(omnisight_timeline PRIVATE
    -Wall
    -Wextra
    -O2
)

# Compiler definitions
target_compile_definitions(omnisight_timeline PRIVATE
    _GNU_SOURCE
    OMNISIGHT_STUB_BUILD=1
)

# Install headers
install(FILES
    timeline.h
    DESTINATION include/omnisight/timeline
)

# Install library
install(TARGETS omnisight_timeline
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
