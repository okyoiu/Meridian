# PURPOSE: Controls how the C++ code compiles
cmake_minimum_required(VERSION 3.15)
project(MeridianBackend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libosmim and its required compression packages to be used
find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
find_package(BZip2 REQUIRED)  # also needed package for reading file

find_path(OSMIUM_INCLUDE_DIR osmium/io/any_input.hpp PATHS /opt/homebrew/include /usr/local/include)
find_path(PROTOZERO_INCLUDE_DIR protozero/exception.hpp PATHS /opt/homebrew/include /usr/local/include)

if(NOT OSMIUM_INCLUDE_DIR OR NOT PROTOZERO_INCLUDE_DIR)
    message(FATAL_ERROR "Missing libosmium or protozero headers!")
endif()

# letting CMake automatically find our Web and JSON frameworks
find_package(Crow REQUIRED)
find_package(nlohmann_json REQUIRED)

# Defining executable
add_executable(server main.cpp parser.cpp router.cpp)

# Including directories
target_include_directories(server PRIVATE 
${OSMIUM_INCLUDE_DIR} 
    ${PROTOZERO_INCLUDE_DIR} 
    ${EXPAT_INCLUDE_DIRS} 
    ${ZLIB_INCLUDE_DIRS}
)

# Linking libraries
target_link_libraries(server PRIVATE 
${EXPAT_LIBRARIES} 
    ${ZLIB_LIBRARIES} 
    ${BZIP2_LIBRARIES} 
    Crow::Crow 
    nlohmann_json::nlohmann_json 
    pthread
)