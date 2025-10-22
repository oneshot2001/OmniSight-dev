# OMNISIGHT Swarm Intelligence Module
# CMake build configuration - Stub implementation

cmake_minimum_required(VERSION 3.16)

# Swarm module library - using stub implementation
add_library(omnisight_swarm STATIC
    swarm_stub.c
)

target_include_directories(omnisight_swarm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/perception
    ${CMAKE_SOURCE_DIR}/src/timeline
)

target_link_libraries(omnisight_swarm
    m            # Math library
    pthread      # POSIX threads
)

# Compiler flags (architecture-agnostic for stub build)
target_compile_options(omnisight_swarm PRIVATE
    -Wall
    -Wextra
    -O2
)

# Compiler definitions
target_compile_definitions(omnisight_swarm PRIVATE
    _GNU_SOURCE
    OMNISIGHT_STUB_BUILD=1
)

# Install headers
install(FILES
    swarm.h
    DESTINATION include/omnisight/swarm
)

# Install library
install(TARGETS omnisight_swarm
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
