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
#include <map>
#include <fstream>

#include "cyth_parser.hpp"

class module_manager
{
private:
    make_cyth_parser cyth_parser_generator;

    std::list< std::string > module_fnames_to_parse;
    std::map<std::string, module_AST_ptr> parsed_modules; // maybe this shouldn't be a map?

public:
    module_manager(bool do_parserTable_IO=true);

    void add_module_fname_to_parse(std::string module_fname);

    module_AST_ptr parse_module(std::string module_fname, bool reporting=false);

    void parse_all_modules( bool reporting=false );

    std::list< module_AST_ptr> get_module_dependency_order();

    module_AST_ptr get_module( std::string file_name );

    bool build_module(module_AST_ptr module);
    
    bool compile_and_link();
};

#endif
