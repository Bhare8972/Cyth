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

#include "parser.hpp"

using namespace csu;
using namespace std;

//functional return wrapper
//for converting a function of one return type to a function of a different return type
//assume that new return type can be constructed from old return type

//may not need this code
//template<typename return_T, typename...args_T>
//class return_wrapper
//{
//public:
    //virtual return_T operator()(args_T...args)
//};
//
//template<typename new_return_T typename old_return_T, typename...args_T>
//class return_wrapper_specific : public return_wrapper<new_return_T, args_T...>
//{
//private:
    //std::function<old_return_T(args_T...)> func;
//public:
    //return_wrapper_specific(std::function<old_return_T(args_T...)> _func)
    //{
        //func=_func;
    //}
//
    //virtual return_T operator()(args_T...args)
    //{
        //return return_T( func(args...) );
    //}
//};


//begin token class
token::next_token_ID=0;
token::set_token_ID(token* _toke)
{
    _toke->token_ID=next_token_ID;
    next_token_ID++;
}

token::token(utf8_string _name, bool _is_terminal)
{
    name=_name;
    is_terminal=_is_terminal;
    set_token_ID(this);
}

token::token(utf8_string _name, bool _is_terminal)
{
    name="unnamed";
    is_terminal=_is_terminal;
    set_token_ID(this);
}

ostream& csu::operator<<(ostream& os, const token& dt)
{
    os<<dt.name;
    return os;
}
//end token class

//terminal
terminal::terminal(utf8_string _name, parser_generator* _par_gen)
{
    name=_name;
    is_terminal=true;
    set_token_ID(this);
    par_gen=_par_gen;
}

void add_pattern(utf8_string pattern, bool return_data=true)
{
    throw gen_exception("not implemented");
}

template<typename ret_type>
void add_pattern(utf8_string pattern, function<ret_type(utf8_string, lexer*)> _func)
{
    throw gen_exception("not implemented");
}

template<typename ret_type>
void add_patterns(initializer_list<utf8_string> patterns, function<ret_type(utf8_string, lexer*)> _func)
{
    throw gen_exception("not implemented");
}
//end terminal class

//non_terminal
class non_terminal : public token
{
private:
    parser_generator* par_gen;
public:
    non_terminal(utf8_string _name, _par_gen);
    
    production& add_production(std::initializer_list<token*> tokens);
};

non_terminal::non_terminal(utf8_string _name, _par_gen)
{
    name=_name;
    par_gen=_par_gen;
    is_terminal=false;
    set_token_ID(this);
}

production& add_production(std::initializer_list<token*> tokens)
{
    throw gen_exception("not implemented");
}
//end non_terminal class

// production
production::next_production_ID=0;
production::production(non_terminal* _L_val, std::vector<token*>& _tokens)
{
    L_val=_L_val;
    tokens=_tokens;
    production_ID=next_production_ID;
    next_production_ID++;
    _assoc=NONE;
}

template<typename ret_type>
production& production::action(std::function<ret_type(std::vector<token_data>&)> _func)
{
    throw gen_exception("not implemented");
}

production& production::assoc(association _assoc)
{
    assoc=_assoc;
}

production_info production::get_info()
{
    throw gen_exception("not implemented");
}
//end production class
