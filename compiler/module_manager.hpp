/*
Copyright 2015 Brian Hare

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

#ifndef MODULE_MANAGER_170212090515
#define MODULE_MANAGER_170212090515

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <memory>

#include "tclap/CmdLine.h"

#include "cyth_parser.hpp"
#include "c_compiler_interface.hpp"


extern std::map<std::string, std::vector<std::string> > default_language_imports;
// this defines all the cyth imports to inject into each module to define base language features
// key is import location, value is vector of names to import
// location should be file in lib/cyth/lang folder



class command_line_options
/// this wraps over tclap to parse the command-line options
{
    TCLAP::CmdLine parser;
    TCLAP::MultiArg<std::string> input_files; // list of input cyth files to parse
    TCLAP::ValueArg<std::string> library_location; // this is required. location of the library folder
    TCLAP::ValueArg<std::string> output_location; // this is optional. If provided, executable will be placed here
    TCLAP::ValueArg<std::string> working_location; // this is optional. If provided, working-files will be placed here
    TCLAP::SwitchArg parserReporting; // if parser does reporting, default False
public:
    command_line_options(int argc, char** argv);

    std::vector< std::string > get_input_files();
    std::string get_library_location();

    bool has_output_location();
    std::string get_output_location();

    bool has_working_location();
    std::string get_working_location();

    bool get_parserReporting();
};
typedef std::shared_ptr<command_line_options> cmmnd_line_ptr;


class module_manager
{
private:
    cmmnd_line_ptr cmmd_line_optns;
    std::shared_ptr<c_compiler_interface> c_interface;

    make_cyth_parser cyth_parser_generator;

    std::list< std::string > module_fnames_to_parse; // these probably need to be cannonical absolute
    std::map<std::string, module_AST_ptr> parsed_modules; // keys are cannonacle absolute fnames

    std::string library_location; // set in constructor, from cmmd_line_optns

    // this is a helper function to compile modules, used by compile_and_link
    bool compile_recursively(module_AST_ptr module);



public:
    module_manager(cmmnd_line_ptr _cmmd_line_optns, bool do_parserTable_IO=true);

    void add_module_fname_to_parse(std::string module_fname); // will convert to canonical absolute

    module_AST_ptr parse_module(std::string canonical_module_fname, bool reporting=false);

    void parse_all_modules( bool reporting=false );

    std::list< module_AST_ptr> get_module_dependency_order();

    module_AST_ptr get_module( std::string canonical_module_fname );

    bool build_module(module_AST_ptr module);

    bool compile_and_link();



    // returns absolute location (appended by file name) of c_file imported by module, to be findable by gcc
    // returns "" if cannot find file
    // includes object files ????
    // searches in order:
    //      absolute location, c-compiler library (not yet), cyth c-header library, cyth c-source libarary, same directory as importing_module
    std::string locate_C_file(std::string c_file_name, module_AST_ptr importing_module);

    // module import string, find module and return canonical absolute string
    // at moment assumes string is a normal path to a file ending in .cy (but itself does NOT end in cy)
    // checks cyth library, local path, then absolute location
    std::string locate_cyth_file(std::string module_import_string, module_AST_ptr importing_module);

};

#endif
