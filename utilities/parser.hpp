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

this file is a LALR1 parser,parser_generator, and associated utilities
*/

#ifndef PARSER_151101062514
#define PARSER_151101062514

#include <functional>
#include <initializer_list>
#include <vector>

#include "UTF8.hpp"
#include "gen_ex.h"
#include "lexer.hpp"
#include "type_erasure.hpp"

//utilities for defining languege
namespace csu{ //cyth standard utilities namespace

class token_data; //pre-declare a class that will eventualy hold data about a token
//will hold a token_ID, data, and a location

class parser_generator; //pre-declare the parser-generator

class token
//represents a token in our languege. Is either a terminal or a non-terminal token
{
private:
    friend std::ostream& operator<<(std::ostream& os, const token& dt);
    static unsigned int next_token_ID;
protected:
    static set_token_ID(token* _toke); //so that all tokens will have a unique ID
public:
    utf8_string name;
    bool is_terminal;
    unsigned int token_ID;
    
    token(utf8_string _name, bool _is_terminal); //normal constructor
    token(bool _is_terminal); //name is set to "unnamed"
};

class terminal : public token
//represents a terminal token
{
private:
    parser_generator* par_gen;
public:
    terminal(utf8_string _name, parser_generator* _par_gen);
    
    //functions to add patterns
    void add_pattern(utf8_string pattern, bool return_data=true); //add a patern that either returns, or doesn't. with data as string
    template<typename ret_type>
    void add_pattern(utf8_string pattern, std::function<ret_type(utf8_string, lexer*)> _func); //specific action
    template<typename ret_type>
    void add_patterns(std::initializer_list<utf8_string> patterns, std::function<ret_type(utf8_string, lexer*)> _func); //specific action to multiple patterns
};

class production; //pre-declare the production token

class non_terminal : public token
{
private:
    parser_generator* par_gen;
public:
    non_terminal(utf8_string _name, _par_gen);
    
    production& add_production(std::initializer_list<token*> tokens);
};

class production_info;

class production
//represents a production, that could produce a non-terminal
{
private:
    friend std::ostream& operator<<(std::ostream& os, const production& dt);
    static unsigned int next_production_ID;
public:
   enum association
   {
      NONE,
      LEFT,
      RIGHT
   };

    unsigned int production_ID;
    non_terminal* L_val;
    std::vector<token*> tokens;
    std::function<token_data(std::vector<token_data>)> action;
    association _assoc;
    
    production(non_terminal* _L_val, std::vector<token*>& _tokens);
    
    template<typename ret_type>
    production& action(std::function<ret_type(std::vector<token_data>&)> _func);
    
    production& assoc(association _assoc);
    
    production_info get_info();
};

//tools used by the generator in making the parser table
class token_data
//holds data passed around by the parser
{
    unsigned int token_ID;
    gen_holder token_data;
    location_span span;
public:
    token_data(_ID, _data, _span);
    
    template<typename ret_T>
    ret_T data();
    
    template<typename ret_T>
    void data(ret_T& ret_data);
    
    location_span loc();
    
    unsigned int get_ID();
};

class production_info
//contains data about a production. Used by parser for reporting info
{
    friend std::ostream& operator<<(std::ostream& os, const production_info& dt);
public:
    unsigned int L_val_ID;
    utf8_string L_val_name;
    unsigned int num_tokens;
    std::function<token_data(std::vector<token_data>)> action;
    
    production_info(unsigned int _L_val_ID, utf8_string _L_val_name, unsigned int _num_tokens, std::function<token_data(std::vector<token_data>)> _action);
};

//classes used in processing the gramer
class item;
class item_set;
class propagation_table;

//classes to represent states in parser
class parser_action
{
    friend std::ostream& operator<<(std::ostream& os, const parser_action& dt);
private:
    enum action_type
    {
        ERROR,
        ACCEPT,
        SHIFT,
        REDUCE
    };
    
    action_type action_todo;
    unsigned int data;
    
    parser_action(action_type _todo, unsigned int _data);
    
public:
    static parser_action get_error();
    static parser_action get_accept();
    static parser_action get_shift(unsigned int state);
    static parser_action get_reduce(unsigned int production_ID);
    
    bool is_error();
    bool is_accept();
    bool is_shift();
    bool is_reduce();
    
    unsigned int get_data();
    
    bool operator==(parser_action& RHS);
};

class parser_state
{
    
public:
    I AM HERE
};

}//end csu namespace
#endif
