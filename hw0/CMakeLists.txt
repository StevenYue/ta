cmake_minimum_required(VERSION 2.8.9)
project (hw0)
set(CMAKE_CXX_FLAGS_DEBUG)
add_definitions(-w)


#Directories
include_directories("${PROJECT_BINARY_DIR}")

#source files
file(GLOB main  "main.c")
file(GLOB map   "map.c")
file(GLOB wc    "wc.c")

#executable
add_executable(main.tsk  ${main}  )
add_executable(map.tsk  ${map}  )
add_executable(wc.tsk  ${wc}  )

