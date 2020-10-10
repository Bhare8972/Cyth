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

#include <list>
#include <algorithm>
#include <exception>
#include <cstdlib>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "module_manager.hpp"
#include "AST_visitor.hpp"
#include "basic_AST_visitors.hpp"
#include "writeAST_to_C.hpp"

using namespace std;
using namespace csu;

 //// some tools////

class cicular_dependancies_exception : public std::exception
{
public:

    virtual const char* what() const throw()
    {
        return "circular dependencies detected";
    }
};

string fname_to_moduleName( const string &module_fname )
{
    string fname = fs::path( module_fname ).filename();

    return fname.substr(0, fname.find('.'));
}

class gather_imports : public AST_visitorTraveler
{
public:

    list< string > imported_C_files;
    list< string > imported_cyth_fnames;

    void cImports_down(import_C_AST_node* ASTnode)
    {
        imported_C_files.push_back( ASTnode->file_name.to_cpp_string() );
    }

    void CythImports_down(import_cyth_AST_node* ASTnode)
    {
        imported_cyth_fnames.push_back( ASTnode->file_name.to_cpp_string() );
    }
};


 //// module manager ////
module_manager::module_manager(bool do_parserTable_IO) :
    cyth_parser_generator(do_parserTable_IO)
{

}

void module_manager::add_module_fname_to_parse(std::string module_fname)
{
    module_fnames_to_parse.push_back( module_fname );
}

module_AST_ptr module_manager::parse_module(string module_fname, bool reporting)
{
    string module_name = fname_to_moduleName( module_fname );

    auto found_fname = parsed_modules.find(module_name);
    if( found_fname != parsed_modules.end() )
    {   return found_fname->second; }

    cout << "parsing file " << module_fname << endl;

    shared_ptr<parser> cyth_parser =  cyth_parser_generator.get_parser();

    //ifstream input_file;
    //input_file.open(module_fname);
    //cyth_parser->reset_input(input_file);
    //utf8_string fname(module_fname);
    ifstream fin( module_fname );
    cyth_parser->reset_input(fin);

    dyn_holder module_output = cyth_parser->parse(reporting);
    module_AST_ptr module_data = *module_output.cast<module_AST_ptr>();

    module_data->module_name = module_name;
    module_data->file_name = module_fname;


    gather_imports gatherer;
    module_data->apply_visitor(&gatherer);

    module_data->imported_C_files = gatherer.imported_C_files;
   //module_data->imported_cyth_modules = gatherer.imported_cyth_modules;

    parsed_modules.insert( make_pair(module_name, module_data) );

    // place imported cyth files
    for( const string &cyth_fname : gatherer.imported_cyth_fnames )
    {
        string imported_module_name = fname_to_moduleName(cyth_fname  );
        module_data->imported_cyth_modules.push_back( imported_module_name );

        auto imported_is_parsed = parsed_modules.find( imported_module_name );
        if( imported_is_parsed == parsed_modules.end() )
        {
            // has not been parsed
            // check if it will be parsed

            bool in_ToParse = false;
            for( auto &module_fname_to_parse : module_fnames_to_parse)
            {
                string module_name_to_parse = fname_to_moduleName( module_fname_to_parse );
                if( module_name_to_parse == imported_module_name )
                {
                    in_ToParse = true;
                    break;
                }
            }

            if( not in_ToParse )
            {
                // not going to be parsed, so must be added to the list
                module_fnames_to_parse.push_back( cyth_fname );
            }
        }
    }

    return module_data;
}

void module_manager::parse_all_modules( bool reporting )
{
    while( module_fnames_to_parse.size() > 0 )
    {
        string top_module = module_fnames_to_parse.front();
        module_fnames_to_parse.pop_front();

        parse_module( top_module, reporting ); // can add more modules to parse
    }
}

list< module_AST_ptr> module_manager::get_module_dependency_order()
{
    list< module_AST_ptr > unused_modules;
    for (auto const& modules : parsed_modules)
    {
        unused_modules.push_back( modules.second );
    }


    list< module_AST_ptr > dependancy_ordered_modules;

    while( unused_modules.size() > 0 )
    {
        list< list< module_AST_ptr >::iterator > elements_to_move;
        for(auto iter = unused_modules.begin(); iter !=unused_modules.end();  ++iter )
        {
            bool dependancies_fufilled = true;
            for(auto &dependancy_module_name : (*iter)->imported_cyth_modules)
            {
                bool this_dependancy_found = false;
                for( auto resolved_module : dependancy_ordered_modules)
                {
                    if( resolved_module->module_name == dependancy_module_name )
                    {
                        this_dependancy_found = true;
                        break;
                    }
                }

                if( not this_dependancy_found)
                {
                    dependancies_fufilled = false;
                    break;
                }
            }

            if( dependancies_fufilled )
            {
                elements_to_move.push_back( iter );
            }
        }


        if( elements_to_move.size() == 0 )
        {
            cout << "circular cyth dependencies. Offending modules:" << endl;
            for( auto &module : unused_modules )
            {
                cout << "  " << module->module_name << " at " << module->file_name << endl;
            }
            throw cicular_dependancies_exception();

        }
        else
        {
            for( auto &iter_item : elements_to_move )
            {
                dependancy_ordered_modules.push_back( *iter_item );
                unused_modules.erase( iter_item );
            }
        }
    }

    return dependancy_ordered_modules;
}


