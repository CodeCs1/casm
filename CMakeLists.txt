cmake_minimum_required(VERSION 3.5)
set(CMAKE_DEBUG_POSTFIX d)
project(casm)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules")
find_package(argp REQUIRED)
if (NOT ARGP_FOUND)
    message(FATAL_MESSAGE "ARGP not found!")
endif()

enable_testing()
add_test(NAME Tokenizer_test COMMAND casm -i test/main.casm -o test/main)

include_directories(src/include)
add_executable(casm src/main.c 
src/ParseF/token.c 
src/ParseF/parse.c
src/ToBinary/expr2bin.c)