# OMNISIGHT Swarm Intelligence Module
# CMake build configuration

cmake_minimum_required(VERSION 3.16)

# Swarm module library
add_library(omnisight_swarm STATIC
    swarm.c
    mqtt_client.c
    federated_learning.c
)

target_include_directories(omnisight_swarm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/perception
    ${CMAKE_SOURCE_DIR}/src/timeline
)

target_link_libraries(omnisight_swarm
    m            # Math library
    pthread      # POSIX threads
    # paho-mqtt3c  # Paho MQTT C library (install separately)
)

# Compiler flags
target_compile_options(omnisight_swarm PRIVATE
    -Wall
    -Wextra
    -O3
    -ffast-math
    -march=armv8-a  # ARTPEC-8 architecture
)

# Install headers
install(FILES
    swarm.h
    mqtt_client.h
    federated_learning.h
    DESTINATION include/omnisight/swarm
)

# Install library
install(TARGETS omnisight_swarm
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
