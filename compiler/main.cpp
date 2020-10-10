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






//some rules:
//    constructors are never virtual or inherited
//    assignment operators are virtual and inhereted, but this is dangerous!!
//    cast_to operators are normal (can be inhereted and virtual).

//    when calling, implicit-methods (constructors, etc) are preferred instead of explicit methods
//          at the moment this is checked by the callee. So the caller should just use the explicit version, and the callee will default to implicit if it is available
//    cast_to operator should be called instead of, in same place as, copy constructions




//// GENERAL FIXES /////
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

// add self var to types , then implement auto  ( I no longer remember what this means)

// need to add self weak_pointer to AST nodes.
// slowley fix symbol table types to rely on the self var
// fix addition and function call write to C, not to be referancable ( I think I did this somewhat?)
    //   need to check that expression cleanup is done correctly, do this AFTER inform_moved
    //   e.g. does source_expression_visitor::accessorExp_up do the right thing?

// IDEA: make c-void pointers a built-in type, and a macro to get cptr!!! (maybe via implicit casting?)






// am working on classes

// assign needs to be able to be virtual!
//need to fix assinment opperators, maybe make a default assign that is defined like now (destructor then copy construct, called VIRTuALLY), but is overridden by __assign__

// inform moved

// explicit copy constructors
// class as a conversion function (as a meta-type! ??)

// assign_to operator


// funtion dectorators
//@nonOverridable (spelling)

// exceptions?
// add binary, unary, and in-place operators (including numerics, no booleans operator!)

// __call__


 //literals
// add int, long, float, double, and bool

// need library!!

//boolean operators

// if, while and for loops

// add accses and iterator operators, and slice built-in
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



int main(int argc, char *argv[])
{
    module_manager cyth_module_manager(false);

    if(argc == 2)
    {
        string fname = argv[1];
//        cout<<"opening: "<<fname<<endl;
//        auto new_module = cyth_module_manager.parse_module(fname, false);
//
//        if(new_module)
//        {
//            cout<< "writing module: '" << fname << "' to cpp." << endl;
//            write_module_to_C(new_module, fname);
//        }
//        else
//        {
//        }

        cyth_module_manager.add_module_fname_to_parse( fname );
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
    }
    else
    {
        cout<<"You're a dope."<<endl;
        return 1;
    }

    return 0;
}
