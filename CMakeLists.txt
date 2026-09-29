cmake_minimum_required(VERSION 3.16)

project(TablutChallenge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TablutChallenge

    # Entry point
    src/main.cpp

    # Core
    src/core/Board.cpp
    src/core/GameState.cpp
    src/core/Rules.cpp
    src/core/Core.cpp

    # Network
    src/network/JsonCodec.cpp
    src/network/ServerClient.cpp

    # Agent
    src/agent/Agent.cpp
    src/agent/FirstLegalAgent.cpp
)

target_include_directories(TablutChallenge PRIVATE include)