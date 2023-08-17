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

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "module_manager.hpp"
#include "AST_visitor.hpp"
#include "basic_AST_visitors.hpp"
#include "writeAST_to_C.hpp"

using namespace std;
using namespace csu;


/// default imported langage features ///

map<string, vector<string> > default_language_imports =
{
    {"/lang/bool_int_long", {"bool","int","long","true","false"}}
};



/// command line parser
command_line_options::command_line_options( int argc, char** argv ) :
    parser("Cyth language compiler", ' ', "pre-alpha"),
    input_files("i", "input", "cyth file to compile", true, "string", parser),
    library_location("l", "lib_loc", "cyth library location", true, "", "string", parser ),
    output_location("", "out_loc", "executable output location", false, "", "string", parser ),
    working_location("", "inter_loc", "intermediate output location", false, "", "string", parser ),
    parserReporting("p","parserReporting","Turn on parser reporting for debuging. Default false.", parser, false)
{
    parser.parse(argc, argv);
}

vector< string > command_line_options::get_input_files()
{
    return input_files.getValue();
}

string command_line_options::get_library_location()
{
    auto val = library_location.getValue();
    fs::path p(val);
    return fs::absolute(p);
}

bool command_line_options::has_output_location()
{
    return output_location.getValue() != "";
}

string command_line_options::get_output_location()
{
    auto val = output_location.getValue();
    if( val=="" ) {return val; }
    fs::path p(val);
    return fs::absolute(p);
}

bool command_line_options::has_working_location()
{
    return working_location.getValue() != "";
}

string command_line_options::get_working_location()
{
    auto val = working_location.getValue();
    if( val=="" ) {return val; }
    fs::path p(val);
    return fs::absolute(p);
}

bool command_line_options::get_parserReporting()
{
    bool val = parserReporting.getValue();
    return val;
}





 /// some tools///

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

string replace_directory(string fname, string new_directory)
{
    char sep = '/';
    #ifdef _WIN32
     sep = '\\';
    #endif

    string ret;

    size_t i = fname.rfind(sep, fname.length());
    if (i != string::npos)
    {
       ret = fname.substr(i+1, fname.length() - i);
    }
    else
    {
        ret = fname;
    }

    if( new_directory[ new_directory.length()-1 ] == '/' )
    {
        ret = new_directory + ret;
    }
    else
    {
        ret = new_directory + "/" + ret;
    }

    return ret;
}

string get_directory(string fname)
{
    char sep = '/';
    #ifdef _WIN32
     sep = '\\';
    #endif

    const size_t last_slash_idx = fname.rfind(sep);
    if (std::string::npos != last_slash_idx)
    {
        return fname.substr(0, last_slash_idx);
    }
    return "";
}

class gather_imports : public AST_visitorTraveler
{
public:

    list< string > imported_C_files;
    list< string > imported_cyth_module_fnames;
    module_manager* manager;
    module_AST_ptr module;

    gather_imports( module_manager* _manager, module_AST_ptr _module)
    {
        manager = _manager;
        module = _module;
    }

    void cImports_down(import_C_AST_node* ASTnode)
    {
        if(ASTnode->import_file_name.to_cpp_string() != "" )
        {
            ASTnode->found_file_location = manager->locate_C_file(ASTnode->import_file_name.to_cpp_string(), module);

            if( ASTnode->found_file_location == "" )
            {
                ASTnode->verification_state = 0;
                cout << "cannot find c file: " << ASTnode->import_file_name << " imported at " << ASTnode->loc << endl;
            }

            imported_C_files.push_back( ASTnode->found_file_location );
        }
    }

    void CythImports_down(import_cyth_AST_node* ASTnode)
    {
        ASTnode->imported_module_fname = manager->locate_cyth_file(ASTnode->import_module_string.to_cpp_string(), module);


        if( ASTnode->imported_module_fname == "" )
        {
            ASTnode->verification_state = 0;
            cout << "cannot find cyth module: " << ASTnode->import_module_string << " imported at " << ASTnode->loc << endl;
        }

        imported_cyth_module_fnames.push_back( ASTnode->imported_module_fname.to_cpp_string() );
    }
};







 /// module manager ///
module_manager::module_manager(cmmnd_line_ptr _cmmd_line_optns, bool do_parserTable_IO) :
    cyth_parser_generator(do_parserTable_IO)
{
    cmmd_line_optns = _cmmd_line_optns;
    library_location = cmmd_line_optns->get_library_location();
    c_interface = make_shared<GCC_compiler_interface>();
}

