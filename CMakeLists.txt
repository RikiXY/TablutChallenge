
cmake_minimum_required(VERSION 3.16)

project(TablutChallenge LANGUAGES CXX)

# C++ configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Adds everything as a library, shared by the player and the arena
add_library(tablut STATIC
    # Core
    src/core/Board.cpp
    src/core/Game.cpp
    src/core/Translator.cpp
    src/core/Rules.cpp

    # Engine
    src/engine/Evaluation.cpp
    src/engine/Search.cpp

    # Config
    src/config/Config.cpp
    
    # Connections
    src/connections/ServerConnector.cpp
    
    # Agents
    src/agent/Agent.cpp
    src/agent/Human.cpp
)

# Header files
target_include_directories(tablut PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Third-party headers with warnings hidden
target_include_directories(tablut SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Compiler warnings
target_compile_options(tablut PUBLIC
    -Wall
    -Wextra
    -Wpedantic
)

# Tournament player
add_executable(TablutChallenge main.cpp)
target_link_libraries(TablutChallenge PRIVATE tablut)
set_target_properties(TablutChallenge PROPERTIES OUTPUT_NAME player)

# Self-play arena
find_package(Threads REQUIRED)
add_executable(arena tools/Arena.cpp)
target_link_libraries(arena PRIVATE tablut Threads::Threads)
