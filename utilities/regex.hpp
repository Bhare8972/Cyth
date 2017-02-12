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


This is a set of classes and functions for use with simple UTF-8 regex.
This is not meant to be a modern competitive regex library, but simply
functionally usefull, particularly for use in defining lexers.
*/

#ifndef LEXER_REGEX_150829030625
#define LEXER_REGEX_150829030625

#include <list>
#include <memory>
#include <iterator>
#include <utility>
#include <vector>

#include "UTF8.hpp"
#include "gen_ex.h"

//document the regex grammer
// the regex grammer has these special charectors: \ . [ ] { } * + ? " | ( )
// inside the regex, any other charector matches itself. To match these charectors, precede them with \    .
// inside a charector class, however, things are different
// to match a newline, use \n,
// outside a charector class:
// . matches any charector
// ( and ) groups regexes
// \ is used for escapes (as said above)
// * matches the previous regex 0 or more times
// + matches the previous regex 1 or more times
// ? matches the prevous regex 0 or 1 times
// | matches the left or the right side.
// *, +, ? and | all apply to the smallest regex possible
// to match a series of regex (eg an 'a' followed by a 'b') simply place them next to each other eg: "ab"
// inside quotes("), every charector, except \, is itself. \n is still newline, and \\ still matches \ .
// cannot match quotes inside of quotes
// inside a class ( [] ):
//      every charector matches itself, except '-' and \.
//      \n is still newline, and \\ still matches \  .
//      - and ] can be matched if they are at beginning.
//      the # charector at beginning of class to include unicode charectcors between (and include) U+C0 to U+1FFFFF
//      can have multiple charectors in beginning of class. 'Beginning' ends when there is a charector that is not a 'beginning of class' charector.
//      any other charector can be matched.
//      ranges can be matched by placing one charector - anouther.  EG: 1-6 matches numbers 1-6 (include 1 and 6)
// unimplimented, but desired, features:
//      use \uxxxxx, where x are lowercase hexadecimal digits, to match unicode chars. Also in strings and classes
//      a ^ at beginning of class means class matches everything but what is inside. (not presently functional)
//           ^ can still be matched if it is not at beginning of class
//      the { and } charectors are used to denote a number of repeats
//      regex functions-ish


//NFA automation
//each state has a set of transistions on spans between charectors. Each span is ordered, but can overlap
//would use a tree with non-overlapping spans, but it would require incrementing and decrementing UTF-8 chars....

namespace csu{

class NFA_transition
//class that represents a span between, and including, two charectors, and the states to transition too
{
public:
    code_point start;
    code_point stop;
    std::list<unsigned int> new_states;

    NFA_transition(const code_point& _start, const code_point& _stop);

    NFA_transition(const code_point& _start, const code_point& _stop, unsigned int first_state);

    NFA_transition(const code_point& _start, const code_point& _stop, const std::list<unsigned int>& _new_states);

    bool in_span(const code_point& val) const;

    bool is_lesser(const code_point& val) const;

    bool is_greater(const code_point& val) const;

    bool equivalent(const code_point& _start, const code_point& _stop ) const;

};//end NFA_transition

class NFA_state
//represent a state in an NFA
{
public:
    static code_point epsilon;
    std::list<NFA_transition> transitions;
    int accepting_info;

    NFA_state();

    NFA_state(int _accepting_info);

    void add_transition(const code_point& _start, const code_point& _stop, unsigned int new_state);

    void add_transition(const code_point& val, unsigned int new_state);

    std::list<unsigned int> get_transitions(const code_point& val);

    void print();
};

class NFA_automation
//this class represents an NFA automation. To test whether certain strings match a regular expression
{

public:
    uint num_states;

    NFA_automation(std::list< std::shared_ptr<NFA_state> >&  _states);

    std::pair<int, int> run(const utf8_string& input, bool print_status=false);
    //tests the input string with the states. Returns the length that was accepted, and the accepting_info of the accepting state. is 0,-1 if not accepting

    void print_states();

private:
    std::vector< std::shared_ptr<NFA_state> >  states; //states in the NFA. first (0th) state is the start state.

    //used in our algorithm
    std::vector< bool > current_states;
    std::vector< bool > new_states;

    bool enter_states(std::vector< bool >& state_list, std::list<unsigned int> transitions);

};

//regex AST nodes

class regex_node
//base regex node, from which to derive all regex nodes
{
public:

    virtual std::list< std::shared_ptr<NFA_state> > get_NFA()=0;
    //function to build the list of NFA states for this node

    virtual utf8_string repr()=0;
};

class multi_span : public regex_node
//regex node representing a union of spans of charectors (VERY large union)
{
public:
    std::list<code_point> initial_points;
    std::list<code_point> final_points;

    multi_span( const std::list<code_point>& _initial_points, const std::list<code_point>& _final_points);

    multi_span(const std::list<code_point>& _points);

    multi_span(code_point pnt);

    std::list< std::shared_ptr<NFA_state> > get_NFA();

    utf8_string repr();
};

class kleane_closure : public regex_node
{
public:
    std::shared_ptr<regex_node> expression;

