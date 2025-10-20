# OMNISIGHT Perception Engine
# CMakeLists.txt for building on ACAP Native SDK

cmake_minimum_required(VERSION 3.16)
project(omnisight_perception VERSION 0.1.0 LANGUAGES C CXX)

# ACAP SDK settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find ACAP SDK packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(VDO REQUIRED vdostream)
pkg_check_modules(LAROD REQUIRED larod)

# OpenCV (ARTPEC-optimized version from SDK)
find_package(OpenCV REQUIRED)

# Source files
set(PERCEPTION_SOURCES
    perception.c
    vdo_capture.c
    larod_inference.c
    tracker.c
    behavior.c
)

# Header files
set(PERCEPTION_HEADERS
    perception.h
    vdo_capture.h
    larod_inference.h
    tracker.h
    behavior.h
)

# Build shared library
add_library(omnisight_perception SHARED ${PERCEPTION_SOURCES})

# Include directories
target_include_directories(omnisight_perception
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${GLIB_INCLUDE_DIRS}
        ${VDO_INCLUDE_DIRS}
        ${LAROD_INCLUDE_DIRS}
        ${OpenCV_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(omnisight_perception
    PUBLIC
        ${GLIB_LIBRARIES}
        ${VDO_LIBRARIES}
        ${LAROD_LIBRARIES}
        ${OpenCV_LIBS}
        m  # Math library
        pthread
)

# Compiler options optimized for ARTPEC-8
target_compile_options(omnisight_perception PRIVATE
    -Wall
    -Wextra
    -Werror
    -O3
    -march=armv8-a
    -mtune=cortex-a53
    -ftree-vectorize
    -ffast-math
)

# Compiler definitions
target_compile_definitions(omnisight_perception PRIVATE
    _GNU_SOURCE
    ARTPEC8
    OMNISIGHT_VERSION="${PROJECT_VERSION}"
)

# Installation
install(TARGETS omnisight_perception
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(FILES ${PERCEPTION_HEADERS}
    DESTINATION include/omnisight
)

# Build test executable if BUILD_TESTING is enabled
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# Export compile commands for IDEs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
