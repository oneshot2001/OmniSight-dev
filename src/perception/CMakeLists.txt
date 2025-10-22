# OMNISIGHT Perception Engine
# CMakeLists.txt - Stub build configuration (no hardware dependencies)

cmake_minimum_required(VERSION 3.16)
project(omnisight_perception VERSION 0.1.0 LANGUAGES C)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Source files - using stub implementation
set(PERCEPTION_SOURCES
    perception_stub.c
)

# Header files
set(PERCEPTION_HEADERS
    perception.h
)

# Build static library (changed from shared for easier linking)
add_library(omnisight_perception STATIC ${PERCEPTION_SOURCES})

# Include directories
target_include_directories(omnisight_perception
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries - only standard libraries needed for stub
target_link_libraries(omnisight_perception
    PUBLIC
        m  # Math library
        pthread
)

# Compiler options (architecture-agnostic for stub build)
target_compile_options(omnisight_perception PRIVATE
    -Wall
    -Wextra
    -O2
)

# Compiler definitions
target_compile_definitions(omnisight_perception PRIVATE
    _GNU_SOURCE
    OMNISIGHT_VERSION="${PROJECT_VERSION}"
    OMNISIGHT_STUB_BUILD=1
)

# Installation
install(TARGETS omnisight_perception
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${PERCEPTION_HEADERS}
    DESTINATION include/omnisight
)

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
