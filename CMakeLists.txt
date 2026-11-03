
cmake_minimum_required(VERSION 3.16)

project(TablutChallenge LANGUAGES CXX)

# C++ configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Executable
add_executable(TablutChallenge
    main.cpp

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
target_include_directories(TablutChallenge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Third-party headers with warnings hidden
target_include_directories(TablutChallenge SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Compiler warnings
target_compile_options(TablutChallenge PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)


set_target_properties(TablutChallenge PROPERTIES OUTPUT_NAME player)