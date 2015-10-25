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
#include <functional>
#include <initializer_list>
#include <memory>
#include <vector>
#include <list>

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

template<typename return_type>
class lexing_functor
//a class to overload in order to return data from lexer. If not subclassed, it will just return an empty return_type
//nice thing, is that this is just one pointer! nice and light
{
public:
    virtual return_type operator()(utf8_string& data, location_span& loc )
    {
        return return_type();
    }
};

template<typename return_type>
class stl_functional : lexing_functor< return_type>
//a subclass of lexing_functor, that can use the stl_functional type
{
private:
    std::function<return_type (utf8_string&, location_span&)> func;

public:

    stl_functional(std::function<return_type (utf8_string&, location_span&)> _func)
    {
        func=_func;
    }

    virtual return_type operator()(utf8_string& data, location_span& loc )
    {
        return func(data, loc);
    }
};

template<typename return_type>
class lexer
// a class to lex files
{
private:
    lexing_functor<return_type> EOF_action; 
    std::shared_ptr< std::vector< std::shared_ptr<DFA_state> > > state_table;
    std::shared_ptr< std::vector< lexing_functor<return_type> > > actions; 
    std::shared_ptr< std::vector< unsigned int> > lexer_states;

public:
    lexer(lexing_functor<return_type> _EOF_action, std::shared_ptr< std::vector< std::shared_ptr<DFA_state> > > _state_table, 
                std::shared_ptr< std::vector< lexing_functor<return_type> > > _actions, std::shared_ptr< std::vector< unsigned int> > _lexer_states)
    {
        EOF_action=_EOF_action;
        actions=_actions;
        lexer_states=_lexer_states;
        //I AM HERE
        //making lexer
    }
}

template<typename return_type >
class lexer_generator
//a class that will create a lexer for us.
{
private:
    
    struct pattern
    {
        utf8_string regular_expression;
        unsigned int action_number;
        unsigned int state;
    };
    
    unsigned int current_state;
    std::list< pattern > patterns;
    
    utf8_string state_table_file_name; //used to save the state_table to a file. 
    //Usefull if the program is ran multiple times (likely)
    
    bool made_table;
    //lexer table information
    std::shared_ptr< std::vector< std::shared_ptr<DFA_state> > > state_table;
    std::shared_ptr< std::vector< lexing_functor<return_type> > > actions;
    std::shared_ptr< std::vector< unsigned int> > lexer_states; //a vector that contains whicch state in state_table is the starting point for a given lexer-state

    lexing_functor<return_type> EOF_action;

public:
 
    lexer_generator(utf8_string _state_table_file_name)
    {
        state_table_file_name=_state_table_file_name;
        current_state=0;
        made_table=false;
        actions=new vector< lexing_functor<return_type> >();
    }
    
    unsigned int increment_state()
    {
        current_state+=1;
        return current_state;
    }
    
    void set_EOF_action( lexing_functor<return_type> _action)
    {
        EOF_action=_action;
    }
    
    void add_pattern(utf8_string _regular_exp, lexing_functor<return_type> _action)
    //add a regex pattern, and an associated action to take.
    {
        pattern new_pattern;
        new_pattern.regular_expression=_regular_exp;
        new_pattern.action_number=actions.size();
        new_pattern.state=current_state;
        patterns.push_back(new_pattern);
        actions->push_back(_action);
    }
    
    void add_pattern(utf8_string _regular_exp, std::function<return_type (utf8_string&, location_span&)> _action)
    //add a regex pattern, and an associated action to take. allow use of C++ stl functionals
    {
        pattern new_pattern;
        new_pattern.regular_expression=_regular_exp;
        new_pattern.state=current_state;
        new_pattern.action_number=actions.size();
        patterns.push_back(new_pattern);
        actions->push_back( stl_functional<return_type>(_action) );
    }
    
    void add_multi_patterns(std::initializer_list<utf8_string> reg_expressions, lexing_functor<return_type> _action)
    {
        unsigned int action_number=actions.size();
        actions->push_back(_action);
        for(const utf8_string& reg_ex : reg_expressions)
        {
            pattern new_pattern;
            new_pattern.regular_expression=reg_ex;
            new_pattern.action_number=action_number;
            new_pattern.state=current_state;
            patterns.push_back(new_pattern);
        }
    }
    
    void add_multi_patterns(std::initializer_list<utf8_string> reg_expressions,  std::function<return_type (utf8_string&, location_span&)> _action)
    {
        stl_functional<return_type> packed_action(_action);
        unsigned int action_number=actions.size();
        actions->push_back(packed_action);
        
        for(const utf8_string& reg_ex : reg_expressions)
        {
            pattern new_pattern;
            new_pattern.regular_expression=reg_ex;
            new_pattern.action=packed_action;
            new_pattern.action_number=action_number;
            patterns.push_back(new_pattern);
        }
    }
    
    lexer<return_type> get_lexer()
    {
        if( not made_table)
        {
            //need to check if the file exists
            //if so, open the file
            //else, generate_state_tables and save to the file
            
            generate_state_tables();
        }
        
        return lexer<return_type>(EOF_action, state_table, actions, lexer_states);
    }
    
private:
    
    void generate_state_tables()
    {
        std::list< std::shared_ptr<DFA_state> > current_state_table;
        std::list< unsigned int > current_lexer_states;
        
        int lexer_state=0;
        auto pattern_iter=patterns.begin();
        while(lexer_state<=current_state) //loop over each potential state
        {
            std::list< std::shared_ptr<NFA_state> > NFA_of_lexerstate;
            std::shared_ptr<NFA_state> first_state(new NFA_state);
            NFA_of_lexerstate.push_back(first_state);
            
            for(  ; pattern_iter->state==lexer_state; ++pattern_iter) //loop over each patern that is in the present lexer_state
            {
                int chars_counted=0;
                auto regex_tree=parse_regex(pattern_iter->regular_expression, chars_counted);
                if( chars_counted< pattern_iter->regular_expression.get_length() )
                    throw gen_exception("full regex cannot be parsed");
                if( not regex_tree)
                    throw gen_exception("could not parse regex");
                    
                //get new states, make end state as accepting
                std::list< std::shared_ptr<NFA_state> > new_states=regex_tree->get_NFA();
                (*(++new_states.begin()))->accepting_info=pattern_iter->action_number;
                 
                 //add to total NFA
                increment_states(new_states, NFA_of_lexerstate.size());
                first_state->add_transition(NFA_state::epsilon, NFA_of_lexerstate.size());
                NFA_of_lexerstate.insert(NFA_of_lexerstate.end(), new_states.begin(), new_states.end());
            }
            
            std::list< std::shared_ptr<DFA_state> > DFA_states=NFA_to_DFA(NFA_of_lexerstate);
            DFA_states=DFA_minimization(DFA_states);
            
            
            current_lexer_states.push_back(current_state_table.size());
            current_state_table.insert(current_state_table.end(), DFA_states.begin(), DFA_states.end() );
            
            lexer_state++;
        }
        
        state_table=new vector< std::shared_ptr<DFA_state> >(current_state_table);
        lexer_states=new vector< unsigned int >(current_lexer_states);
        made_table=true;
    }
};


} //end csu namespace


#endif
