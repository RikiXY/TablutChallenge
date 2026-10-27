
cmake_minimum_required(VERSION 3.16)

project(TablutChallenge LANGUAGES CXX)

# C++ configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Executable
add_executable(TablutChallenge
    main.cpp

    # Core
    src/core/Board.cpp
    src/core/Game.cpp
    src/core/GameState.cpp
    src/core/Translator.cpp
    src/core/Turn.cpp
    src/core/Action.cpp
    src/core/Parser.cpp
    
    # Connections
    src/connections/ServerConnector.cpp
    
    # Agents
    src/agent/Player.cpp
    src/agent/Agent.cpp
    src/agent/Human.cpp
)

# Header files
target_include_directories(TablutChallenge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Compiler warnings
target_compile_options(TablutChallenge PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)


set_target_properties(TablutChallenge PROPERTIES OUTPUT_NAME player)