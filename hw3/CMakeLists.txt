cmake_minimum_required(VERSION 2.8.9)
project (hw3)
set(CMAKE_CXX_FLAGS_DEBUG)
add_definitions(-std=c++11)

#Directories
include_directories("${PROJECT_BINARY_DIR}")

#source files
file(GLOB SOURCES  "*.c")

#executable
add_executable(hw:wq.tsk  ${SOURCES}  )

