# created from template OpenCV included c++ project
#   by BreakDS 
#   Mar 23 2012 @ Univeristy of Wisconsin-Madison
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Ask CMake to search for current c/c++ compiler
set(CMAKE_CXX_COMPILER_INIT "/usr/bin/clang++")


# Project Description Starts Here
PROJECT(IconFitter)


# System & Project Information
MESSAGE(STATUS "Project Name:           ${PROJECT_NAME}")
MESSAGE(STATUS "Project Source Dir:     ${PROJECT_SOURCE_DIR}")
MESSAGE(STATUS "Kernel:                 ${CMAKE_SYSTEM}")
EXECUTE_PROCESS(COMMAND hostname OUTPUT_VARIABLE HOSTNAME)
MESSAGE(STATUS "Hostname:               ${HOSTNAME}")
MESSAGE(STATUS "Build Type:             ${CMAKE_BUILD_TYPE}")
MESSAGE(STATUS "C++:                    ${CMAKE_CXX_COMPILER}")

option (BUILD_UNITTEST "Build unit tests as well." ON)


# For breakds@AzraelWaker
include_directories("~/pf/projects" ".")
  
# Linker Flags
set(CMAKE_EXE_LINKER_FLAGS "-lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui -lopencv_core")
set(CMAKE_CXX_FLAGS "-std=c++0x")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -DNDEBUG -O3 -fopenmp")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fopenmp")
set(CMAKE_CXX_FLAGS_GPROF "-O1 -pg")


if (BUILD_UNITTEST)
endif (BUILD_UNITTEST)

ADD_EXECUTABLE(template_match template_match.cc)
ADD_EXECUTABLE(test test.cc)
