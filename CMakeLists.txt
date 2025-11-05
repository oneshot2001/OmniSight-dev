# OMNISIGHT - Precognitive Security System
# Main CMakeLists.txt - Stub build configuration (no hardware dependencies)

cmake_minimum_required(VERSION 3.16)
project(OMNISIGHT VERSION 0.1.0 LANGUAGES C)

# C Standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Options
option(BUILD_TESTING "Build test programs" OFF)
option(BUILD_EXAMPLES "Build example programs" OFF)
option(ENABLE_TIMELINE "Enable Timeline Threading module" ON)
option(ENABLE_SWARM "Enable Swarm Intelligence module" ON)
option(ENABLE_PERCEPTION "Enable Perception engine" ON)

# Find required packages
find_package(Threads REQUIRED)

# Common include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/perception
  ${CMAKE_CURRENT_SOURCE_DIR}/src/timeline
  ${CMAKE_CURRENT_SOURCE_DIR}/src/swarm
  ${CMAKE_CURRENT_SOURCE_DIR}/src/http
)

# Add subdirectories for modules
if(ENABLE_PERCEPTION)
  add_subdirectory(src/perception)
endif()

if(ENABLE_TIMELINE)
  add_subdirectory(src/timeline)
endif()

if(ENABLE_SWARM)
  add_subdirectory(src/swarm)
endif()

# HTTP server library
add_library(omnisight_http STATIC
  src/http/mongoose.c
  src/http/http_server.c
)

target_compile_options(omnisight_http PRIVATE
  -Wall
  -Wextra
  -O2
  -DMG_ENABLE_PACKED_FS=0
)

# Main application executable
add_executable(omnisight
  src/main.c
  src/omnisight_core.c
)

# Link modules
target_link_libraries(omnisight
  PRIVATE
    omnisight_perception
    omnisight_timeline
    omnisight_swarm
    omnisight_http
    Threads::Threads
    m
)

# Compiler optimizations (architecture-agnostic for stub build)
target_compile_options(omnisight PRIVATE
  -Wall
  -Wextra
  -O2
)

# Compiler definitions
target_compile_definitions(omnisight PRIVATE
  _GNU_SOURCE
  OMNISIGHT_VERSION="${PROJECT_VERSION}"
  OMNISIGHT_STUB_BUILD=1
)

# Test executable for HTTP server
if(BUILD_TESTING)
  add_executable(test_http_server
    src/http/test_http_server.c
  )

  target_link_libraries(test_http_server
    PRIVATE
      omnisight_perception
      omnisight_timeline
      omnisight_swarm
      omnisight_http
      Threads::Threads
      m
  )

  target_compile_definitions(test_http_server PRIVATE
    _GNU_SOURCE
    OMNISIGHT_VERSION="${PROJECT_VERSION}"
    OMNISIGHT_STUB_BUILD=1
  )
endif()

# Installation
install(TARGETS omnisight
  RUNTIME DESTINATION bin
)

# Package configuration
set(CPACK_PACKAGE_NAME "omnisight")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "OMNISIGHT")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OMNISIGHT Precognitive Security System")
set(CPACK_GENERATOR "TGZ")

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "OMNISIGHT Stub Build Configuration:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  C compiler:        ${CMAKE_C_COMPILER}")
message(STATUS "  Build mode:        STUB (no hardware dependencies)")
message(STATUS "")
message(STATUS "Modules:")
message(STATUS "  Perception:        ${ENABLE_PERCEPTION} (stub)")
message(STATUS "  Timeline:          ${ENABLE_TIMELINE} (stub)")
message(STATUS "  Swarm:             ${ENABLE_SWARM} (stub)")
message(STATUS "  Testing:           ${BUILD_TESTING}")
message(STATUS "")
