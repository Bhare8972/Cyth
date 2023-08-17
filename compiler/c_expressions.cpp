/*

Copyright 2020 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file defines C-expressions and how they write to file and cleanup
*/

#include "c_expressions.hpp"
#include "sym_table.hpp"

using namespace std;
using namespace csu;

void C_expression::write_cleanup( Csource_out_ptr source_fout )
{
    for( auto &child : child_expressions)
    {
        child->write_cleanup( source_fout );
    }
    cleanup_this( source_fout );
}

void C_expression::add_cleanup_child( shared_ptr<C_expression> child )
{
    child_expressions.push_back( child );
}



simple_C_expression::simple_C_expression( utf8_string _exp, shared_ptr<varType> _type,
                        bool _can_be_referenced, bool _has_output_ownership ) :
    exp( _exp )
{
    cyth_type = _type;

    has_output_ownership = _has_output_ownership;
    can_be_referenced = _can_be_referenced;
}



owned_name::owned_name(shared_ptr<varType> __cyth_type, csu::utf8_string __name) :
    name( __name )
{
    cyth_type = __cyth_type;
    has_output_ownership = true;
    can_be_referenced = true;
}

void owned_name::cleanup_this( Csource_out_ptr source_fout )
{
    if( has_output_ownership )
    {
        if(not can_be_referenced )
        {
            auto EXP = get_C_expression();
            if( EXP.get_length() >0 )
            {
                cout << "WARNING!: an expression must be destructed but cannot be referenced!" << endl;
                cout << "  exp >" <<  EXP << "<" << endl;
            }
        }

        cyth_type->write_destructor(name, source_fout);
    }
}
