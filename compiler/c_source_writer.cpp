
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


This file defines a wrapper over file stream that facilitates writing C-code
*/


#include "c_source_writer.hpp"

using namespace csu;
using namespace std;



output_Csource_file::output_Csource_file(string _output_fname, int _number_spaces_per_line) :
    out_file(_output_fname),
    output_fname( _output_fname )
{
    next_variable_ID = 0;
    //ln_strt.parent = shared_from_this();
    number_spaces_per_line = _number_spaces_per_line;
}


output_Csource_file::output_Csource_file(string _output_fname) :
    out_file(_output_fname),
    output_fname( _output_fname )
{
    next_variable_ID = 0;
    //ln_strt.parent = shared_from_this();
    number_spaces_per_line = 2;
}


ostream& output_Csource_file::out_strm()
{
    return out_file;
}

void output_Csource_file::enter_scope()
{
    if( ln_strt.parent.expired() )
    {
        ln_strt.parent = shared_from_this();
    }

    variable_ID_stack.push_back(next_variable_ID);
    next_variable_ID = 0;
}

void output_Csource_file::leave_scope()
{
    if(variable_ID_stack.size()>0)
    {
        next_variable_ID = variable_ID_stack.back();
        variable_ID_stack.pop_back();
    }
}

utf8_string output_Csource_file::get_unique_string()
{
    utf8_string out_string = to_string(next_variable_ID);
    next_variable_ID += 1;
    return "__cytmp__" + out_string;
}



ostream& operator<<(ostream& os, const output_Csource_file::line_space_writer& dt)
{
    if( not dt.parent.expired() )
    {
        Csource_out_ptr Cfile_obj = dt.parent.lock();
        for(int i=0; i < Cfile_obj->variable_ID_stack.size(); i++)
        {
            for(int j=0; j<Cfile_obj->number_spaces_per_line; j++)
            {
                os<<' ';
            }
        }
    }

    return os;
}

Csource_out_ptr open_C_writer(string output_fname)
{
    return make_shared<output_Csource_file>(output_fname, 2);
}
