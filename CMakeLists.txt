cmake_minimum_required(VERSION 3.0)
project(bearwasm)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -O2 -g")

set(SOURCES src/main.cpp src/Module.cpp src/Interpreter.cpp
	src/VirtualMachine.cpp src/Util.cpp)

add_executable(bearwasm ${SOURCES})
target_include_directories(bearwasm PUBLIC include/)

