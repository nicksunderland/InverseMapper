# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.13.4/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.13.4/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/build

# Include any dependencies generated for this target.
include CMakeFiles/rvm_regression_ex.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/rvm_regression_ex.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rvm_regression_ex.dir/flags.make

CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.o: CMakeFiles/rvm_regression_ex.dir/flags.make
CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.o: ../rvm_regression_ex.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.o"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.o -c /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/rvm_regression_ex.cpp

CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.i"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/rvm_regression_ex.cpp > CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.i

CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.s"
	/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/rvm_regression_ex.cpp -o CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.s

# Object files for target rvm_regression_ex
rvm_regression_ex_OBJECTS = \
"CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.o"

# External object files for target rvm_regression_ex
rvm_regression_ex_EXTERNAL_OBJECTS =

rvm_regression_ex: CMakeFiles/rvm_regression_ex.dir/rvm_regression_ex.cpp.o
rvm_regression_ex: CMakeFiles/rvm_regression_ex.dir/build.make
rvm_regression_ex: dlib_build/libdlib.a
rvm_regression_ex: /usr/X11R6/lib/libSM.dylib
rvm_regression_ex: /usr/X11R6/lib/libICE.dylib
rvm_regression_ex: /usr/X11R6/lib/libX11.dylib
rvm_regression_ex: /usr/X11R6/lib/libXext.dylib
rvm_regression_ex: /usr/local/lib/libpng.dylib
rvm_regression_ex: /usr/lib/libz.dylib
rvm_regression_ex: /usr/lib/libcblas.dylib
rvm_regression_ex: /usr/lib/liblapack.dylib
rvm_regression_ex: /usr/lib/libsqlite3.dylib
rvm_regression_ex: CMakeFiles/rvm_regression_ex.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable rvm_regression_ex"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rvm_regression_ex.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rvm_regression_ex.dir/build: rvm_regression_ex

.PHONY : CMakeFiles/rvm_regression_ex.dir/build

CMakeFiles/rvm_regression_ex.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rvm_regression_ex.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rvm_regression_ex.dir/clean

CMakeFiles/rvm_regression_ex.dir/depend:
	cd /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/build /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/build /Users/nicholassunderland/Desktop/Dlib_Numerical_Library/dlib-19.17/examples/build/CMakeFiles/rvm_regression_ex.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/rvm_regression_ex.dir/depend

