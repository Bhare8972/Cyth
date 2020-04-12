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
#include "writeAST_to_C.hpp"

using namespace std;


//ADDING CLASSES
// then methods
//    now implementing self. First need to add in C-pointer ref type
//      func def now has self
//      need to add to function calling
//      first fix function_call_writer for default, then fix overloads
//      finally fix source_expression_visitor::functionCall_Exp_up to do the write thing


// fix lexor to use quotations. Fix for strings

// improve C variable naming
// fix csu::dynamic cast exception
// loc needs name of file



// add classes (include conversion operators)!
// exceptions?
// add binary, unary, and in-place operators (including numerics and bool)

// add int, long, float, double, and bool

// add imports from C and cyth (presently it just defines a name, and doesn't actually import anything!)
     // add build system
     // add cyth headers

// if, while and for loops

// add accses and iterator operators, and slice built-in
// add string
// add var?
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
        cout<<"opening: "<<fname<<endl;
        auto new_module = cyth_module_manager.parse_module(fname, false);

        if(new_module)
        {
            cout<< "writing module: '" << fname << "' to cpp." << endl;
            write_module_to_C(new_module, fname);
        }
        else
        {
        }
    }


    cout<<"DONE!"<<endl;
}
