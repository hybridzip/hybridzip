# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /home/supercmmetry/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/201.7846.88/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/supercmmetry/.local/share/JetBrains/Toolbox/apps/CLion/ch-0/201.7846.88/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/supercmmetry/Projects/hybridzip

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/supercmmetry/Projects/hybridzip/cmake-build-release

# Include any dependencies generated for this target.
include tests/CMakeFiles/gtest_run.dir/depend.make

# Include the progress variables for this target.
include tests/CMakeFiles/gtest_run.dir/progress.make

# Include the compile flags for this target's objects.
include tests/CMakeFiles/gtest_run.dir/flags.make

tests/CMakeFiles/gtest_run.dir/memory_test.cpp.o: tests/CMakeFiles/gtest_run.dir/flags.make
tests/CMakeFiles/gtest_run.dir/memory_test.cpp.o: ../tests/memory_test.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/supercmmetry/Projects/hybridzip/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object tests/CMakeFiles/gtest_run.dir/memory_test.cpp.o"
	cd /home/supercmmetry/Projects/hybridzip/cmake-build-release/tests && /usr/bin/clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/gtest_run.dir/memory_test.cpp.o -c /home/supercmmetry/Projects/hybridzip/tests/memory_test.cpp

tests/CMakeFiles/gtest_run.dir/memory_test.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/gtest_run.dir/memory_test.cpp.i"
	cd /home/supercmmetry/Projects/hybridzip/cmake-build-release/tests && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/supercmmetry/Projects/hybridzip/tests/memory_test.cpp > CMakeFiles/gtest_run.dir/memory_test.cpp.i

tests/CMakeFiles/gtest_run.dir/memory_test.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/gtest_run.dir/memory_test.cpp.s"
	cd /home/supercmmetry/Projects/hybridzip/cmake-build-release/tests && /usr/bin/clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/supercmmetry/Projects/hybridzip/tests/memory_test.cpp -o CMakeFiles/gtest_run.dir/memory_test.cpp.s

# Object files for target gtest_run
gtest_run_OBJECTS = \
"CMakeFiles/gtest_run.dir/memory_test.cpp.o"

# External object files for target gtest_run
gtest_run_EXTERNAL_OBJECTS =

tests/gtest_run: tests/CMakeFiles/gtest_run.dir/memory_test.cpp.o
tests/gtest_run: tests/CMakeFiles/gtest_run.dir/build.make
tests/gtest_run: lib/libgtest.a
tests/gtest_run: lib/libgtest_main.a
tests/gtest_run: src/hzip/libhzip.a
tests/gtest_run: lib/libgtest.a
tests/gtest_run: other/bitio/libbitio.a
tests/gtest_run: other/loguru/libloguru.a
tests/gtest_run: /opt/intel/mkl/lib/intel64/libmkl_core.so
tests/gtest_run: /opt/intel/mkl/lib/intel64/libmkl_sequential.so
tests/gtest_run: /opt/intel/mkl/lib/intel64/libmkl_intel_lp64.so
tests/gtest_run: /usr/lib/libdl.so
tests/gtest_run: tests/CMakeFiles/gtest_run.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/supercmmetry/Projects/hybridzip/cmake-build-release/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable gtest_run"
	cd /home/supercmmetry/Projects/hybridzip/cmake-build-release/tests && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/gtest_run.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
tests/CMakeFiles/gtest_run.dir/build: tests/gtest_run

.PHONY : tests/CMakeFiles/gtest_run.dir/build

tests/CMakeFiles/gtest_run.dir/clean:
	cd /home/supercmmetry/Projects/hybridzip/cmake-build-release/tests && $(CMAKE_COMMAND) -P CMakeFiles/gtest_run.dir/cmake_clean.cmake
.PHONY : tests/CMakeFiles/gtest_run.dir/clean

tests/CMakeFiles/gtest_run.dir/depend:
	cd /home/supercmmetry/Projects/hybridzip/cmake-build-release && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/supercmmetry/Projects/hybridzip /home/supercmmetry/Projects/hybridzip/tests /home/supercmmetry/Projects/hybridzip/cmake-build-release /home/supercmmetry/Projects/hybridzip/cmake-build-release/tests /home/supercmmetry/Projects/hybridzip/cmake-build-release/tests/CMakeFiles/gtest_run.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tests/CMakeFiles/gtest_run.dir/depend

