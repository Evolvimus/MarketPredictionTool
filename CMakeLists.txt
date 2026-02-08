cmake_minimum_required(VERSION 3.15)
project(predict_app CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# CPR for HTTP requests
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        871ed52d350214a034f6ef8d3b8f51c5ce1bb4c4 # 1.10.0
)
FetchContent_MakeAvailable(cpr)

# nlohmann_json for JSON parsing
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(json)

add_executable(predict_app
    src/main.cpp
    src/market_data.cpp
    src/analysis.cpp
    src/ollama_client.cpp
)

target_link_libraries(predict_app PRIVATE cpr::cpr nlohmann_json::nlohmann_json)

target_include_directories(predict_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
