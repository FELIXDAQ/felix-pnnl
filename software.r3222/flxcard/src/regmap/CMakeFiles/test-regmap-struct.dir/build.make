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
include regmap/CMakeFiles/test-regmap-struct.dir/depend.make

# Include the progress variables for this target.
include regmap/CMakeFiles/test-regmap-struct.dir/progress.make

# Include the compile flags for this target's objects.
include regmap/CMakeFiles/test-regmap-struct.dir/flags.make

regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o: regmap/CMakeFiles/test-regmap-struct.dir/flags.make
regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o: /home/echurch/felix/software.r3222/regmap/test/test-regmap-struct.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/echurch/felix/software.r3222/flxcard/src/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o   -c /home/echurch/felix/software.r3222/regmap/test/test-regmap-struct.c

regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.i"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/echurch/felix/software.r3222/regmap/test/test-regmap-struct.c > CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.i

regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.s"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/echurch/felix/software.r3222/regmap/test/test-regmap-struct.c -o CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.s

regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.requires:
.PHONY : regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.requires

regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.provides: regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.requires
	$(MAKE) -f regmap/CMakeFiles/test-regmap-struct.dir/build.make regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.provides.build
.PHONY : regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.provides

regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.provides.build: regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o

# Object files for target test-regmap-struct
test__regmap__struct_OBJECTS = \
"CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o"

# External object files for target test-regmap-struct
test__regmap__struct_EXTERNAL_OBJECTS =

regmap/test-regmap-struct: regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o
regmap/test-regmap-struct: regmap/CMakeFiles/test-regmap-struct.dir/build.make
regmap/test-regmap-struct: regmap/libregmap.a
regmap/test-regmap-struct: regmap/CMakeFiles/test-regmap-struct.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable test-regmap-struct"
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test-regmap-struct.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
regmap/CMakeFiles/test-regmap-struct.dir/build: regmap/test-regmap-struct
.PHONY : regmap/CMakeFiles/test-regmap-struct.dir/build

regmap/CMakeFiles/test-regmap-struct.dir/requires: regmap/CMakeFiles/test-regmap-struct.dir/test/test-regmap-struct.c.o.requires
.PHONY : regmap/CMakeFiles/test-regmap-struct.dir/requires

regmap/CMakeFiles/test-regmap-struct.dir/clean:
	cd /home/echurch/felix/software.r3222/flxcard/src/regmap && $(CMAKE_COMMAND) -P CMakeFiles/test-regmap-struct.dir/cmake_clean.cmake
.PHONY : regmap/CMakeFiles/test-regmap-struct.dir/clean

regmap/CMakeFiles/test-regmap-struct.dir/depend:
	cd /home/echurch/felix/software.r3222/flxcard/src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/regmap /home/echurch/felix/software.r3222/flxcard/src /home/echurch/felix/software.r3222/flxcard/src/regmap /home/echurch/felix/software.r3222/flxcard/src/regmap/CMakeFiles/test-regmap-struct.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : regmap/CMakeFiles/test-regmap-struct.dir/depend

