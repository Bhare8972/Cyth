/*
Copyright 2020 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file defines a class that manages parsed modules
*/


#include <cstdlib>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "c_compiler_interface.hpp"

using namespace std;

bool GCC_compiler_interface::compile_Csource(string& source_fname, string& output_fname)
{
    string command = "gcc " + source_fname + " -o " + output_fname + " -c";
    int tmp = system( command.c_str() );
    return fs::exists( output_fname ) and tmp==0;
}



bool GCC_compiler_interface::link(string& out_fname, list< string > object_files_to_include)
{
    string link_command = "gcc -o " + out_fname;
    for( string &obj_fname :  object_files_to_include)
    {
        link_command += " " + obj_fname;
    }

    int tmp = system( link_command.c_str() );
    return fs::exists( out_fname ) and tmp==0;
}
