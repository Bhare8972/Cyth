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

#ifndef C_COMPILER_INTERFACE_483011181020
#define C_COMPILER_INTERFACE_483011181020

#include <string>
#include <list>

class c_compiler_interface
{
public:

    virtual bool compile_Csource(std::string& source_fname, std::string& output_fname) = 0;
    virtual bool link(std::string& out_fname, std::list< std::string > object_files_to_include) = 0;

};

class GCC_compiler_interface : public c_compiler_interface
{
public:
    bool compile_Csource(std::string& source_fname, std::string& output_fname);
    bool link(std::string& out_fname, std::list< std::string > object_files_to_include);
};

#endif
