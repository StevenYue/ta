cmake_minimum_required(VERSION 2.8.9)
project (hw2)
set(CMAKE_CXX_FLAGS_DEBUG)
add_definitions(-w -std=gnu99 -c -ggdb3)

#Directories
include_directories("${PROJECT_BINARY_DIR}")

#source files
file(GLOB SOURCES  "*.c")

#executable
add_executable(hw2.tsk  ${SOURCES}  )

