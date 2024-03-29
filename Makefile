# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/knightzj/projects/caobot_miraicpp_v1.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/knightzj/projects/caobot_miraicpp_v1.0

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target install/strip
install/strip: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip

# Special rule for the target install/strip
install/strip/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing the project stripped..."
	/usr/bin/cmake -DCMAKE_INSTALL_DO_STRIP=1 -P cmake_install.cmake
.PHONY : install/strip/fast

# Special rule for the target install/local
install/local: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local

# Special rule for the target install/local
install/local/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Installing only the local directory..."
	/usr/bin/cmake -DCMAKE_INSTALL_LOCAL_ONLY=1 -P cmake_install.cmake
.PHONY : install/local/fast

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "No interactive CMake dialog available..."
	/usr/bin/cmake -E echo No\ interactive\ CMake\ dialog\ available.
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# Special rule for the target install
install: preinstall
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install

# Special rule for the target install
install/fast: preinstall/fast
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Install the project..."
	/usr/bin/cmake -P cmake_install.cmake
.PHONY : install/fast

# Special rule for the target list_install_components
list_install_components:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Available install components are: \"Unspecified\""
.PHONY : list_install_components

# Special rule for the target list_install_components
list_install_components/fast: list_install_components

.PHONY : list_install_components/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/knightzj/projects/caobot_miraicpp_v1.0/CMakeFiles /home/knightzj/projects/caobot_miraicpp_v1.0/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/knightzj/projects/caobot_miraicpp_v1.0/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named bot

# Build rule for target.
bot: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 bot
.PHONY : bot

# fast build rule for target.
bot/fast:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/build
.PHONY : bot/fast

#=============================================================================
# Target rules for targets named mirai-cpp

# Build rule for target.
mirai-cpp: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 mirai-cpp
.PHONY : mirai-cpp

# fast build rule for target.
mirai-cpp/fast:
	$(MAKE) -f mirai-cpp/CMakeFiles/mirai-cpp.dir/build.make mirai-cpp/CMakeFiles/mirai-cpp.dir/build
.PHONY : mirai-cpp/fast

src/logger.o: src/logger.cpp.o

.PHONY : src/logger.o

# target to build an object file
src/logger.cpp.o:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/logger.cpp.o
.PHONY : src/logger.cpp.o

src/logger.i: src/logger.cpp.i

.PHONY : src/logger.i

# target to preprocess a source file
src/logger.cpp.i:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/logger.cpp.i
.PHONY : src/logger.cpp.i

src/logger.s: src/logger.cpp.s

.PHONY : src/logger.s

# target to generate assembly for a file
src/logger.cpp.s:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/logger.cpp.s
.PHONY : src/logger.cpp.s

src/main.o: src/main.cpp.o

.PHONY : src/main.o

# target to build an object file
src/main.cpp.o:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/main.cpp.o
.PHONY : src/main.cpp.o

src/main.i: src/main.cpp.i

.PHONY : src/main.i

# target to preprocess a source file
src/main.cpp.i:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/main.cpp.i
.PHONY : src/main.cpp.i

src/main.s: src/main.cpp.s

.PHONY : src/main.s

# target to generate assembly for a file
src/main.cpp.s:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/main.cpp.s
.PHONY : src/main.cpp.s

src/module.o: src/module.cpp.o

.PHONY : src/module.o

# target to build an object file
src/module.cpp.o:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/module.cpp.o
.PHONY : src/module.cpp.o

src/module.i: src/module.cpp.i

.PHONY : src/module.i

# target to preprocess a source file
src/module.cpp.i:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/module.cpp.i
.PHONY : src/module.cpp.i

src/module.s: src/module.cpp.s

.PHONY : src/module.s

# target to generate assembly for a file
src/module.cpp.s:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/module.cpp.s
.PHONY : src/module.cpp.s

src/poker.o: src/poker.cpp.o

.PHONY : src/poker.o

# target to build an object file
src/poker.cpp.o:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/poker.cpp.o
.PHONY : src/poker.cpp.o

src/poker.i: src/poker.cpp.i

.PHONY : src/poker.i

# target to preprocess a source file
src/poker.cpp.i:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/poker.cpp.i
.PHONY : src/poker.cpp.i

src/poker.s: src/poker.cpp.s

.PHONY : src/poker.s

# target to generate assembly for a file
src/poker.cpp.s:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/poker.cpp.s
.PHONY : src/poker.cpp.s

src/robot.o: src/robot.cpp.o

.PHONY : src/robot.o

# target to build an object file
src/robot.cpp.o:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/robot.cpp.o
.PHONY : src/robot.cpp.o

src/robot.i: src/robot.cpp.i

.PHONY : src/robot.i

# target to preprocess a source file
src/robot.cpp.i:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/robot.cpp.i
.PHONY : src/robot.cpp.i

src/robot.s: src/robot.cpp.s

.PHONY : src/robot.s

# target to generate assembly for a file
src/robot.cpp.s:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/robot.cpp.s
.PHONY : src/robot.cpp.s

src/utils.o: src/utils.cpp.o

.PHONY : src/utils.o

# target to build an object file
src/utils.cpp.o:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/utils.cpp.o
.PHONY : src/utils.cpp.o

src/utils.i: src/utils.cpp.i

.PHONY : src/utils.i

# target to preprocess a source file
src/utils.cpp.i:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/utils.cpp.i
.PHONY : src/utils.cpp.i

src/utils.s: src/utils.cpp.s

.PHONY : src/utils.s

# target to generate assembly for a file
src/utils.cpp.s:
	$(MAKE) -f CMakeFiles/bot.dir/build.make CMakeFiles/bot.dir/src/utils.cpp.s
.PHONY : src/utils.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... install/strip"
	@echo "... install/local"
	@echo "... edit_cache"
	@echo "... install"
	@echo "... list_install_components"
	@echo "... rebuild_cache"
	@echo "... bot"
	@echo "... mirai-cpp"
	@echo "... src/logger.o"
	@echo "... src/logger.i"
	@echo "... src/logger.s"
	@echo "... src/main.o"
	@echo "... src/main.i"
	@echo "... src/main.s"
	@echo "... src/module.o"
	@echo "... src/module.i"
	@echo "... src/module.s"
	@echo "... src/poker.o"
	@echo "... src/poker.i"
	@echo "... src/poker.s"
	@echo "... src/robot.o"
	@echo "... src/robot.i"
	@echo "... src/robot.s"
	@echo "... src/utils.o"
	@echo "... src/utils.i"
	@echo "... src/utils.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -S$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

