cmake_minimum_required(VERSION 3.10)

# set the project name
project(jishmo)

# add the executable
add_executable(jishmo jishmo.c)
target_link_libraries(jishmo avutil avformat)
 

