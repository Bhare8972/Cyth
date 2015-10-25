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

this file is a set of utilities and a lexer, designed for lexing text files in UTF-8
*/

#ifndef LEXER_151017032552
#define LEXER_151017032552

#include "UTF8.hpp"
#include "gen_ex.h"
#include "regex.hpp"
#include "logger.hpp"

#include <iostream>
#include <fstream>


namespace csu{ //cyth standard utilities namespace

//basic utility classes

class location
//location class, to track a location in a file
{
    friend std::ostream& operator<<(std::ostream& os, const location& dt);
    public:
    
    int line;
    int column;
    
    location();
    location(const location& RHS);
    //move the location forward according to some string
    
    location& operator=(const location& other) ;
    //assignment
    
    void update(utf8_string& input);
};
std::ostream& operator<<(std::ostream& os, const location& dt);

class location_span
//location_span, to span between two locations in a file
{
    friend location_span operator +(location_span& LHS, location_span& RHS);
    friend std::ostream& operator<<(std::ostream& os, const location_span& dt);
    public:
    
    location start;
    location end;
    
    location_span(const location& _start, const location& _end);
    location_span(const location_span& RHS);
};
location_span operator +(const location_span& LHS, const location_span& RHS);
std::ostream& operator<<(std::ostream& os, const location_span& dt);

class ring_buffer
//ring_buffer class is used to store data in from a file so it can be processed
//implimented like a doubly-linked list
{
public:
    class ring_buffer_node
    //stores a single code point
    {
        public:
        code_point charector;
        ring_buffer_node* next;
        ring_buffer_node* previous;
        
        ring_buffer_node()
        {
            next=NULL;
            previous=NULL;
        }
    };
    
    std::istream fin;
    bool has_read_EOF;
    ring_buffer_node* start_node; //beginning of the loaded and read data
    ring_buffer_node* end_node; //end (not included) of the read data
    ring_buffer_node* empty_node; //end (not included) of the loaded data
    int length_read; //distance from start_node to end_node
    int length_loaded; //distance from start_node to empty_node
    int n_nodes;
    
    ring_buffer(const std::istream& fin_);
    ~ring_buffer();
    
    void add_nodes(int n_nodes_);
    //create extra empty nodes after the empty node
    
    void load_data();
    //load data from the input file
    
    code_point next();
    //return next charector with 'reading' it
    
    code_point read();
    //read the next charector and return it
    
    utf8_string string();
    //return a string representing all read nodes
    
    utf8_string reset_string();
    //set start_node to end_node, and return string that was read
    
    void backup(int N);
    //backup end node N places
    
    void put(utf8_string& data);
    //put data before empty_node
    
    void insert(utf8_string& data);
    //place data before end_node
    
    void reject();
    //reject the read string. back end_node to start_node
};

template<typename return_type >
class lexer_generator
//a class that will create a lexer for use
{
    I AM HERE. MAKING LEXER_GEN
    LEXER TECH WILL HAVE A TEMPLATE, NOT DYNAMIC TYPE
};

} //end csu namespace


#endif
