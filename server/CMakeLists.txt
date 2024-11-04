# Specify the minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(server)

# Set the C standard to C11 (you can adjust this as needed)
set(CMAKE_C_STANDARD 11)


# Add executable target with all source files
add_executable(server
        server.c
        server.h
        ../protocol.h
        ../protocol.c

)

# Add include directories (for header files)
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})


# Add compiler flags (optional)
target_compile_options(server PRIVATE -Wall -g)
