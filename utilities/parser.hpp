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
#include "iter_wrap.hpp"

//TODO:
//  add multiple starting non-terms

//Algorithm improvements:
// in class item: replace get_PostTokens with two functions, one gives number of posttokens, and ond returns individual post-tokens based on index
// when calculating LR0 closure, use something to keep track if a production was added or not. A boolean in the production, or an array
// replace deque with list in appropriate places
// fix propagation_table generation to loop over tokens last
// test for slowdowns of copying token_data and item (espcially in propagation_table)
// add parser table compaction



//utilities for defining languege
namespace csu{ //cyth standard utilities namespace

///// foward declarations /////

class parser_generator; //pre-declare the parser-generator

class non_terminal;

class production; //productions show how to get from a list of terminals to a non-terminal...ish
class production_info;//hold data from a production, for the final parser

class parser;

///// token data /////

class token_data
//holds data passed around by the parser
{
    unsigned int token_ID;
    dyn_holder t_data;
    location_span span;
public:
    token_data(){}
    token_data(unsigned int _ID, dyn_holder _data, location_span _span);

    dyn_holder data()
    {
        return t_data;
    }

    template<typename ret_T>
    ret_T data()
    {
        return *t_data.cast<ret_T>();
    }

    template<typename ret_T>
    void data(ret_T& ret_data)
    {
        t_data.cast(ret_data);
    }

    location_span loc();

    unsigned int get_ID();
};

///// functionals /////

//parser
class parser_function_class
//a class that will encapsulate all actions that the parser takes
{
public:
    typedef iter_wrap<std::list<token_data> > data_T;

    virtual dyn_holder call(data_T& data)=0;
};
typedef std::shared_ptr<parser_function_class> parser_function_ptr;

template<typename return_T>
class parser_functional : public parser_function_class
//inherits from parser_action_class to wrap around std::function
{
    std::function<return_T(data_T& )> func;
public:
    parser_functional(std::function<return_T(data_T& )> _func){func=_func;}
    virtual dyn_holder call(data_T& data) { return dyn_holder( func(data) ); }
};

class parser_return_action : public parser_function_class
//a simple action for the parser to just return the data associated with the ith token
{
private:
    int ith_token;

public:
    parser_return_action(int i)
    {
        ith_token=i;
    }

    virtual dyn_holder call(data_T& data)
    {
        return data[ith_token].data();
    }
};



//lexer
class lexer_function_generic //: public lexer_function_class
{
    bool return_data;
    unsigned int terminal_ID;
public:
    lexer_function_generic(unsigned int _ID, bool _return_data);

    token_data operator()( utf8_string& data, location_span& loc, lexer<token_data>* lex);
};


template<typename ret_type>
class lexer_functional//: public lexer_function_class
{
    std::function<ret_type(utf8_string, lexer<token_data>*)> func;
    unsigned int terminal_ID;
public:
    lexer_functional(unsigned int _ID, std::function<ret_type(utf8_string, lexer<token_data>*)> _func)
    {
        terminal_ID=_ID;
        func=_func;
    }

    token_data operator()( utf8_string& data, location_span& loc, lexer<token_data>* lex)
    {
        dyn_holder new_data( func(data, lex) );
        token_data ret(terminal_ID, new_data, loc);
        return ret;
    }
};

///// tokens /////

class token
//represents a token in our languege. Is either a terminal or a non-terminal token
{
//private:
  //  friend



public:
    utf8_string name;
    bool is_terminal;
    unsigned int token_ID; //starts at 1. 0 means "unknown"
    token(){ }

    //token(){ }
    virtual ~token(){}

    //extra constructors are not needed, as mostly derived classes will be used
};
std::ostream& operator<<(std::ostream& os, const token& dt);
typedef std::shared_ptr<token> token_ptr;

class terminal : public token
//represents a terminal token
{
private:
    lexer_generator<token_data>* lex_gen;

public:
    terminal(utf8_string _name, unsigned int ID, lexer_generator<token_data>* _lex_gen);
    terminal(utf8_string _name); //new unknown terminal

    //functions to add patterns
    void add_pattern(utf8_string pattern, bool return_data=true); //add a patern that either returns, or doesn't. with data as string

    template<typename ret_type>
    void add_pattern(utf8_string pattern, std::function<ret_type(utf8_string, lexer<token_data>*)> _func) //specific action
    {
        lexer_functional<ret_type> new_lex_func(token_ID, _func);
        lexer_generator<token_data>::lex_func_t lex_func(new_lex_func);
        lex_gen->add_pattern(pattern, lex_func);
    }

//    template<typename ret_type>
//    void add_patterns(std::initializer_list<utf8_string> patterns, std::function<ret_type(utf8_string, lexer<token_data>*)> _func) //specific action to multiple patterns
//    {
//        lexer_functional<ret_type> new_lex_func(token_ID, _func);
//        for(auto& pattern : patterns)
//        {
//            par_gen->add_lexer_pattern(pattern, new_lex_func);
//        }
//    }
};
typedef std::shared_ptr<terminal> terminal_ptr;

class token_string_converter
// this class allows C++ to implicitly convert from a string to a undefined token.
// it is necisary for the add_production function above to work appropriately
{
    public:
    token_ptr data;

    token_string_converter(const char* _data);
    token_string_converter(std::shared_ptr<non_terminal> _data);
    token_string_converter(token_ptr _data);
    token_string_converter(terminal_ptr _data);
    operator token_ptr() const;
};

class non_terminal : public token
{
private:
    parser_generator* par_gen;
public:
    std::list< std::shared_ptr<production> > productions;

