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
include CMakeFiles/flx-spi.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/flx-spi.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/flx-spi.dir/flags.make

CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o: CMakeFiles/flx-spi.dir/flags.make
CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o: ../src/flx-spi.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/echurch/felix/software.r3222/flxcard/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o -c /home/echurch/felix/software.r3222/flxcard/src/flx-spi.cpp

CMakeFiles/flx-spi.dir/src/flx-spi.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/flx-spi.dir/src/flx-spi.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/echurch/felix/software.r3222/flxcard/src/flx-spi.cpp > CMakeFiles/flx-spi.dir/src/flx-spi.cpp.i

CMakeFiles/flx-spi.dir/src/flx-spi.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/flx-spi.dir/src/flx-spi.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/echurch/felix/software.r3222/flxcard/src/flx-spi.cpp -o CMakeFiles/flx-spi.dir/src/flx-spi.cpp.s

CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.requires:

.PHONY : CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.requires

CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.provides: CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.requires
	$(MAKE) -f CMakeFiles/flx-spi.dir/build.make CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.provides.build
.PHONY : CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.provides

CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.provides.build: CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o


# Object files for target flx-spi
flx__spi_OBJECTS = \
"CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o"

# External object files for target flx-spi
flx__spi_EXTERNAL_OBJECTS =

flx-spi: CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o
flx-spi: CMakeFiles/flx-spi.dir/build.make
flx-spi: libFlxCard.so
flx-spi: regmap/libregmap.a
flx-spi: CMakeFiles/flx-spi.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/echurch/felix/software.r3222/flxcard/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable flx-spi"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/flx-spi.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/flx-spi.dir/build: flx-spi

.PHONY : CMakeFiles/flx-spi.dir/build

CMakeFiles/flx-spi.dir/requires: CMakeFiles/flx-spi.dir/src/flx-spi.cpp.o.requires

.PHONY : CMakeFiles/flx-spi.dir/requires

CMakeFiles/flx-spi.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/flx-spi.dir/cmake_clean.cmake
.PHONY : CMakeFiles/flx-spi.dir/clean

CMakeFiles/flx-spi.dir/depend:
	cd /home/echurch/felix/software.r3222/flxcard/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard/build /home/echurch/felix/software.r3222/flxcard/build /home/echurch/felix/software.r3222/flxcard/build/CMakeFiles/flx-spi.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/flx-spi.dir/depend
