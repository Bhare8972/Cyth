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
#include "basic_AST_visitors.hpp"

using namespace std;
using namespace csu;

module_manager::module_manager(bool do_parserTable_IO) :
    cyth_parser_generator(do_parserTable_IO)
{

}


module_AST_ptr module_manager::parse_module(string module_fname, bool reporting)
{
    string module_name = module_fname.substr(0, module_fname.find('.'));

    auto found_fname = parsed_modules.find(module_name);
    if( found_fname != parsed_modules.end() )
    {   return found_fname->second; }

    shared_ptr<parser> cyth_parser =  cyth_parser_generator.get_parser();

    ifstream input_file;
    input_file.open(module_fname);
    cyth_parser->reset_input(input_file);

    dyn_holder module_output = cyth_parser->parse(reporting);
    module_AST_ptr module_data = *module_output.cast<module_AST_ptr>();

    //// here we process the AST into a usable state ////

    //first set the symbol table
    set_symbol_table set_vstr(module_data.get(), module_name);
    module_data->apply_visitor(&set_vstr);

    //define all names
    define_names define_vstr;
    module_data->apply_visitor(&define_vstr);

    //reference all names
    reference_names referance_vstr;
    module_data->apply_visitor(&referance_vstr);

    //register overloads
    register_overloads overloads_vstr;
    module_data->apply_visitor(&overloads_vstr);

    // set all types
    uint i=0;
    for( ; i<module_data->max_symbol_loops; i++)
    {
        build_types build_vstr;
        module_data->apply_visitor(&build_vstr);
        if( not build_vstr.changes_were_made )
        {
            break;
        }
    } // TODO: check number of loops
    cout<<"type checked in: "<<i<<" loops"<<endl;

    verify_symbol_table verify_vstr;
    module_data->apply_visitor(&verify_vstr);
    bool sym_table_good = (module_data->verification_state == 1);

    if( not sym_table_good)
    {
        cout << "ERROR: module '" << module_fname << "' symbols could not be defined." << endl;
        module_data = nullptr;
    }

    parsed_modules.insert( make_pair(module_name, module_data) );

    return module_data;
}


