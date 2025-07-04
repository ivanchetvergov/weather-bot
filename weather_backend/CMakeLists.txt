cmake_minimum_required(VERSION 3.14)
project(weather_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# all .cc files
file(GLOB_RECURSE SRC_FILES
    ${PROJECT_SOURCE_DIR}/controllers/src/*.cc
    ${PROJECT_SOURCE_DIR}/services/src/*.cc
    ${PROJECT_SOURCE_DIR}/models/*.cc
    ${PROJECT_SOURCE_DIR}/filters/*.cc
    ${PROJECT_SOURCE_DIR}/plugins/*.cc
)

# create executable
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${PROJECT_SOURCE_DIR}/main.cc
)

# include headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/controllers/include
    ${PROJECT_SOURCE_DIR}/services/include
    ${PROJECT_SOURCE_DIR}/models
)

# ling drogon
set(CMAKE_PREFIX_PATH "$ENV{HOME}/drogon-install")
find_package(Drogon REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)

# views
drogon_create_views(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/views
    ${CMAKE_CURRENT_BINARY_DIR}
)

# tests
add_subdirectory(test)