    kleane_closure( std::shared_ptr<regex_node> _expression );

    std::list< std::shared_ptr<NFA_state> > get_NFA();

    utf8_string repr();
};

class partial_closure : public regex_node
{
public:
    std::shared_ptr<regex_node> expression;

    partial_closure( std::shared_ptr<regex_node> _expression );

    std::list< std::shared_ptr<NFA_state> > get_NFA();

    utf8_string repr();
};

class option_node : public regex_node
{
public:
    std::shared_ptr<regex_node> expression;

    option_node( std::shared_ptr<regex_node> _expression );

    std::list< std::shared_ptr<NFA_state> > get_NFA();

    utf8_string repr();
};

class union_node : public regex_node
{
public:
    std::shared_ptr<regex_node> LHS;
    std::shared_ptr<regex_node> RHS;

    union_node( std::shared_ptr<regex_node> _LHS, std::shared_ptr<regex_node> _RHS );

    std::list< std::shared_ptr<NFA_state> > get_NFA();

    utf8_string repr();
};

class concat_node : public regex_node
{
public:
    std::list< std::shared_ptr<regex_node> > nodes;

    concat_node( );

    concat_node( std::list< std::shared_ptr<regex_node> > _nodes );

    void add_node(std::shared_ptr<regex_node> new_node);

    std::list< std::shared_ptr<NFA_state> > get_NFA();

    utf8_string repr();
};

//functions for parsing regex into a regex AST

void count_whitespace(const utf8_string& regex, uint& position);
//counts the amount of whitespace following(including) position

std::shared_ptr<regex_node> parse_literal(const utf8_string& regex, uint& position);

std::shared_ptr<regex_node> parse_class(const utf8_string& regex, uint& position);

std::shared_ptr<regex_node> parse_single_node(const utf8_string& regex, uint& position);
//parse regex for single regexes. Will raise a gen_exception if regex can't be read

std::shared_ptr<regex_node> parse_concat_node(const utf8_string& regex, uint& position);
//parses a series of regex nodes, forming them into a concat_node Will raise a gen_exception if regex can't be read

std::shared_ptr<regex_node> parse_regex(const utf8_string& regex_pattern, uint& chars_counted);
//parse a regex patern, and return the representitive regex parse tree. Will raise a gen_exception if regex can't be read


//helper functions for producing a NFA

NFA_automation compile_regex_NFA(std::list<utf8_string> patterns);
//take a list of paterns and compile it into a NFA_automation
//will raise a gen_exception if the pattern cannot be read

//DFA automation

class DFA_transition
//class that represents a span between, and including, two charectors, and a single state to transition to
{
public:
    code_point start;
    code_point stop;
    unsigned int new_state;

    DFA_transition();

    DFA_transition(const code_point& _start, const code_point& _stop);
    //will raise a general exception if start is greater than stop

    DFA_transition(const code_point& _start, const code_point& _stop, unsigned int _new_state);
    //will raise a general exception if start is greater than stop

    bool in_span(const code_point& val) const;

    bool is_lesser(const code_point& val) const;

    bool is_greater(const code_point& val) const;

    bool equivalent(const code_point& _start, const code_point& _stop ) const;
};//end DFA_transition

class DFA_state
//represent a state in an DFA
{
public:
    std::list<DFA_transition> transitions;
    int accepting_info;

    DFA_state();

    DFA_state(int _accepting_info);

    void add_transition(const code_point& _start, const code_point& _stop, unsigned int new_state);
    // will throw a gen_exception if _start or _stop is already in the state

    int get_transition(const code_point& val);
    //returns -1 if val is not a valid transition

    bool distinguishable(std::shared_ptr<DFA_state> LHS);
    //returns if LHS is distinguishable from this DFA_state

    void print();
};// end DFA state

class DFA_automation
{
public:
    std::vector<std::shared_ptr<DFA_state> > DFA_states;

    DFA_automation(std::list<std::shared_ptr<DFA_state> >& _DFA_states);

    void print_states();
    //prints the states to std::out

    std::pair<int, int> run(const utf8_string& input, bool print_status=false);
    //tests the input string with the states. Returns the length that was accepted, and the accepting_info of the accepting state. is 0,-1 if not accepting
};//end DFA_automation

//helper functions for producing a DFA
std::list< std::shared_ptr<DFA_state> > NFA_to_DFA(std::list< std::shared_ptr<NFA_state> >& NFA_states);
DFA_automation compile_regex_DFA(std::list<utf8_string> patterns);
std::list< std::shared_ptr<DFA_state> > DFA_minimization(const std::list< std::shared_ptr<DFA_state> >& _DFA_states);
void increment_states(std::list< std::shared_ptr<NFA_state> >& states, uint incre);

}//end namespace

#endif
