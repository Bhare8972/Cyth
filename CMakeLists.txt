cmake_minimum_required(VERSION 2.8)

project("cyth")

##include utilities
set(utils ./utilities)
set(compiler ./compiler)

include_directories ("${utils}")
include_directories ("${compiler}")
include_directories ("./tclap")

##C++ settings
set(CMAKE_CXX_FLAGS "-Wno-sign-compare")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -Wall -Wno-sign-compare -Og -ggdb -D_GLIBCXX_DEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cyth
	      ${compiler}/main.cpp
          ${compiler}/cyth_parser.cpp
	      ${compiler}/module_manager.cpp
	      ${compiler}/cyth_AST.cpp
	      ${compiler}/sym_table.cpp
	      ${compiler}/writeAST_to_C.cpp
	      ${compiler}/AST_visitor.cpp
	      ${compiler}/basic_AST_visitors.cpp
	      ${compiler}/c_compiler_interface.cpp
	      ${compiler}/c_expressions.cpp
	      ${compiler}/c_source_writer.cpp

              ${utils}/lexer.cpp
              ${utils}/logger.cpp
              ${utils}/regex.cpp
              ${utils}/UTF8.cpp
              ${utils}/parser.cpp)


target_link_libraries(${PROJECT_NAME} stdc++fs)









