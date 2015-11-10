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
Much of this is based off of the dragon book.
*/

#ifndef PARSER_151101062514
#define PARSER_151101062514

#include <functional>
#include <initializer_list>
#include <vector>
#include <map>
#include <utility>

#include "UTF8.hpp"
#include "gen_ex.h"
#include "lexer.hpp"
#include "type_erasure.hpp"

//utilities for defining languege
namespace csu{ //cyth standard utilities namespace

///// foward declarations /////

class token_data; //pre-declare a class that will eventualy hold data about a token
//will hold a token_ID, data, and a location

class parser_generator; //pre-declare the parser-generator

class production; //productions show how to get from a list of terminals to a non-terminal...ish
class production_info;//hold data from a production, for the final parser

class parser;

///// functionals /////

class parser_function_class
//a class that will encapsulate all actions that the parser takes
{
public:
    virtual dyn_holder call(std::vector<token_data>& data)=0;
};
typedef std::shared_ptr<parser_function_class> parser_function_ptr;

typedef<typename return_T>
class parser_functional : parser_function_class
//inherits from parser_action_class to wrap around std::function
{
    std::function<return_T(std::vector<token_data>& )> func;
public:
    parser_functional(std::function<return_T(std::vector<token_data>& )> _func){func=_func;}
    virtual dyn_holder call(std::vector<token_data>& data) { return dyn_holder( func(data) ); }
};

class lexer_function_class
{
public:
    virtual token_data operator()( utf8_string&, location_span&, lexer<token_data>* )=0;
};

class lexer_function_generic : public lexer_function_class
{
    bool return_data;
    unsigned int terminal_ID;
public:
    lexer_function_generic(unsigned int _ID, bool _return_data)
    {
        terminal_ID=_ID;
        return_data=_return_data;
    }

    virtual token_data operator()( utf8_string& data, location_span& loc, lexer<token_data>* lex)
    {
        lex->continue_lexing(return_data);
        dyn_holder new_data(data);
        token_data ret(teminal_ID, new_data, loc);
        return ret;
    }
};
    
template<typedef ret_type>
class lexer_functional: public lexer_function_class
{
    std::function<ret_type(utf8_string, lexer<token_data>*)> func;
    unsigned int terminal_ID;
public:
    lexer_functional(unsigned int _ID, std::function<ret_type(utf8_string, lexer<token_data>*)> _func)
    {
        teminal_ID=_ID;
        func=_func;
    }

    virtual token_data operator()( utf8_string& data, location_span& loc, lexer<token_data>* lex)
    {
        dyn_holder new_data( func(data, lex) );
        token_data ret(teminal_ID, new_data, loc);
        return ret;
    }
};

///// tokens /////

class token
//represents a token in our languege. Is either a terminal or a non-terminal token
{
private:
    friend std::ostream& operator<<(std::ostream& os, const token& dt);
public:
    utf8_string name;
    bool is_terminal;
    unsigned int token_ID; //starts at 1. 0 means "unknown"
    
    token(utf8_string _name); //new unknown terminal
    
    //extra constructors are not needed, as mostly derived classes will be used
};
typedef std::shared_ptr<token> token_ptr;

class terminal : public token
//represents a terminal token
{
private:
    parser_generator* par_gen;
    
public:
    terminal(utf8_string _name, unsigned int ID, parser_generator* _par_gen);
    
    //functions to add patterns
    void add_pattern(utf8_string pattern, bool return_data=true); //add a patern that either returns, or doesn't. with data as string
    template<typename ret_type>
    void add_pattern(utf8_string pattern, std::function<ret_type(utf8_string, lexer<token_data>*)> _func); //specific action
    template<typename ret_type>
    void add_patterns(std::initializer_list<utf8_string> patterns, std::function<ret_type(utf8_string, lexer<token_data>*)> _func); //specific action to multiple patterns
};
typedef std::shared_ptr<terminal> terminal_ptr;

class non_terminal : public token
{
private:
    parser_generator* par_gen;
public:
    std::list< std::shared_ptr<production> > productions;
    
    non_terminal(utf8_string _name, unsigned int ID, parser_generator* _par_gen);
    
    production& add_production(std::initializer_list< token_ptr > tokens);
};
typedef std::shared_ptr<non_terminal> non_terminal_ptr;

class token_data
//holds data passed around by the parser
{
    unsigned int token_ID;
    dyn_holder t_data;
    location_span span;
public:
    token_data(unsigned int _ID, dyn_holder _data, location_span _span);
    
    template<typename ret_T>
    ret_T data();
    
    template<typename ret_T>
    void data(ret_T& ret_data);
    
    location_span loc();
    
    unsigned int get_ID();
};

///// productions //////

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
    std::vector<token_ptr> tokens;
    parser_function_ptr action;
    association _assoc;
    
    production(non_terminal* _L_val, std::vector<token_ptr>& _tokens);
    
    template<typename ret_type>
    production& action(std::function<ret_type(std::vector<token_data>&)> _func);
    
    production& assoc(association _assoc);
    production& assoc(utf8_string _assoc); //will convert a utf8_string into one of the three associations
    
    production_info get_info();
};


class production_info
//contains data about a production. Used by parser for reporting info
{
    friend std::ostream& operator<<(std::ostream& os, const production_info& dt);
public:
    unsigned int L_val_ID;
    utf8_string L_val_name;
    unsigned int num_tokens;
    parser_function_ptr action; //will need to see if we really need this
    
    production_info(unsigned int _L_val_ID, utf8_string& _L_val_name, unsigned int _num_tokens, parser_function_ptr _action);
};
typedef std::shared_ptr<production_info> production_info_ptr;

///// items /////
class item
{
    friend std::ostream& operator<<(std::ostream& os, const item& dt);
public:
    std::shared_ptr<production> prod;
    unsigned int loc;
    token_ptr lookahead;
    
