# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/f/DockerFiles/ICT374-A2/shell

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/egg_shell.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/egg_shell.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/egg_shell.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/egg_shell.dir/flags.make

CMakeFiles/egg_shell.dir/main.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/main.c.o: ../main.c
CMakeFiles/egg_shell.dir/main.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/egg_shell.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/main.c.o -MF CMakeFiles/egg_shell.dir/main.c.o.d -o CMakeFiles/egg_shell.dir/main.c.o -c /mnt/f/DockerFiles/ICT374-A2/shell/main.c

CMakeFiles/egg_shell.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/shell/main.c > CMakeFiles/egg_shell.dir/main.c.i

CMakeFiles/egg_shell.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/shell/main.c -o CMakeFiles/egg_shell.dir/main.c.s

CMakeFiles/egg_shell.dir/signals.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/signals.c.o: ../signals.c
CMakeFiles/egg_shell.dir/signals.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/egg_shell.dir/signals.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/signals.c.o -MF CMakeFiles/egg_shell.dir/signals.c.o.d -o CMakeFiles/egg_shell.dir/signals.c.o -c /mnt/f/DockerFiles/ICT374-A2/shell/signals.c

CMakeFiles/egg_shell.dir/signals.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/signals.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/shell/signals.c > CMakeFiles/egg_shell.dir/signals.c.i

CMakeFiles/egg_shell.dir/signals.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/signals.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/shell/signals.c -o CMakeFiles/egg_shell.dir/signals.c.s

CMakeFiles/egg_shell.dir/command.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/command.c.o: ../command.c
CMakeFiles/egg_shell.dir/command.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/egg_shell.dir/command.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/command.c.o -MF CMakeFiles/egg_shell.dir/command.c.o.d -o CMakeFiles/egg_shell.dir/command.c.o -c /mnt/f/DockerFiles/ICT374-A2/shell/command.c

CMakeFiles/egg_shell.dir/command.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/command.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/shell/command.c > CMakeFiles/egg_shell.dir/command.c.i

CMakeFiles/egg_shell.dir/command.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/command.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/shell/command.c -o CMakeFiles/egg_shell.dir/command.c.s

CMakeFiles/egg_shell.dir/token.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/token.c.o: ../token.c
CMakeFiles/egg_shell.dir/token.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/egg_shell.dir/token.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/token.c.o -MF CMakeFiles/egg_shell.dir/token.c.o.d -o CMakeFiles/egg_shell.dir/token.c.o -c /mnt/f/DockerFiles/ICT374-A2/shell/token.c

CMakeFiles/egg_shell.dir/token.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/token.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/shell/token.c > CMakeFiles/egg_shell.dir/token.c.i

CMakeFiles/egg_shell.dir/token.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/token.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/shell/token.c -o CMakeFiles/egg_shell.dir/token.c.s

CMakeFiles/egg_shell.dir/history.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/history.c.o: ../history.c
CMakeFiles/egg_shell.dir/history.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/egg_shell.dir/history.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/history.c.o -MF CMakeFiles/egg_shell.dir/history.c.o.d -o CMakeFiles/egg_shell.dir/history.c.o -c /mnt/f/DockerFiles/ICT374-A2/shell/history.c

CMakeFiles/egg_shell.dir/history.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/history.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/shell/history.c > CMakeFiles/egg_shell.dir/history.c.i

CMakeFiles/egg_shell.dir/history.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/history.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/shell/history.c -o CMakeFiles/egg_shell.dir/history.c.s

CMakeFiles/egg_shell.dir/builtins.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/builtins.c.o: ../builtins.c
CMakeFiles/egg_shell.dir/builtins.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/egg_shell.dir/builtins.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/builtins.c.o -MF CMakeFiles/egg_shell.dir/builtins.c.o.d -o CMakeFiles/egg_shell.dir/builtins.c.o -c /mnt/f/DockerFiles/ICT374-A2/shell/builtins.c