    non_terminal(utf8_string _name, unsigned int ID, parser_generator* _par_gen);

    production& add_production(std::initializer_list< token_string_converter > tokens);
};
typedef std::shared_ptr<non_terminal> non_terminal_ptr;

///// productions //////

class production
//represents a production, that could produce a non-terminal
{
private:
    //friend std::ostream& operator<<(std::ostream& os, const production& dt);
    static unsigned int next_production_ID;
    static unsigned int next_precedence_level;
public:
   enum association
   {
      NONE,
      LEFT,
      RIGHT
   };

    unsigned int id;
    non_terminal* L_val;
    std::vector<token_ptr> tokens;
    parser_function_ptr action;
    association assoc;
    unsigned int precedence; //lower number means higher precedence. Highist precedence is 1. 0 is no-precedence.

    production(non_terminal* _L_val, std::vector<token_ptr>& _tokens);

    template<typename ret_type>
    production& set_action(std::function<ret_type(parser_function_class::data_T&)> _func)
    {
        parser_function_ptr new_parser_func(new parser_functional<ret_type>(_func));
        action = new_parser_func;
        return *this;
    }

    production& set_return_action(int i);
    /// set the action to just return the data associated with the i'th token

    production& set_associativity(association _assoc);
    production& set_associativity(utf8_string _assoc); //will convert a utf8_string into one of the three associations
    production& set_precedence();//productions where this is set first will have a higher precedence

    std::shared_ptr<production_info> get_info();

    bool is_left_associative();
    bool is_right_associative();
};
std::ostream& operator<<(std::ostream& os, const production& dt);


class production_info
//contains data about a production. Used by parser for reporting info
{
    //friend std::ostream& operator<<(std::ostream& os, const production_info& dt);
public:
    unsigned int L_val_ID;
    utf8_string L_val_name;
    unsigned int num_tokens;
    parser_function_ptr action;

    production_info(unsigned int _L_val_ID, utf8_string& _L_val_name, unsigned int _num_tokens, parser_function_ptr _action);
};
std::ostream& operator<<(std::ostream& os, const production_info& dt);
typedef std::shared_ptr<production_info> production_info_ptr;

///// items /////
class item
{
   // friend std::ostream& operator<<(std::ostream& os, const item& dt);
   // friend bool operator<(const item &LHS, const item &RHS);
public:
    std::shared_ptr<production> prod;
    unsigned int loc;
    token_ptr lookahead;

    item(){}
    item(std::shared_ptr<production> _prod, unsigned int _loc); //LR 0 constructor
    item(std::shared_ptr<production> _prod, unsigned int _loc, token_ptr _lookahead); //LR 1 constructor

    bool is_kernal( non_terminal_ptr augmented_start);
    bool is_LR0();

    std::vector<token_ptr> get_postTokens();

    item copy();
    item copy(token_ptr new_lookahead);

    bool operator==(const item& RHS) const;

};
std::ostream& operator<<(std::ostream& os, const item& dt);
bool operator<(const item &LHS, const item &RHS);


class item_set
{
    //friend std::ostream& operator<<(std::ostream& os, const item_set& dt);
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
std::ostream& operator<<(std::ostream& os, const item_set& dt);
typedef std::shared_ptr<item_set> item_set_ptr;

class propagation_table
{
private:
    typedef std::pair< unsigned int, item >  FROM_T;
    typedef std::pair< item_set_ptr, item >  TO_T;

    class table_iterator
    //a class for looping over returns from the table
    {
    public:
        typedef std::multimap< FROM_T, TO_T >::iterator iterator;
        std::pair<iterator, iterator> iters;

        table_iterator(std::pair<iterator, iterator> _iters);
        iterator& begin();
        iterator& end();
    };
public:
    std::multimap< FROM_T, TO_T > table;

    void add_propagation(item_set_ptr from_set, item& from_item, item_set_ptr to_set, item& to_item);
    table_iterator get_propagation(item_set_ptr from_set, item& from_item);

    void print();
};

///// parser_state /////
class parser_action
{
    //friend std::ostream& operator<<(std::ostream& os, const parser_action& dt);
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
    parser_action();