    item(std::shared_ptr<production> _prod, unsigned int _loc); //LR 0 constructor
    item(std::shared_ptr<production> _prod, unsigned int _loc, token_ptr _lookahead); //LR 1 constructor
    
    bool is_kernal( non_terminal_ptr augmented_start);
    bool is_LR0();
    
    std::list<token_ptr> get_postTokens();
    
    item copy();
    item copy(token_ptr new_lookahead);
    
    bool operator==(item& RHS);
};

class item_set
{
    friend std::ostream& operator<<(std::ostream& os, const item_set& dt);
public:
    typedef std::list<item>::iterator iterator;

    std::list<item> items;
    std::map<unsigned int, std::shared_ptr<item_set>  > goto_table;
    unsigned int id;
        
    item_set();
    item_set(unsigned int _id);
    item_set(unsigned int _id, item first_item);
    
    void append(item new_item);
    void add_goto(token_ptr _token, std::shared_ptr<item_set> goto_set);
    std::shared_ptr<item_set> get_goto(token_ptr _token);
    
    bool operator==(item_set& RHS);
    bool has_item(item& RHS);
    unsigned int size();
    
    iterator begin();
    iterator end();
};
typedef std::shared_ptr<item_set> item_set_ptr;

class propagation_table
{
private:
    typedef std::pair< item_set_ptr, item& >  FROM_T;
    typedef std::pair< item_set_ptr, item& >  TO_T;
    
    class table_iterator
    //a class for looping over returns from the table
    {
    public:
        typedef std::multimap< FROM_T, TO_T >::iterator iterator;
        pair<iterator, iterator> iters;
        
        table_iterator(pair<iterator, iterator>& _iters);
        iterator& begin();
        iterator& end();
    };
public:
    std::multimap< FROM_T, TO_T > table;
    
    void add_propagation(item_set_ptr from_set, item& from_item, item_set_ptr to_set, item& to_item);
    table_iterator get_propagation(item_set_ptr from_set, item& from_item);
};

///// parser_state /////
class parser_action
{
    friend std::ostream& operator<<(std::ostream& os, const parser_action& dt);
private:
    enum action_type
    {
        ERROR,
        ACCEPT,
        SHIFT,
        REDUCE,
        NONE
    };
    
    action_type action_todo;
    unsigned int data;
    
    parser_action(action_type _todo, unsigned int _data);
    
public:

    static parser_action get_none();
    static parser_action get_error();
    static parser_action get_accept();
    static parser_action get_shift(unsigned int state);
    static parser_action get_reduce(unsigned int production_ID);
    
    bool is_error();
    bool is_accept();
    bool is_shift();
    bool is_reduce();
    bool is_none();
    
    unsigned int get_data();
    
    bool operator==(parser_action& RHS);
};

class parser_state
{
    std::map<unsigned int, unsigned int> GOTO; //on a token goto a state
    std::map<unsigned int, parser_action> >  ACTION; //on acceptance of a non-term, take an action
    parser_action default_action;
public:
    parser_state();

    void add_goto(unsigned int _token_ID, unsigned int _state);
    void add_action(unsigned int _non_term, parser_action _action);
    void set_default(parser_action _default);
    
    unsigned int get_goto(unsigned int _token_ID);
    parser_action& get_action(unsigned int non_term);
    
};

class parser_generator
{
private:
    utf8_string parser_table_file_name;
    terminal_ptr EOF_terminal;
    terminal_ptr EPSILON_terminal;
    
    std::map<utf8_string, terminal_ptr> terminals;
    std::map<utf8_string, non_terminal_ptr> non_terminals;
    unsigned int next_token_num;
    
    std::shared_ptr<lexer_generator> lex_gen;
    
    non_terminal_ptr start_nonterm;//we may want more than one of these.
    //operator precedenace
    bool parser_table_generated;
    
    //the parse table information
    std::map<unsigned int, utf8_string> term_map; //map terminal ID to terminal name
    std::vector<production_info_ptr> production_information;
    std::vector<parser_state> state_table;
    
public:
    parser_generator(utf8_string _parser_table_file_name, utf8_string _lexer_table_file_name);
    
    //set error function
    
    //functions to define the languege
    terminal_ptr get_EOF_terminal();
    terminal_ptr get_EPSILON_terminal();
    std::shared_ptr<lexer_generator> get_lexer_generator();
    
    //these two will throw a general exception if there is any token of the same name
    terminal_ptr new_terminal(utf8_string name);
    non_terminal_ptr new_nonterminal(utf8_string name);
    
    void set_start_nonterm(non_terminal_ptr _start_nonterm);
    void set_precedence(std::initializer_list<non_terminal_ptr> operators);
    void print_grammer();
    
    std::shared_pointer<parser> get_parser();
    
private: //functions usefull for terminal and non_terminal
    friend class terminal;
    friend class non_terminal;
    
    void add_lexer_pattern(utf8_string& regex_pattern, lexer_function_class func);
    void resolve_unknown_terminal(token_ptr _new_terminal); //this is for when the terminal is referanced by a string
    

private: //languege processing functions
    std::list<token_ptr> first(std::list<token_ptr>& tokens);
    std::list<token_ptr> first_single(token_ptr _token, std::list<unsigned int>& exclusion_productions, bool& output_contains_epsilon);
    
    void closure_LR1(item_set_ptr input_set);
    void closure_LR0(item_set_ptr input_set);
    
    item_set_ptr goto_LR0(item_set_ptr input_set, token_ptr _token);
    std::list<item_set_ptr> LR0_itemsets(non_terminal_ptr _start_token);
    
    void generate_parser_table();
    void load_from_file();
    void load_to_file();
};

}//end csu namespace
#endif
