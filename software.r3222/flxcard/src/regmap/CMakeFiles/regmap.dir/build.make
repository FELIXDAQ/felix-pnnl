# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The program to use to edit the cache.
CMAKE_EDIT_COMMAND = /usr/bin/ccmake

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/echurch/felix/software.r3222/flxcard

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/echurch/felix/software.r3222/flxcard/src

# Include any dependencies generated for this target.
include regmap/CMakeFiles/regmap.dir/depend.make

# Include the progress variables for this target.
include regmap/CMakeFiles/regmap.dir/progress.make

# Include the compile flags for this target's objects.
include regmap/CMakeFiles/regmap.dir/flags.make

regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o: regmap/CMakeFiles/regmap.dir/flags.make
regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o: /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/echurch/felix/software.r3222/flxcard/src/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/regmap.dir/src/regmap-symbol.c.o   -c /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c

regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/regmap.dir/src/regmap-symbol.c.i"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c > CMakeFiles/regmap.dir/src/regmap-symbol.c.i

regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/regmap.dir/src/regmap-symbol.c.s"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c -o CMakeFiles/regmap.dir/src/regmap-symbol.c.s

regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.requires:
.PHONY : regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.requires

regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.provides: regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.requires
	$(MAKE) -f regmap/CMakeFiles/regmap.dir/build.make regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.provides.build
.PHONY : regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.provides

regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.provides.build: regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o

# Object files for target regmap
regmap_OBJECTS = \
"CMakeFiles/regmap.dir/src/regmap-symbol.c.o"

# External object files for target regmap
regmap_EXTERNAL_OBJECTS =

regmap/libregmap.a: regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o
regmap/libregmap.a: regmap/CMakeFiles/regmap.dir/build.make
regmap/libregmap.a: regmap/CMakeFiles/regmap.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C static library libregmap.a"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && $(CMAKE_COMMAND) -P CMakeFiles/regmap.dir/cmake_clean_target.cmake
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/regmap.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
regmap/CMakeFiles/regmap.dir/build: regmap/libregmap.a
.PHONY : regmap/CMakeFiles/regmap.dir/build

regmap/CMakeFiles/regmap.dir/requires: regmap/CMakeFiles/regmap.dir/src/regmap-symbol.c.o.requires
.PHONY : regmap/CMakeFiles/regmap.dir/requires

regmap/CMakeFiles/regmap.dir/clean:
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && $(CMAKE_COMMAND) -P CMakeFiles/regmap.dir/cmake_clean.cmake
.PHONY : regmap/CMakeFiles/regmap.dir/clean

regmap/CMakeFiles/regmap.dir/depend:
	cd /home/echurch/felix/software.r3222/flxcard/src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/regmap /home/echurch/felix/software.r3222/flxcard/src /home/echurch/felix/software.r3222/flxcard/src/regmap /home/echurch/felix/software.r3222/flxcard/src/regmap/CMakeFiles/regmap.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : regmap/CMakeFiles/regmap.dir/depend