CMakeFiles/egg_shell.dir/builtins.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/builtins.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/shell/builtins.c > CMakeFiles/egg_shell.dir/builtins.c.i

CMakeFiles/egg_shell.dir/builtins.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/builtins.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/shell/builtins.c -o CMakeFiles/egg_shell.dir/builtins.c.s

CMakeFiles/egg_shell.dir/terminal.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/terminal.c.o: ../terminal.c
CMakeFiles/egg_shell.dir/terminal.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object CMakeFiles/egg_shell.dir/terminal.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/terminal.c.o -MF CMakeFiles/egg_shell.dir/terminal.c.o.d -o CMakeFiles/egg_shell.dir/terminal.c.o -c /mnt/f/DockerFiles/ICT374-A2/shell/terminal.c

CMakeFiles/egg_shell.dir/terminal.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/terminal.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/shell/terminal.c > CMakeFiles/egg_shell.dir/terminal.c.i

CMakeFiles/egg_shell.dir/terminal.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/terminal.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/shell/terminal.c -o CMakeFiles/egg_shell.dir/terminal.c.s

CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o: /mnt/f/DockerFiles/ICT374-A2/protocol.c
CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o -MF CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o.d -o CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o -c /mnt/f/DockerFiles/ICT374-A2/protocol.c

CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/protocol.c > CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.i

CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/protocol.c -o CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.s

CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o: CMakeFiles/egg_shell.dir/flags.make
CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o: /mnt/f/DockerFiles/ICT374-A2/server/server.c
CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o: CMakeFiles/egg_shell.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building C object CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -MD -MT CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o -MF CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o.d -o CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o -c /mnt/f/DockerFiles/ICT374-A2/server/server.c

CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/f/DockerFiles/ICT374-A2/server/server.c > CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.i

CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/f/DockerFiles/ICT374-A2/server/server.c -o CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.s

# Object files for target egg_shell
egg_shell_OBJECTS = \
"CMakeFiles/egg_shell.dir/main.c.o" \
"CMakeFiles/egg_shell.dir/signals.c.o" \
"CMakeFiles/egg_shell.dir/command.c.o" \
"CMakeFiles/egg_shell.dir/token.c.o" \
"CMakeFiles/egg_shell.dir/history.c.o" \
"CMakeFiles/egg_shell.dir/builtins.c.o" \
"CMakeFiles/egg_shell.dir/terminal.c.o" \
"CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o" \
"CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o"

# External object files for target egg_shell
egg_shell_EXTERNAL_OBJECTS =

egg_shell: CMakeFiles/egg_shell.dir/main.c.o
egg_shell: CMakeFiles/egg_shell.dir/signals.c.o
egg_shell: CMakeFiles/egg_shell.dir/command.c.o
egg_shell: CMakeFiles/egg_shell.dir/token.c.o
egg_shell: CMakeFiles/egg_shell.dir/history.c.o
egg_shell: CMakeFiles/egg_shell.dir/builtins.c.o
egg_shell: CMakeFiles/egg_shell.dir/terminal.c.o
egg_shell: CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/protocol.c.o
egg_shell: CMakeFiles/egg_shell.dir/mnt/f/DockerFiles/ICT374-A2/server/server.c.o
egg_shell: CMakeFiles/egg_shell.dir/build.make
egg_shell: CMakeFiles/egg_shell.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Linking C executable egg_shell"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/egg_shell.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/egg_shell.dir/build: egg_shell
.PHONY : CMakeFiles/egg_shell.dir/build

CMakeFiles/egg_shell.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/egg_shell.dir/cmake_clean.cmake
.PHONY : CMakeFiles/egg_shell.dir/clean

CMakeFiles/egg_shell.dir/depend:
	cd /mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/f/DockerFiles/ICT374-A2/shell /mnt/f/DockerFiles/ICT374-A2/shell /mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug /mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug /mnt/f/DockerFiles/ICT374-A2/shell/cmake-build-debug/CMakeFiles/egg_shell.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/egg_shell.dir/depend