module_AST_ptr module_manager::get_module( string file_name )
{
    string module_name = fname_to_moduleName( file_name );

    auto found_fname = parsed_modules.find(module_name);
    if( found_fname != parsed_modules.end() )
    {   return found_fname->second;
    }
    else
    {
        return nullptr;
    }
}

bool module_manager::build_module(module_AST_ptr module)
{
    //// here we process the AST into a usable state ////
    //first set the symbol table
    cout << "building module " << module->module_name << endl;

    set_symbol_table set_vstr(module.get());
    module->apply_visitor(&set_vstr);
    //define all names
    define_names define_vstr( this );
    module->apply_visitor(&define_vstr);

    //reference all names
    reference_names referance_vstr;
    module->apply_visitor(&referance_vstr);

    //register overloads
    register_overloads overloads_vstr;
    module->apply_visitor(&overloads_vstr);

    //register overloads
    build_classes class_vstr;
    module->apply_visitor(&class_vstr);



    // set all types
    uint i = 0;
    for( ; i<module->max_symbol_loops; i++)
    {
        bool debug = false;
        build_types build_vstr(debug);
        module->apply_visitor(&build_vstr);
        if( not build_vstr.changes_were_made )
        {
            break;
        }
    } // TODO: check number of loops
    cout<<"  type built in: "<< (i+1) <<" loops"<<endl;

    verify_symbol_table verify_vstr;
    module->apply_visitor(&verify_vstr);
    bool sym_table_good = (module->verification_state == 1);

    if( not sym_table_good)
    {
        cout << "ERROR: module '" << module->module_name << "' symbols could not be defined." << endl;
        module = nullptr;
        return false;

    }
    else
    {
        module->find_main_func();
        if( module->main_status == -1 )
        {
            module = nullptr;
            return false;
        }
    }

    cout << "  writing to C" << endl;
    write_module_to_C(module);

    cout << "  compiling" << endl;
    // eventually this shoudl be its own function/class
    string object_fname = module->C_source_fname + ".o";
    string command = "gcc " + module->C_source_fname + " -o " + object_fname + " -c";
    int tmp = system( command.c_str() );
    bool obj_file_exits =  fs::exists( object_fname ) and tmp==0;
    if( obj_file_exits and (module->main_status == 1)  )
    {
        string main_object_fname = module->C_mainSource_fname + ".o";
        command = "gcc " + module->C_mainSource_fname + " -o " + main_object_fname + " -c";
        tmp = system( command.c_str() );
        return fs::exists( main_object_fname ) and tmp==0;
    }
    else
    {
        return obj_file_exits;
    }
}

bool module_manager::compile_and_link()
{
    for (auto const& name_module_pair : parsed_modules)
    {
        auto module = name_module_pair.second;

        if( module->main_status == 1)
        {
            list< string > object_files_to_include;
            list< module_AST_ptr > modules_to_include;

            object_files_to_include.push_back( module->C_mainSource_fname + ".o" );
            modules_to_include.push_back( module );

            for( auto new_module : modules_to_include )
            {
                object_files_to_include.push_back( new_module->C_source_fname + ".o" );

                for(auto &import_cyth_name : new_module->imported_cyth_modules)
                {
                    modules_to_include.push_back( get_module( import_cyth_name ) );
                }

                for(auto &command : new_module->compiler_comands )
                {
                    string &full_cmd = command.command;
                    size_t first_space = full_cmd.find(' ');
                    auto cmd_name = full_cmd.substr(3, first_space-3);
                    if( cmd_name=="link" )
                    {
                        size_t second_space = full_cmd.find(' ', first_space+1);
                        string object_file_name = full_cmd.substr(first_space+1, second_space-first_space-1);
                        string object_run_cmd = full_cmd.substr(second_space+1);

                        int ignored __attribute__((unused));
                        ignored = system( object_run_cmd.c_str() );

                        object_files_to_include.push_back( object_file_name );

                    }
                }
            }


            string link_command = "gcc -o " + module->file_name + ".out";
            for( string &obj_fname :  object_files_to_include)
            {
                link_command += " " + obj_fname;
            }

            int ignored __attribute__((unused));
            ignored = system( link_command.c_str() );
        }
    }

    return true;
}