    static parser_action get_none();
    static parser_action get_error();
    static parser_action get_accept();
    static parser_action get_shift(unsigned int state);
    static parser_action get_reduce(unsigned int production_ID);

    bool is_error() const;
    bool is_accept() const;
    bool is_shift() const;
    bool is_reduce() const;
    bool is_none() const;

    unsigned int get_data() const;

    bool operator==(parser_action& RHS);

    //for reading and writing the data to and from a file
    void binary_write(std::ostream& output);
    void binary_read(std::istream& input);
};
std::ostream& operator<<(std::ostream& os, const parser_action& dt);

class parser_state
{
public:
    std::map<unsigned int, unsigned int> GOTO; //on a token goto a state
    std::map<unsigned int, parser_action>  ACTION; //on acceptance of a non-term, take an action
    parser_action default_action=parser_action::get_error();

    parser_state();

    void add_goto(unsigned int _token_ID, unsigned int _state);
    void add_action(unsigned int _non_term, parser_action _action);
    void set_default(parser_action _default);

    unsigned int get_goto(unsigned int _token_ID);
    parser_action get_action(unsigned int non_term);
};

const unsigned int error_token_id=1;
const unsigned int eof_token_id=error_token_id+1;
class parser_generator
{
private:
    utf8_string parser_table_file_name;
    terminal_ptr EOF_terminal;
    terminal_ptr ERROR_terminal;

    std::map<utf8_string, terminal_ptr> terminals;
    std::map<utf8_string, non_terminal_ptr> non_terminals;
    unsigned int next_token_num;

    std::shared_ptr<lexer_generator<token_data> > lex_gen;

    non_terminal_ptr start_nonterm;//we may want more than one of these.
    bool parser_table_generated;

    //the parse table information
    std::shared_ptr< std::map<unsigned int, utf8_string> > term_map; //map terminal ID to terminal name
    std::shared_ptr< std::vector<production_info_ptr> > production_information;
    std::shared_ptr< std::vector<parser_state> > state_table;

public:
    parser_generator(utf8_string _parser_table_file_name, utf8_string _lexer_table_file_name);

    //functions to define the languege
    terminal_ptr get_EOF_terminal();
    terminal_ptr get_ERROR_terminal();
    std::shared_ptr<lexer_generator<token_data> > get_lexer_generator();

    //these two will throw a general exception if there is any token of the same name
    terminal_ptr new_terminal(utf8_string name);
    non_terminal_ptr new_nonterminal(utf8_string name);

    void set_start_nonterm(non_terminal_ptr _start_nonterm);
    void print_grammer();
    void reset_table(); //resets the parser_table so that it will need to be regenerated.

    std::shared_ptr<parser> get_parser(bool do_file_IO=true);

    template<class lexertype, class parsertype>
    std::shared_ptr<parsertype> get_parser(bool do_file_IO=true)
    {
        if(not parser_table_generated)
        {
            if(do_file_IO)
            {
                load_from_file();
            }

            if( not parser_table_generated)
            {
                generate_parser_table();
                if(do_file_IO) load_to_file();
            }
        }

        if( not parser_table_generated )
        {
            return nullptr; // maybe should throw error or try to recover?
        }

        std::shared_ptr<lexertype> new_lex = lex_gen->get_lexer<lexertype>(do_file_IO);
        auto new_lex2 = std::static_pointer_cast< lexer<token_data> >( new_lex );
        return std::make_shared< parser >(new_lex2, term_map, production_information, state_table);
    }

private: //functions usefull for terminal and non_terminal
    friend class terminal;
    friend class non_terminal;

    //void add_lexer_pattern(utf8_string& regex_pattern, lexer_generator<token_data>::lex_func_t func);
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

/// TODO: fix lexer to be shared_ptr
class parser
{
private:
    std::shared_ptr< std::map<unsigned int, utf8_string> > term_map; //map terminal ID to terminal name
    std::shared_ptr< std::vector<production_info_ptr> > production_information;
    std::shared_ptr< std::vector<parser_state> > state_table;

    std::list< token_data > stack;
    token_data next_terminal;
    unsigned int error_recovery; //is zero if not in error recovery

    void state_string(std::ostream& os);
    int parse_step(bool reporting=false);

public:
    std::shared_ptr<lexer<token_data> > lex;

    parser(std::shared_ptr<lexer<token_data> > _lex, std::shared_ptr< std::map<unsigned int, utf8_string> > _term_map,
           std::shared_ptr< std::vector<production_info_ptr> > _production_information, std::shared_ptr< std::vector<parser_state> > _state_table);

    std::shared_ptr<parser> copy();
    dyn_holder parse(bool reporting=false);
    dyn_holder get_data();

    //void reset_input(utf8_string& file_name);
    void reset_input(std::istream& _input);
    void reset_input(std::istream& _input, utf8_string& file_name);
    void reset();

};

}//end csu namespace
#endif
