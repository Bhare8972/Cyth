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

//TODO:
// current trying to implement blocks
//  parser is implemented. Need to implement AST, and following stuff. Include fixing function in parrellel

// add functions (include the @-thingies, before and after function name)
    // BLOCK
    //     nested functions
    // parameters
    // returns
    //    function call as expression
    // decorators
    // call C functions
    // __main__
// auto keyword for variable declaration
    // assignment for functions
// add classes (include conversion operators)!
// add binary, unary, and in-place operators (except bool)
// add int, long, float, double built-ins  (methods should be non-virtual)
// add bool operators and bool built-in
// add exceptions..
// add accses and iterator operators, and slice built-in
// add string
// add var?
// add container build-ins (list, array, dict)
// add imports from C and cyth (presently it just defines a name, and doesn't actually import anything!)

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
