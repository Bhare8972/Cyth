cmake_minimum_required(VERSION 2.8)

project("cyth")

##include utilities
set(utils ./utilities)
set(compiler ./compiler)

include_directories ("${utils}")
include_directories ("${compiler}")

##C++ settings
set(CMAKE_CXX_FLAGS "-std=c++0x -Wno-sign-compare")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -Wall -Wno-sign-compare -Og -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")


add_executable(cyth
	      ${compiler}/main.cpp
          ${compiler}/cyth_parser.cpp
	      ${compiler}/module_manager.cpp
	      ${compiler}/cyth_AST.cpp
	      ${compiler}/sym_table.cpp
	      ${compiler}/writeAST_to_C.cpp
	      ${compiler}/AST_visitor.cpp
	      ${compiler}/basic_AST_visitors.cpp

              ${utils}/lexer.cpp
              ${utils}/logger.cpp
              ${utils}/regex.cpp
              ${utils}/UTF8.cpp
              ${utils}/parser.cpp)












