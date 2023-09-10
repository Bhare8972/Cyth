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
#include "serialize.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <functional>
#include <initializer_list>
#include <memory>
#include <vector>
#include <list>

namespace csu{ //cyth standard utilities namespace

//basic utility classes

class location_span;
class location
//location class, to track a location in a file
{
    friend std::ostream& operator<<(std::ostream& os, const location& dt);
    public:

    utf8_string file_name;
    int line;
    int column;

    location();
    location(utf8_string& _file_name);
    location(const location& RHS);
    //move the location forward according to some string

    location& operator=(const location& other) ;
    //assignment

    bool operator<(const location& LHS) const;
    bool operator==(const location& LHS) const;

    bool operator<=(const location& LHS) const;
    bool operator>(const location& LHS) const;
    bool operator>=(const location& LHS) const;

    location_span update(utf8_string& input); //moves the location forward by the input. Return the span between new and old positions
    void backup(utf8_string& input); //oppisite of update. Assumes previous lines all have length zero
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
    location_span();

    bool is_comparible(const location_span& LHS);

    bool strictly_GT(const location_span& LHS);
    bool strictly_LT(const location_span& LHS);
};
location_span operator +(const location_span& LHS, const location_span& RHS);
location_span operator +( location_span& LHS, location_span& RHS);
std::ostream& operator<<(std::ostream& os, const location_span& dt);

class ring_buffer
//ring_buffer class is used to store data in from a file so it can be processed
//implimented like a doubly-linked list
{
    std::istream& fin;
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

    bool has_loaded_EOF; //is true if the entire file has been loaded. EG: empty_node is at end of file
    bool has_read_EOF; //is true if has_loaded_EOF is true and end_node==empty_node
    ring_buffer_node* start_node; //beginning of the loaded and read data
    ring_buffer_node* end_node; //end (not included) of the read data
    ring_buffer_node* empty_node; //end (not included) of the loaded data
    int length_read; //distance from start_node to end_node
    int length_loaded; //distance from start_node to empty_node
    int n_nodes;

    ring_buffer( std::istream& fin_);
    ~ring_buffer();

    void add_nodes(int n_nodes_);
    //create extra empty nodes after the empty node

    void load_data();
    //load data from the input file

    code_point next();
    //return next charector without 'reading' it

    bool next_is_EOF();
    //return if the next codepoint is EOF (as next() won't necisarily say)

    code_point read();
    //read the next charector and return it

    utf8_string get_string();
    //return a string representing all read nodes

    utf8_string reset_string();
    //set start_node to end_node, and return string that was read

    void backup(int N);
    //backup end node N places

    void put(utf8_string& data);
    //put data before empty_node

    void insert(utf8_string& data);
    //place data before start_node, then move start_node backwards
    //throws error if any chars read

    void reject();
    //reject the read string. back end_node to start_node
};

class lexer_exception : public std::exception
//a generalized exception class. Does not take charge of deleteing char*
{
private:
    template< typename T, typename...Ts>
    void add_msg(std::stringstream& stream, T msg_head, Ts... msg)
    {
        stream<<msg_head;
        add_msg(stream, msg...);
    }

    void add_msg(std::stringstream& stream)
    {
        msg=stream.str();
    }

public:
    std::string msg;

    template<typename... Ts>
    lexer_exception(Ts... _msgs)
    {
        std::stringstream stream;
        add_msg(stream, _msgs...);
    }

    lexer_exception(const lexer_exception& to_copy) : msg(to_copy.msg )
    //copy constructor
    {
    }


    const char* what() const throw()
    {
        return msg.c_str();
    }
};

template<typename return_type>
class lexer
// a class to lex files
{
public:
    typedef std::function<return_type (utf8_string&, location_span&, lexer<return_type>*)>  lex_func_t;
private:

    //the lexing table info
    lex_func_t EOF_action;
    std::shared_ptr< std::vector< std::shared_ptr<DFA_state> > > state_table;
    std::shared_ptr< std::vector< lex_func_t > > actions;
    std::shared_ptr< std::vector< unsigned int> > lexer_states;

    //input
    std::shared_ptr< ring_buffer > input_buffer;

    //state
    unsigned int lexer_state;
    bool continue_lexing_b; //a variable to tell if we keep lexing after we kind a token?

public:

    location loc;

    lexer(lex_func_t _EOF_action, std::shared_ptr< std::vector< std::shared_ptr<DFA_state> > > _state_table,
                std::shared_ptr< std::vector< lex_func_t > > _actions, std::shared_ptr< std::vector< unsigned int> > _lexer_states)
    {
        continue_lexing_b=false;
        lexer_state=0;
        EOF_action=_EOF_action;
        state_table=_state_table;
        actions=_actions;
        lexer_states=_lexer_states;
        std::stringstream tmp;
        input_buffer=std::shared_ptr<ring_buffer>(new ring_buffer(tmp));
    }

    lexer( const lexer<return_type>& RHS)
    {
        continue_lexing_b = false;
        lexer_state = 0;
        EOF_action = RHS.EOF_action;
        state_table = RHS.state_table;
        actions = RHS.actions;
        lexer_states = RHS.lexer_states;
        std::stringstream tmp;
        input_buffer = std::shared_ptr<ring_buffer>(new ring_buffer(tmp));
    }

    virtual ~lexer() = default;

    void print_machine()
    {
        unsigned int i=0;
        for(auto DFE: *state_table)
        {
            std::cout<<"state: "<<i<<std::endl;
            DFE->print();
            i++;
        }
    }

    void reset()
    //reset the lexer state to keep lexing. Does not alter the location in the input source.
    {
        lexer_state = 0;
        continue_lexing_b = false;
    }

    //functions to control buffer
//    void set_input(utf8_string& file_name)
//    {
//        std::ifstream fin(file_name.to_cpp_string());
//        input_buffer = std::shared_ptr<ring_buffer>( new ring_buffer(fin)); //this line may need to be updated
//        loc = location( file_name );//reset location
//    }

    void set_input(std::istream& _input)
    {
        input_buffer = std::shared_ptr<ring_buffer>(new ring_buffer(_input));
        loc = location();//reset location
        loc.file_name = "input_stream";
    }

    void set_input(std::istream& _input, utf8_string &stream_name)
    {
        input_buffer = std::shared_ptr<ring_buffer>(new ring_buffer(_input));
        loc = location();//reset location
        loc.file_name = stream_name;
    }

    void unput(utf8_string& data)
    {
        loc.backup(data);
        input_buffer->insert(data);
    }

    //lexing control functions
    void set_state(unsigned int _new_state)
    {
        lexer_state = _new_state;
    }

    void continue_lexing(bool _cont_lex)
    {
        continue_lexing_b = _cont_lex;
    }

    return_type operator()()
    //lex and eat patterns until the _return_data flag for an action is true, in which case, run the function and return the data
    {
        while(true)
        {
            unsigned int initial_DFA_index = (*lexer_states)[lexer_state];
            unsigned int DFA_state_index = 0;
            auto DFA_state = (*state_table)[DFA_state_index+initial_DFA_index];
            unsigned int last_action_index = 0;//note that this is illegitamate until has_read_accepting_state is true
            bool has_read_accepting_state = false;
            while(not input_buffer->has_read_EOF)
            {
                if(DFA_state->accepting_info != -1)
                {
                    has_read_accepting_state=true;
                    last_action_index=DFA_state->accepting_info;
                }

                auto next_char = input_buffer->next();
                if( next_char.is_empty() )
                {
                    if( input_buffer->next_is_EOF() )
                    {
                        input_buffer->read();//read the EOF
                        break;
                    }
                    else
                    {
                        throw lexer_exception("empty codepoint");
                    }
                }

                int new_state = DFA_state->get_transition( next_char );
                if(new_state==-1)
                {
                    break; //no transition
                }
                else
                {
                    input_buffer->read(); //read the charector.
                    DFA_state_index=new_state; //make the transition
                    DFA_state = (*state_table)[DFA_state_index+initial_DFA_index];
                }
            }


            if(input_buffer->has_read_EOF and DFA_state->accepting_info != -1)
            //there may be an unaccepted state if the next char is EOF
            {
                has_read_accepting_state = true;
                last_action_index = DFA_state->accepting_info;
            }

            if( has_read_accepting_state )
            {
                continue_lexing_b = false;
                utf8_string data = input_buffer->reset_string();
                location_span span = loc.update(data);
                auto ret_data = (*actions)[last_action_index](data, span, this); //control functions can be called here, and change lexer state

                //std::string tmp;
                if(not continue_lexing_b)
                {
                    return ret_data;
                }
            }
            else if(input_buffer->has_read_EOF)
            {
                if(input_buffer->length_read==0) //legitamate EOF
                {
                    continue_lexing_b = false;
                    utf8_string data="";
                    location_span span=loc.update(data);

                    auto ret_data=EOF_action(data, span, this); //this can change the lexer state
                    if(not continue_lexing_b)
                    {
                        return ret_data;
                    }
                }
                else //bad EOF
                {
                    utf8_string data=input_buffer->reset_string();
                    location_span span=loc.update(data);
                    throw lexer_exception("Reached EOF before token( ", data, " ) was fully lexed at ", span);
                }
            }
            else
            {
                input_buffer->read(); //nead to read the char we couldn't transition on.
                utf8_string data = input_buffer->reset_string();
                location_span span = loc.update(data);
                throw lexer_exception("Could not read token (", data, ") ", span);
            }

        }
    }
};

//some utility functions and classes used in the lexer_generator
template<typename return_type >
class lex_func_keep_lexing
{
public:
    return_type operator()(utf8_string& data, location_span& span, lexer<return_type>* lex)
    {
        lex->continue_lexing(true);
        return return_type();
    }
};

template<typename return_type >
class lexer_generator
//a class that will create a lexer for us.
{
public:

    typedef std::function<return_type (utf8_string&, location_span&, lexer<return_type>*)>  lex_func_t;
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
    std::shared_ptr< std::vector< lex_func_t > > actions;
    std::shared_ptr< std::vector< unsigned int> > lexer_states; //a vector that contains which state in state_table is the starting point for a given lexer-state

    lex_func_t EOF_action;

public:


    lexer_generator(utf8_string _state_table_file_name)
    {
        state_table_file_name=_state_table_file_name;
        current_state=0;
        made_table=false;
        actions=std::shared_ptr< std::vector< lex_func_t > >( new std::vector< lex_func_t > );
    }

    unsigned int increment_state()
    {
        current_state+=1;
        return current_state;
    }

    void set_EOF_action( lex_func_t _action)
    {
        EOF_action=_action;
    }

    void add_nonreturn_pattern(utf8_string _regular_exp)
    //add a regex pattern, where the lexer will just eat the pattern and keep parsing.
    {
        pattern new_pattern;
        new_pattern.regular_expression = _regular_exp;
        new_pattern.action_number = actions->size();
        new_pattern.state = current_state;
        patterns.push_back(new_pattern);
        actions->push_back(lex_func_keep_lexing<return_type>());
    }

    void add_pattern(utf8_string _regular_exp, lex_func_t _action)
    //add a regex pattern, and an associated action to take. allow use of C++ stl functionals
    {
        pattern new_pattern;
        new_pattern.regular_expression=_regular_exp;
        new_pattern.state=current_state;
        new_pattern.action_number=actions->size();
        patterns.push_back(new_pattern);
        actions->push_back( _action );
    }

    void add_multi_patterns(std::initializer_list<utf8_string> reg_expressions, lex_func_t _action)
    {
        unsigned int action_number=actions->size();
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

    std::shared_ptr< lexer<return_type> > get_lexer(bool do_file_IO=true)
    {
        if( not made_table)
        {
            if(do_file_IO)
            {
                load_from_file();
            }

            if(not made_table)
            {
                generate_state_tables();
                if(do_file_IO) load_to_file();
            }
        }

        return std::make_shared< lexer<return_type> >(EOF_action, state_table, actions, lexer_states);
    }

    template<class lexertype>
    std::shared_ptr< lexertype > get_lexer(bool do_file_IO=true)
    {
        if( not made_table)
        {
            if(do_file_IO)
            {
                load_from_file();
            }

            if(not made_table)
            {
                generate_state_tables();
                if(do_file_IO) load_to_file();
            }
        }

        return std::make_shared< lexertype >(EOF_action, state_table, actions, lexer_states);
    }

    void load_from_file()
    //read the state table from a file
    {
        std::ifstream fin(state_table_file_name.to_cpp_string());
        if(not fin) return;

        //reset the generator.
        state_table.reset();
        lexer_states.reset();
        made_table=false;

        /*
        //read patterns and check they are the same
        uint num_saved_patterns;
        binary_read(fin, num_saved_patterns);
        if(num_saved_patterns != patterns.size())
        {
            throw lexer_exception("Incompatible saved Lexar table");
        }
        for(uint i=0; i<num_saved_patterns; i++)
        {
            auto& internal_pattern = patterns[i];
            unsigned int saved_action_number;
            binary_read( fin, saved_action_number );
            if(saved_action_number != internal_pattern.action_number)
            {
                throw lexer_exception("Incompatible saved Lexar table");
            }

            unsigned int saved_state;
            binary_read(fin, saved_state);
            if(saved_state != internal_pattern.state)
            {
                throw lexer_exception("Incompatible saved Lexar table");
            }

            std::string saved_reg_exp;
            binary_read( fin, saved_reg_exp );
            if( internal_pattern.regular_expression != saved_reg_exp.c_str() )
            {
                throw lexer_exception("Incompatible saved Lexar table");
            }
        }
        */

        //get state_table
        unsigned int num_states=0;
        binary_read(fin, num_states);
        state_table=std::shared_ptr<std::vector< std::shared_ptr<DFA_state> > > (new std::vector< std::shared_ptr<DFA_state> >() );
        state_table->reserve(num_states);
        for(uint i=0; i<num_states; i++)
        {
            std::shared_ptr<DFA_state> NDFS(new DFA_state());
            binary_read(fin, NDFS->accepting_info);
            uint N_transitions=0;
            binary_read(fin, N_transitions);
            for(uint j=0; j<N_transitions; j++)
            {
                DFA_transition new_tran;
                binary_read(fin, new_tran.new_state);
                new_tran.start=code_point(fin);
                new_tran.stop=code_point(fin);
                NDFS->transitions.push_back(new_tran);
            }
            state_table->push_back(NDFS);
        }

        //get lexer_states
        unsigned int N_lexer_states=0;
        binary_read(fin, N_lexer_states);
        lexer_states=std::shared_ptr<std::vector< unsigned int > >( new std::vector< unsigned int >( ) );
        lexer_states->reserve(N_lexer_states);
        for(uint i=0; i<N_lexer_states; i++)
        {
            uint j=0;
            binary_read(fin, j);
            lexer_states->push_back(j);
        }
        made_table=true;
    }

    void load_to_file()
    //serialize the state_table to a file
    {
        if(not made_table) return;

        std::ofstream fout(state_table_file_name.to_cpp_string(), std::ios_base::binary);

        /*
        Can we read the inputs to file??
        what are the inputs?
        std::list< pattern > patterns;
    struct pattern
    {
        utf8_string regular_expression;
        unsigned int action_number;
        unsigned int state;
    };
        */


/*
        //write paterns, for verification
        uint num_patterns = patterns.size();
        binary_write(fout, num_patterns);
        for(auto& pat : patterns)
        {
           binary_write(fout, pat.action_number);
           binary_write(fout, pat.state);

           std::string OUT = pat.regular_expression.to_cpp_string();
           binary_write(fout, OUT);
        }
*/



        //first we output state_table
        uint num_states=state_table->size();
        binary_write(fout, num_states);
        for(auto ste : *state_table)
        {
            binary_write(fout, ste->accepting_info);
            uint N_transitions=ste->transitions.size();
            binary_write(fout, N_transitions);
            for(auto& tran : ste->transitions)
            {
                binary_write(fout, tran.new_state);
                tran.start.put(fout);
                tran.stop.put(fout);
            }
        }
        //then we output lexer_states
        uint N_lexer_states=lexer_states->size();
        binary_write(fout, N_lexer_states);
        for(auto lex_ste : *lexer_states)
        {
            uint j=lex_ste;
            binary_write(fout, j);
        }
    }

private:

    void generate_state_tables()
    {
        std::cout<<"Generating Lexer"<<std::endl;


        std::list< std::shared_ptr<DFA_state> > current_state_table;
        std::list< unsigned int > current_lexer_states;

        unsigned int lexer_state=0;
        auto pattern_iter=patterns.begin();
        auto pattern_end=patterns.end();
        while(lexer_state<=current_state) //loop over each potential state
        {

            std::list< std::shared_ptr<NFA_state> > NFA_of_lexerstate;
            std::shared_ptr<NFA_state> first_state(new NFA_state);
            NFA_of_lexerstate.push_back(first_state);

            for(  ; pattern_iter!= pattern_end and pattern_iter->state==lexer_state; ++pattern_iter) //loop over each patern that is in the present lexer_state
            {
                unsigned int chars_counted=0;
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

        state_table=std::shared_ptr<std::vector< std::shared_ptr<DFA_state> > > (new std::vector< std::shared_ptr<DFA_state> >(current_state_table.begin(), current_state_table.end()) );
        lexer_states=std::shared_ptr<std::vector< unsigned int > >( new std::vector< unsigned int >(current_lexer_states.begin(), current_lexer_states.end()));
        made_table=true;

        std::cout<<"LEXER MADE"<<std::endl;
    }
};
#define LEX_FUNC(TYPE) [](const utf8_string& data, const location_span& loc, lexer<TYPE>* lex)->TYPE


} //end csu namespace


#endif
