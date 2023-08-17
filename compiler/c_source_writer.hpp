
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

#ifndef C_SOURCE_WRITER_201220153600
#define C_SOURCE_WRITER_201220153600

#include <memory>
#include <list>
#include <fstream>

#include "UTF8.hpp"

class output_Csource_file : public std::enable_shared_from_this<output_Csource_file>
{
private:

    unsigned long next_variable_ID;
    std::ofstream out_file;
    std::string output_fname;


public:
    class line_space_writer
    {
    public:
        std::weak_ptr<output_Csource_file> parent;
        friend std::ostream& operator<<(std::ostream& os, const line_space_writer& dt);
    };

    std::list<unsigned long> variable_ID_stack;
    unsigned int number_spaces_per_line;

    output_Csource_file(std::string _output_fname);
    output_Csource_file(std::string _output_fname, int _number_spaces_per_line);

    line_space_writer ln_strt;
    std::ostream& out_strm();

    void enter_scope();
    void leave_scope();

    csu::utf8_string get_unique_string();
};

typedef std::shared_ptr<output_Csource_file> Csource_out_ptr;

std::ostream& operator<<(std::ostream& os, const output_Csource_file::line_space_writer& dt);

Csource_out_ptr open_C_writer(std::string output_fname);

#endif
