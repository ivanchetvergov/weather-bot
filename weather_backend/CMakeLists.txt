cmake_minimum_required(VERSION 3.14)
project(weather_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Создаем временный список для всех наших префиксных путей
set(WEATHER_BACKEND_CMAKE_PREFIX_PATHS "")

# 1. Добавляем путь к вашей установке Drogon в наш список ПЕРВЫМ!
list(APPEND WEATHER_BACKEND_CMAKE_PREFIX_PATHS "/Users/ivan/prog/libs/drogon_installed")

# 2. Собираем пути для Homebrew зависимостей
execute_process(COMMAND brew --prefix openssl@3 OUTPUT_VARIABLE OPENSSL_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND brew --prefix c-ares OUTPUT_VARIABLE CARES_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND brew --prefix postgresql@14 OUTPUT_VARIABLE PG_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE) 

# 3. Добавляем пути Homebrew зависимостей в наш список
list(APPEND WEATHER_BACKEND_CMAKE_PREFIX_PATHS
    "${OPENSSL_PREFIX}"
    "${CARES_PREFIX}"
    "${PG_PREFIX}"
)

# 4. И только теперь присваиваем весь собранный список CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "${WEATHER_BACKEND_CMAKE_PREFIX_PATHS}")

# collect all .cc files
file(GLOB_RECURSE SRC_FILES
    #${PROJECT_SOURCE_DIR}/controllers/src/*.cc
    ${PROJECT_SOURCE_DIR}/services/src/*.cc
    ${PROJECT_SOURCE_DIR}/parsers/src/*.cc
    ${PROJECT_SOURCE_DIR}/kafka/src/*.cc
    ${PROJECT_SOURCE_DIR}/command_logic/src/*.cc
    ${PROJECT_SOURCE_DIR}/models/*.cc
    ${PROJECT_SOURCE_DIR}/filters/*.cc
    ${PROJECT_SOURCE_DIR}/plugins/*.cc
)

# main target
add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${PROJECT_SOURCE_DIR}/main.cc
)

# include headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
    #${PROJECT_SOURCE_DIR}/controllers/include
    ${PROJECT_SOURCE_DIR}/parsers/include
    ${PROJECT_SOURCE_DIR}/command_logic/include
    ${PROJECT_SOURCE_DIR}/services/include
    ${PROJECT_SOURCE_DIR}/kafka/include
    ${PROJECT_SOURCE_DIR}/models
)

# Теперь find_package(Drogon...) найдет его правильно
find_package(Drogon REQUIRED CONFIG)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)


# views
drogon_create_views(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/views
    ${CMAKE_CURRENT_BINARY_DIR}
)

# tests
add_subdirectory(test)

# Пути к CppKafka и Librdkafka
# Это путь, куда ты установил cppkafka с помощью make install
set(CPPKAFKA_INSTALL_PREFIX "/Users/ivan/prog/libs/cppkafka_installed")

# Путь к librdkafka, установленной через Homebrew
execute_process(COMMAND brew --prefix librdkafka OUTPUT_VARIABLE RDKAFKA_PREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)

# Добавляем пути к заголовочным файлам для cppkafka и librdkafka
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CPPKAFKA_INSTALL_PREFIX}/include # Заголовочные файлы cppkafka
    ${RDKAFKA_PREFIX}/include          # Заголовочные файлы librdkafka
)

# Линкуем с библиотеками cppkafka и librdkafka
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # Используем правильное имя и расширение файла .dylib
        ${CPPKAFKA_INSTALL_PREFIX}/lib/libcppkafka.dylib
        ${RDKAFKA_PREFIX}/lib/librdkafka.dylib
)