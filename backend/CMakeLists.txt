# PURPOSE: Controls how the C++ code compiles
cmake_minimum_required(VERSION 3.15)
project(MeridianBackend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find libosmim and its required compression packages to be used
find_package(ZLIB REQUIRED)
find_package(EXPAT REQUIRED)
find_package(BZip2 REQUIRED)  # also needed package for reading file
find_path(OSMIUM_INCLUDE_DIR osmium/io/any_input.hpp
    #added PATHS keyword to explictly check the Apple and Intel Homebrew folders (i run homebrew)
    PATHS /opt/homebrew/include /usr/local/include
)    

# we need this bc `libosmium` can read/write OpenStreetMap (OSM) data
if(NOT OSMIUM_INCLUDE_DIR)
    message(FATAL_ERROR "libosmim headers not found! Please re-check installation.")
endif()

# another added library for PROTOZERO
find_path(PROTOZERO_INCLUDE_DIR protozero/exception.hpp
    PATHS /opt/homebrew/include /usr/local/include
)

if(NOT PROTOZERO_INCLUDE_DIR)
    message(FATAL_ERROR "protozero headers not found! Please run: brew install protozero")
endif()

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
    ${BZIP2_LIBRARIES} # added line
    pthread
)