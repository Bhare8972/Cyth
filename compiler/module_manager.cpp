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

#include "module_manager.hpp"

using namespace std;
using namespace csu;


void module_manager::parse_module(string module_fname, bool reporting)
{
    if( parsed_modules.find(module_fname) != parsed_modules.end() ) return;

    shared_ptr<parser> cyth_parser =  cyth_parser_generator.get_parser();

    ifstream input_file;
    input_file.open(module_fname);
    cyth_parser->reset_input(input_file);

    dyn_holder module_output=cyth_parser->parse(reporting);
    //// need to check validity of module output here
    module_AST_ptr module_data;
    module_output.cast(module_data);

    parsed_modules.insert( make_pair(module_fname, module_data) );

}


