cmake_minimum_required(VERSION 3.15)
project(bearwasm CXX)
enable_language(ASM_NASM)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -O0 -g")
set(CMAKE_ASM_NASM_LINK_EXECUTABLE "ld <CMAKE_ASM_NASM_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_ASM_NASM_OBJECT_FORMAT macho64)

set(SOURCES src/main.cpp src/Module.cpp src/Interpreter.cpp
	src/VirtualMachine.cpp src/Util.cpp src/ASMInterpreter.asm)

add_executable(bearwasm ${SOURCES})
target_include_directories(bearwasm PUBLIC include/)

