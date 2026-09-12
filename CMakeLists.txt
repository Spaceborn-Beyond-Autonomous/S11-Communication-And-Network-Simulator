cmake_minimum_required(VERSION 3.16)

project(S11_Communication_Network_Simulator
    VERSION 0.1.0
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(S11_SOURCES
    src/main.c
    
    src/core/network_manager.c
    src/core/packet_manager.c
    src/core/config_manager.c

    src/models/latency_model.c
    src/models/packet_loss_model.c
    src/models/jitter_model.c
    src/models/bandwidth_model.c
    src/models/jamming_model.c

    src/protocols/ros2_adapter.c
    src/protocols/dds_adapter.c
    src/protocols/mavlink_adapter.c

    src/topology/link_manager.c
    src/topology/partition_manager.c
    src/topology/recovery_manager.c

    src/logger/network_logger.c
    src/monitor/network_monitor.c
)

add_executable(s11_simulator
    ${S11_SOURCES}
)

target_include_directories(s11_simulator
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)