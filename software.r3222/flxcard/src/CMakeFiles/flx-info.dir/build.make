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
include CMakeFiles/flx-info.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/flx-info.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/flx-info.dir/flags.make

CMakeFiles/flx-info.dir/flx-info.cpp.o: CMakeFiles/flx-info.dir/flags.make
CMakeFiles/flx-info.dir/flx-info.cpp.o: flx-info.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/echurch/felix/software.r3222/flxcard/src/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/flx-info.dir/flx-info.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/flx-info.dir/flx-info.cpp.o -c /home/echurch/felix/software.r3222/flxcard/src/flx-info.cpp

CMakeFiles/flx-info.dir/flx-info.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/flx-info.dir/flx-info.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/echurch/felix/software.r3222/flxcard/src/flx-info.cpp > CMakeFiles/flx-info.dir/flx-info.cpp.i

CMakeFiles/flx-info.dir/flx-info.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/flx-info.dir/flx-info.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/echurch/felix/software.r3222/flxcard/src/flx-info.cpp -o CMakeFiles/flx-info.dir/flx-info.cpp.s

CMakeFiles/flx-info.dir/flx-info.cpp.o.requires:
.PHONY : CMakeFiles/flx-info.dir/flx-info.cpp.o.requires

CMakeFiles/flx-info.dir/flx-info.cpp.o.provides: CMakeFiles/flx-info.dir/flx-info.cpp.o.requires
	$(MAKE) -f CMakeFiles/flx-info.dir/build.make CMakeFiles/flx-info.dir/flx-info.cpp.o.provides.build
.PHONY : CMakeFiles/flx-info.dir/flx-info.cpp.o.provides

CMakeFiles/flx-info.dir/flx-info.cpp.o.provides.build: CMakeFiles/flx-info.dir/flx-info.cpp.o

# Object files for target flx-info
flx__info_OBJECTS = \
"CMakeFiles/flx-info.dir/flx-info.cpp.o"

# External object files for target flx-info
flx__info_EXTERNAL_OBJECTS =

flx-info: CMakeFiles/flx-info.dir/flx-info.cpp.o
flx-info: CMakeFiles/flx-info.dir/build.make
flx-info: libFlxCard.so
flx-info: regmap/libregmap.a
flx-info: CMakeFiles/flx-info.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable flx-info"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/flx-info.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/flx-info.dir/build: flx-info
.PHONY : CMakeFiles/flx-info.dir/build

CMakeFiles/flx-info.dir/requires: CMakeFiles/flx-info.dir/flx-info.cpp.o.requires
.PHONY : CMakeFiles/flx-info.dir/requires

CMakeFiles/flx-info.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/flx-info.dir/cmake_clean.cmake
.PHONY : CMakeFiles/flx-info.dir/clean

CMakeFiles/flx-info.dir/depend:
	cd /home/echurch/felix/software.r3222/flxcard/src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard/src /home/echurch/felix/software.r3222/flxcard/src /home/echurch/felix/software.r3222/flxcard/src/CMakeFiles/flx-info.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/flx-info.dir/depend
