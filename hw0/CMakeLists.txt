cmake_minimum_required(VERSION 2.8.9)
project (hw0)
set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
add_definitions(-w -g)


#Directories
include_directories("${PROJECT_BINARY_DIR}")

#source files
file(GLOB main  "main.c")
file(GLOB map   "map.c")
file(GLOB wc    "wc.c")

#executable
add_executable(main  ${main}  )
add_executable(map  ${map}  )
add_executable(wc  ${wc}  )

