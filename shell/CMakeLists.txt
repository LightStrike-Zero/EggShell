# Specify the minimum version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name
project(egg_shell_project)

# Set the C standard to C11 (you can adjust this as needed)
set(CMAKE_C_STANDARD 11)


# Add executable target with all source files
add_executable(egg_shell
        main.c
        signals.c
        command.c
        token.c
        history.c
        builtins.c
        terminal.c
        ../protocol.c
        ../server/server.c
)

# Add include directories (for header files)
target_include_directories(egg_shell PRIVATE ${CMAKE_SOURCE_DIR})

# Add compiler flags (optional)
target_compile_options(egg_shell PRIVATE -Wall -g)