void module_manager::add_module_fname_to_parse(std::string module_fname)
{
    string fname = fs::canonical(fs::path( module_fname ));
    module_fnames_to_parse.push_back( fname );
}


module_AST_ptr module_manager::parse_module(string canonical_module_fname, bool reporting)
{

    auto found_module = parsed_modules.find( canonical_module_fname );
    if( found_module != parsed_modules.end() )
    {   return found_module->second; }

    cout << "parsing file " << canonical_module_fname << endl;

    shared_ptr<parser> cyth_parser =  cyth_parser_generator.get_parser();

    //ifstream input_file;
    //input_file.open(module_fname);
    //cyth_parser->reset_input(input_file);
    //utf8_string fname(module_fname);
    ifstream fin( canonical_module_fname );
    utf8_string UTF8_fname = canonical_module_fname; // this is kinda dum
    cyth_parser->reset_input(fin, UTF8_fname);

    dyn_holder module_output = cyth_parser->parse(reporting);
    module_AST_ptr module_data = *module_output.cast<module_AST_ptr>();





    // set names
    module_data->module_name = fname_to_moduleName( canonical_module_fname );
    module_data->file_name = canonical_module_fname;

    /// inject default langauge features///
    // first check this isn't a default file (if is in lib/cyth/lang)
    auto mod_dir = get_directory(module_data->file_name);
    string libCythLang_directory =  fs::canonical( fs::path( library_location+"/cyth/lang" ) );
    if( mod_dir != libCythLang_directory )
    {
        location_span tmpLoc;
        for( auto &lang_imports : default_language_imports)
        {
            auto &cyth_import_loc = lang_imports.first;
            for( auto &import_name : lang_imports.second  )
            {
                auto import_AST = make_shared<import_cyth_AST_node>( cyth_import_loc, import_name, tmpLoc );
                module_data->add_AST_node( import_AST );
            }
        }
    }





    // gather and process imports
    gather_imports gatherer( this, module_data ); // note this finds all C import file names and makes them absolute. (should also be done for cyth files too)
    module_data->apply_visitor(&gatherer);

    module_data->imported_C_files = gatherer.imported_C_files;
   //module_data->imported_cyth_modules = gatherer.imported_cyth_modules;

    parsed_modules.insert( make_pair(canonical_module_fname, module_data) );

    // place imported cyth files
    for( const string &cyth_mod_fname : gatherer.imported_cyth_module_fnames ) // these are returned by locate_cyth_file, thus are canonical absolute
    {
        module_data->imported_cyth_modules.push_back( cyth_mod_fname );

        if( parsed_modules.find(cyth_mod_fname) == parsed_modules.end() )
        {
            // has not been parsed
            // check if it will be parsed

            bool in_ToParse = false;
            for( auto &module_fname_to_parse : module_fnames_to_parse)
            {
                if( module_fname_to_parse == cyth_mod_fname )
                {
                    in_ToParse = true;
                    break;
                }
            }

            if( not in_ToParse )
            {
                // not going to be parsed, so must be added to the list
                module_fnames_to_parse.push_back( cyth_mod_fname );
            }
        }
    }

    return module_data;
}

