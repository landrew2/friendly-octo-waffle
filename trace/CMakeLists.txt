cmake_minimum_required(VERSION 3.1)
project(trace)

# include all cpp and header files in project
file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.hpp")
add_executable(trace ${SOURCES} ${HEADERS})
