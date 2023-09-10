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

This is the entry point for the Cyth compiler
*/

#include <string>
#include "module_manager.hpp"

using namespace std;


//PROBLEM. what happens in writeAST_to_C.cpp ::  function_argument_acclimator  if you have nested functiopns?


/// some rules:
//    constructors are never virtual or inherited
//    assignment operators are virtual and inhereted, but this is dangerous!!
//    cast_to operators are normal (can be inhereted and virtual).

//    when calling, implicit-methods (constructors, etc) are preferred instead of explicit methods
//          at the moment this is checked by the callee. So the caller should just use the explicit version, and the callee will default to implicit if it is available
//    cast_to operator should be called instead of, in same place as, copy constructions

//    when converting from one type to anouther, first check copy constructor, then check cast_to. I.E. copy constructor is prefered over cast_to operator.
//    every time the compiler does a cast, the next allowable cast type must go "down" by one. I.E. explicit casting methods should only call implicit casting methods
//          and implicit casting methods should only call pointer casting (I THINK???)

// assignment operators are automatically generated for each implicit copy constructor (should probably include explicit as well!!!)





/// GENERAL FIXES ///

// SOMEHOW need to check to make sure previous lexar/parser tabels are valid with current rules.

// fix lexor to use quotations. Fix for strings

// improve C variable naming
// fix csu::dynamic cast exception
// DefClassType::initialize is wasteful
// is DefClassType::write_member_setter correct???? where is it even used?
     // what are the ruless for castTo? does is destruct first, can it be used in-place of assignment???
     // destruction with assignment is a total mess.


// when printing grammer rules, print in order of number...

// TODO: defaulted params should use explicit or implicit copy constructors??? should they use conversion operators?

// imports

    // improve module-based compiler commands
    // import from different locations (need file system manager)
    // import module namespaces

    // make so compileNlink compiles a bit more intellegently

// add self var to types , then implement auto  ( I no longer remember what this means)

// need to add self weak_pointer to AST nodes.
// slowley fix symbol table types to rely on the self var
// fix addition and function call write to C, not to be referancable ( I think I did this somewhat?)
    //   need to check that expression cleanup is done correctly, do this AFTER inform_moved
    //   e.g. does source_expression_visitor::accessorExp_up do the right thing?

// IDEA: make c-void pointers a built-in type, and a macro to get cptr!!! (maybe via implicit casting?)

// raw_C_pointer_reference explicit_castTo not quite right, needs to also call explicit constructor I think?

// handle main function return codes


// NOTE: langauge has problem in that send-by-ref function calls can always "cast" so it is not obvious if call is truly by-reference or not
//    May not be a problem.   NOTE FROM FUTURE: this is fixed. static variables are, var variables are not. This may not currently be inforced!!

// function argument type in sym_table may need location?

// try to fix function and method types to not include symbol name?


// TODO: DefClassType::get_pointer handles C_expression wrong!! probablyu fine for now

//should inform_moved be recursive?

// need to add support for __initialize__ and __deinitialize__ functions in modules. Also need to call destructors on global memory after __deinitialize__

// todo: functions like: write_parent_access, parental_write_call, and get_member_full_inheritance do not check if exp is referencable.  Is this a problem? How to fix it?

// methodDef_up at 1548 need local destructor??
// funcDef_up same?

//   add compiler command to force build parser (even if no files to build), to read from file if avaialbe (default), or force read from file error if no file. Perhaps just make rebuild parser if tables older than compiled parser?

// place controls by boolean passed-by-value to DOWN vistors to control if certain children are visited or not

// lexer line spacing testing should give error msg on tabs

// function definition loc_spans only include header, should include block

// printing loc_span should only print fname once

// self-pointer should actually be a struct that includes self-pointer, as well as a reference-block (like a shared-ptr that exists to say if memory is alive or not). This reference block is made when the memory-controlled variable is made (static, or var-variable is malloced)
    //  this reference block is passed to all members of a class. Thus, everyone can have a shared-ptr that says if they are alive or not, essentially allowing safty-references.


/// current work plan ///

//WORKING on And/Or , should work, needs testing!



 // do later:
 // unary, and in-place operators
 // float, double
 // __call__ operator




// add accses and iterator operators, and slice built-in
// void_ptr?
// add string


// exceptions
// add var
// add concepts
// add iterators and iter-loops
// add container build-ins (list, array, dict)
// add var references (copy to dynamic variable when goes out of scope?)
// add nested functions and classes
    // requires closure. Perhaps when a closured static variable goes out-of-scope, it is copied to a dynamic variable?
// metaprograming
    // macros
    // decorators

// funtion dectorators
//@nonOverridable (spelling)

// special class types. unique, small, etc....




// SOME DOCS:
// compiler commnads:
// !!!link <object file>....
// !!!command <system command....>  (NOTE: command should return 0 on succsesful exit)
// !!!compileNlink <C_source_file....>


int main(int argc, char *argv[])
{
    cmmnd_line_ptr commands;

    try
    {
        commands = make_shared<command_line_options>(argc, argv);
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
	{
	    cout << "error: " << e.error() << " for arg " << e.argId() << endl;
	    return 1;
    }

    module_manager cyth_module_manager(commands, true);


    for( auto& module_fname : commands->get_input_files() )
    {
        cyth_module_manager.add_module_fname_to_parse( module_fname );
    }

    cyth_module_manager.parse_all_modules( );
    auto sorted_modules = cyth_module_manager.get_module_dependency_order();

    for(auto module : sorted_modules)
    {
        if( not cyth_module_manager.build_module( module ) )
        {
            cout << "build unsuccessful. CLOSING!" << endl;
            return 1;
        }
    }



    if( not cyth_module_manager.compile_and_link() )
    {
        cout << "compiling or linking unsuccessful" << endl;
        return 1;
    }



    cout<<"DONE!"<<endl;
    return 0;
}