void module_manager::parse_all_modules( bool reporting )
{
    reporting = reporting or (cmmd_line_optns->get_parserReporting());

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
            for(auto &dependancy_module_fname : (*iter)->imported_cyth_modules)
            {

                bool this_dependancy_found = false;
                for( auto resolved_module : dependancy_ordered_modules)
                {
                    if( resolved_module->file_name == dependancy_module_fname )
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


module_AST_ptr module_manager::get_module( string canonical_module_fname )
{
    //string module_name = fname_to_moduleName( _module_name );

    auto found_module = parsed_modules.find(canonical_module_fname);
    if( found_module != parsed_modules.end() )
    {   return found_module->second; }
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



// SET C FILE NAMES!
    string base_name = module->file_name;
    if( cmmd_line_optns->has_working_location() )
    {
        auto new_loc = cmmd_line_optns->get_working_location();
        base_name = replace_directory(base_name, new_loc);
    }
    module->C_header_fname = base_name + ".h";
    module->C_source_fname = base_name + ".c";



    cout << "  writing to C" << endl;
    write_module_to_C(module);
    if( module->main_status == 1 )// write main function?
    {
        return write_mainFunc_to_C(module, this);
    }
    return true;

//
//    cout << "  compiling" << endl;
//    // eventually this shoudl be its own function/class
//    string object_fname = module->C_source_fname + ".o";
//    string command = "gcc " + module->C_source_fname + " -o " + object_fname + " -c";
//    int tmp = system( command.c_str() );
//    bool obj_file_exits =  fs::exists( object_fname ) and tmp==0;
//    if( obj_file_exits and (module->main_status == 1)  )
//    {
//        string main_object_fname = module->C_mainSource_fname + ".o";
//        command = "gcc " + module->C_mainSource_fname + " -o " + main_object_fname + " -c";
//        tmp = system( command.c_str() );
//        return fs::exists( main_object_fname ) and tmp==0;
//    }
//    else
//    {
//        return obj_file_exits;
//    }
}

bool module_manager::compile_recursively(module_AST_ptr module)
{
    // first check if children need compiled
    for(auto &child_module_name : module->imported_cyth_modules )
    {
        auto child_module = get_module( child_module_name );
        if( child_module->C_object_fname.length()==0 )
        {
            if( not compile_recursively(child_module) )
            { return false; }
        }

        module->object_fnames_to_link.push_back( child_module->C_object_fname );
        for(auto &new_obj_name : child_module->object_fnames_to_link)
        {
            module->object_fnames_to_link.push_back( new_obj_name );
        }
    }

    // next take care of some compiling commands
    for(auto &compile_cmd : module->compiler_comands )
    {
        string &full_cmd = compile_cmd.command;
        size_t first_space = full_cmd.find(' ');
        auto cmd_name = full_cmd.substr(3, first_space-3);

        if( cmd_name=="link" )
        {
            string object_file_name = full_cmd.substr(first_space+1);
            string found_object_filename = locate_C_file(object_file_name, module);

            if( found_object_filename=="" )
            {
                cout << "cannot find object file: " <<  object_file_name << endl;
                return false;
            }

            module->object_fnames_to_link.push_back( object_file_name );
        }
        else if( cmd_name=="command" )
        {
            string command = full_cmd.substr(first_space+1);
            int tmp = system( command.c_str() );
            if( tmp != 0)
            {
                cout << "command " << command << " on line " << compile_cmd.loc << " was unsucssesful. Returned: " << tmp << endl;
                return false;
            }
        }
        else if( cmd_name=="compileNlink" )
        {
            string c_source_file = full_cmd.substr(first_space+1);
            string found_source_file = locate_C_file(c_source_file, module);

            if( found_source_file=="" )
            {
                cout << "cannot find C-source file: " <<  c_source_file << endl;
                return false;
            }

            string c_obj_file = found_source_file + ".o";
            if( cmmd_line_optns->has_working_location() )
            {
                auto new_loc = cmmd_line_optns->get_working_location();
                c_obj_file = replace_directory(c_obj_file, new_loc);
            }
            // what to do in case of else?

            bool succesful = c_interface->compile_Csource(found_source_file, c_obj_file  );
            if( not succesful)
            {
                cout << "could not compile " << c_source_file << " from line " << compile_cmd.loc << endl;
                return false;
            }
            module->object_fnames_to_link.push_back( c_obj_file );
        }


//        if( cmd_name=="link" )
//        {
//            size_t second_space = full_cmd.find(' ', first_space+1);
//            string object_file_name = full_cmd.substr(first_space+1, second_space-first_space-1);
//            string object_run_cmd = full_cmd.substr(second_space+1);
//
//            int ignored __attribute__((unused));
//            ignored = system( object_run_cmd.c_str() );
//
//            object_files_to_include.push_back( object_file_name );
//        }
    }

    // now we try to compile the file itself
    string c_obj_file = module->C_source_fname + ".o";
    bool succesful = c_interface->compile_Csource(module->C_source_fname, c_obj_file  );
    if(not  succesful)
    {
        cout << "could not compile module " << module->module_name << endl;
        return false;
    }

    module->C_object_fname = c_obj_file;
    return true;
}

bool module_manager::compile_and_link()
{
    cout << "compiling" << endl;
    for (auto const& name_module_pair : parsed_modules)
    {
    auto module = name_module_pair.second;
    if( module->main_status == 1)
    {
        // compile module
        bool succesful = compile_recursively(module);
        if( not succesful)
        {
            return false;
        }

        // compile main source file
        string main_object_fname = module->C_mainSource_fname + ".o";
        succesful = c_interface->compile_Csource(module->C_mainSource_fname, main_object_fname  );

        if( not succesful)
        {
            cout << "could not compile " << module->module_name << " main function" << endl;
            return false;
        }

        // link!
        list< string > object_files_to_include;
        object_files_to_include.push_back( main_object_fname );
        object_files_to_include.push_back( module->C_object_fname );
        for( auto &obj_name : module->object_fnames_to_link )
        {
            // need to make sure we don't link in same think twice
            bool file_included = false;
            for(auto &included_obj : object_files_to_include)
            {
                if( included_obj==obj_name )
                {
                    file_included = true;
                    break;
                }
            }

            if( not file_included )
            {
                object_files_to_include.push_back( obj_name );
            }
        }

        string out_name = module->file_name + ".out";
        if( cmmd_line_optns->has_output_location() )
        {
            auto new_loc = cmmd_line_optns->get_output_location();
            out_name = replace_directory(out_name, new_loc  );
        }


        succesful = c_interface->link(out_name, object_files_to_include  );
        if(not succesful )
        {
            cout << "could not link " << module->module_name << " main function" << endl;
            return false;
        }

//
//        list< string > object_files_to_include;
//        list< module_AST_ptr > modules_to_include;
//
//        object_files_to_include.push_back( module->C_mainSource_fname + ".o" );
//        modules_to_include.push_back( module );
//
//        for( auto new_module : modules_to_include )
//        {
//            object_files_to_include.push_back( new_module->C_source_fname + ".o" );
//
//            for(auto &import_cyth_name : new_module->imported_cyth_modules)
//            {
//                modules_to_include.push_back( get_module( import_cyth_name ) );
//            }
//
//            for(auto &command : new_module->compiler_comands )
//            {
//                string &full_cmd = command.command;
//                size_t first_space = full_cmd.find(' ');
//                auto cmd_name = full_cmd.substr(3, first_space-3);
//                if( cmd_name=="link" )
//                {
//                    size_t second_space = full_cmd.find(' ', first_space+1);
//                    string object_file_name = full_cmd.substr(first_space+1, second_space-first_space-1);
//                    string object_run_cmd = full_cmd.substr(second_space+1);
//
//                    int ignored __attribute__((unused));
//                    ignored = system( object_run_cmd.c_str() );
//
//                    object_files_to_include.push_back( object_file_name );
//
//                }
//            }
//        }
//
//
//        string link_command = "gcc -o " + module->file_name + ".out";
//        for( string &obj_fname :  object_files_to_include)
//        {
//            link_command += " " + obj_fname;
//        }
//
//        int ignored __attribute__((unused));
//        ignored = system( link_command.c_str() );
    }
    }

    return true;
}


string module_manager::locate_C_file(string c_file_name, module_AST_ptr importing_module)
{

    // check absolute location
    auto tst_path = fs::absolute( fs::path(c_file_name));
    if( fs::exists( tst_path ) )
    {
        return tst_path;
    }

    // test in system C-library
    // ??????

    // first check C-header cyth lib
    tst_path = fs::absolute( fs::path( library_location + "/c_header/" + c_file_name ));
    if( fs::exists( tst_path ) )
    {
        return tst_path;
    }
    // or C-source cyth lib
    tst_path = fs::absolute( fs::path( library_location + "/c_source/" + c_file_name ));
    if( fs::exists( tst_path ) )
    {
        return tst_path;
    }

    // tst at location of module
    tst_path = fs::absolute( fs::path( get_directory(importing_module->file_name)+'/'+c_file_name ));
    if( fs::exists( tst_path ) )
    {
        return tst_path;
    }

    // cannot find!!!
    return "";
}

string module_manager::locate_cyth_file(string module_import_string, module_AST_ptr importing_module)
{
    module_import_string += ".cy";

    // check cyth lib
    auto tst_path = fs::canonical( fs::path( library_location+"/cyth/"+module_import_string));
    if( fs::exists( tst_path ) )
    {
        return tst_path;
    }

    // tst at location of module
    tst_path = fs::canonical( fs::path( get_directory(importing_module->file_name)+'/'+module_import_string ));
    if( fs::exists( tst_path ) )
    {
        return tst_path;
    }

    // finally test absolute location
    tst_path = fs::canonical( fs::path(module_import_string));
    if( fs::exists( tst_path ) )
    {
        return tst_path;
    }

    // cannot find!!!
    return "";
}






