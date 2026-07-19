cmake_minimum_required(VERSION 3.16)

project(FIXProtocolEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# ============================
# Shared FIX Engine Library
# ============================

add_library(FIXCore
    src/encoder.cpp
    src/parser.cpp
    src/validator.cpp
    src/utils.cpp
    src/checksum.cpp
    src/message_validator.cpp
    src/fix_message.cpp
    src/body_length.cpp
    src/session.cpp
    src/execution_report.cpp
)

# ============================
# Main FIX Engine Demo
# ============================

add_executable(FIXEngine
    src/main.cpp
)

target_link_libraries(FIXEngine
    FIXCore
)

# ============================
# FIX Server
# ============================

add_executable(FIXServer
    apps/server.cpp
)

target_link_libraries(FIXServer
    FIXCore
    ws2_32
)

# ============================
# FIX Client
# ============================

add_executable(FIXClient
    apps/client.cpp
)

target_link_libraries(FIXClient
    FIXCore
    ws2_32
)