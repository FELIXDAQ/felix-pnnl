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
CMAKE_BINARY_DIR = /home/echurch/felix/software.r3222/flxcard/build

# Include any dependencies generated for this target.
include CMakeFiles/flx-irq-test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/flx-irq-test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/flx-irq-test.dir/flags.make

CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o: CMakeFiles/flx-irq-test.dir/flags.make
CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o: ../src/flx-irq-test.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/echurch/felix/software.r3222/flxcard/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o -c /home/echurch/felix/software.r3222/flxcard/src/flx-irq-test.cpp

CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/echurch/felix/software.r3222/flxcard/src/flx-irq-test.cpp > CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.i

CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/echurch/felix/software.r3222/flxcard/src/flx-irq-test.cpp -o CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.s

CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.requires:
.PHONY : CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.requires

CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.provides: CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.requires
	$(MAKE) -f CMakeFiles/flx-irq-test.dir/build.make CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.provides.build
.PHONY : CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.provides

CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.provides.build: CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o

# Object files for target flx-irq-test
flx__irq__test_OBJECTS = \
"CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o"

# External object files for target flx-irq-test
flx__irq__test_EXTERNAL_OBJECTS =

flx-irq-test: CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o
flx-irq-test: CMakeFiles/flx-irq-test.dir/build.make
flx-irq-test: libFlxCard.so
flx-irq-test: regmap/libregmap.a
flx-irq-test: CMakeFiles/flx-irq-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable flx-irq-test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/flx-irq-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/flx-irq-test.dir/build: flx-irq-test
.PHONY : CMakeFiles/flx-irq-test.dir/build

CMakeFiles/flx-irq-test.dir/requires: CMakeFiles/flx-irq-test.dir/src/flx-irq-test.cpp.o.requires
.PHONY : CMakeFiles/flx-irq-test.dir/requires

CMakeFiles/flx-irq-test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/flx-irq-test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/flx-irq-test.dir/clean

CMakeFiles/flx-irq-test.dir/depend:
	cd /home/echurch/felix/software.r3222/flxcard/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard/build /home/echurch/felix/software.r3222/flxcard/build /home/echurch/felix/software.r3222/flxcard/build/CMakeFiles/flx-irq-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/flx-irq-test.dir/depend

