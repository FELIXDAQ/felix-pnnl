# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


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
CMAKE_COMMAND = /home/echurch/artdaq2/products/cmake/v3_5_1/Linux64bit+3.10-2.17/bin/cmake

# The command to remove a file.
RM = /home/echurch/artdaq2/products/cmake/v3_5_1/Linux64bit+3.10-2.17/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/echurch/felix/software.r3222/flxcard

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/echurch/felix/software.r3222/flxcard/build

# Include any dependencies generated for this target.
include regmap/CMakeFiles/regmap2.dir/depend.make

# Include the progress variables for this target.
include regmap/CMakeFiles/regmap2.dir/progress.make

# Include the compile flags for this target's objects.
include regmap/CMakeFiles/regmap2.dir/flags.make

regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o: regmap/CMakeFiles/regmap2.dir/flags.make
regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o: /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/echurch/felix/software.r3222/flxcard/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o"
	cd /home/echurch/felix/software.r3222/flxcard/build/regmap && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/regmap2.dir/src/regmap-symbol.c.o   -c /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c

regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/regmap2.dir/src/regmap-symbol.c.i"
	cd /home/echurch/felix/software.r3222/flxcard/build/regmap && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c > CMakeFiles/regmap2.dir/src/regmap-symbol.c.i

regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/regmap2.dir/src/regmap-symbol.c.s"
	cd /home/echurch/felix/software.r3222/flxcard/build/regmap && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/echurch/felix/software.r3222/regmap/src/regmap-symbol.c -o CMakeFiles/regmap2.dir/src/regmap-symbol.c.s

regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.requires:

.PHONY : regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.requires

regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.provides: regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.requires
	$(MAKE) -f regmap/CMakeFiles/regmap2.dir/build.make regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.provides.build
.PHONY : regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.provides

regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.provides.build: regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o


# Object files for target regmap2
regmap2_OBJECTS = \
"CMakeFiles/regmap2.dir/src/regmap-symbol.c.o"

# External object files for target regmap2
regmap2_EXTERNAL_OBJECTS =

regmap/libregmap2.so: regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o
regmap/libregmap2.so: regmap/CMakeFiles/regmap2.dir/build.make
regmap/libregmap2.so: regmap/CMakeFiles/regmap2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/echurch/felix/software.r3222/flxcard/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library libregmap2.so"
	cd /home/echurch/felix/software.r3222/flxcard/build/regmap && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/regmap2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
regmap/CMakeFiles/regmap2.dir/build: regmap/libregmap2.so

.PHONY : regmap/CMakeFiles/regmap2.dir/build

regmap/CMakeFiles/regmap2.dir/requires: regmap/CMakeFiles/regmap2.dir/src/regmap-symbol.c.o.requires

.PHONY : regmap/CMakeFiles/regmap2.dir/requires

regmap/CMakeFiles/regmap2.dir/clean:
	cd /home/echurch/felix/software.r3222/flxcard/build/regmap && $(CMAKE_COMMAND) -P CMakeFiles/regmap2.dir/cmake_clean.cmake
.PHONY : regmap/CMakeFiles/regmap2.dir/clean

regmap/CMakeFiles/regmap2.dir/depend:
	cd /home/echurch/felix/software.r3222/flxcard/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/regmap /home/echurch/felix/software.r3222/flxcard/build /home/echurch/felix/software.r3222/flxcard/build/regmap /home/echurch/felix/software.r3222/flxcard/build/regmap/CMakeFiles/regmap2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : regmap/CMakeFiles/regmap2.dir/depend

