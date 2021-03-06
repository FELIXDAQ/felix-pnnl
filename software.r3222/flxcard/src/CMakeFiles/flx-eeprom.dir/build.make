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
include CMakeFiles/flx-eeprom.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/flx-eeprom.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/flx-eeprom.dir/flags.make

CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o: CMakeFiles/flx-eeprom.dir/flags.make
CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o: flx-eeprom.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report /home/echurch/felix/software.r3222/flxcard/src/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_FLAGS) -o CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o -c /home/echurch/felix/software.r3222/flxcard/src/flx-eeprom.cpp

CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -E /home/echurch/felix/software.r3222/flxcard/src/flx-eeprom.cpp > CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.i

CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_FLAGS) -S /home/echurch/felix/software.r3222/flxcard/src/flx-eeprom.cpp -o CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.s

CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.requires:
.PHONY : CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.requires

CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.provides: CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.requires
	$(MAKE) -f CMakeFiles/flx-eeprom.dir/build.make CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.provides.build
.PHONY : CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.provides

CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.provides.build: CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o

# Object files for target flx-eeprom
flx__eeprom_OBJECTS = \
"CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o"

# External object files for target flx-eeprom
flx__eeprom_EXTERNAL_OBJECTS =

flx-eeprom: CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o
flx-eeprom: CMakeFiles/flx-eeprom.dir/build.make
flx-eeprom: libFlxCard.so
flx-eeprom: regmap/libregmap.a
flx-eeprom: CMakeFiles/flx-eeprom.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX executable flx-eeprom"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/flx-eeprom.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/flx-eeprom.dir/build: flx-eeprom
.PHONY : CMakeFiles/flx-eeprom.dir/build

CMakeFiles/flx-eeprom.dir/requires: CMakeFiles/flx-eeprom.dir/flx-eeprom.cpp.o.requires
.PHONY : CMakeFiles/flx-eeprom.dir/requires

CMakeFiles/flx-eeprom.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/flx-eeprom.dir/cmake_clean.cmake
.PHONY : CMakeFiles/flx-eeprom.dir/clean

CMakeFiles/flx-eeprom.dir/depend:
	cd /home/echurch/felix/software.r3222/flxcard/src && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard /home/echurch/felix/software.r3222/flxcard/src /home/echurch/felix/software.r3222/flxcard/src /home/echurch/felix/software.r3222/flxcard/src/CMakeFiles/flx-eeprom.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/flx-eeprom.dir/depend

