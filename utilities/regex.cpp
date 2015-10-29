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

#include "regex.hpp"
#include <iterator>
using namespace std;
using namespace csu;

const char special_regex_charectors[]="\\.[]{}*+?\"|()";

code_point NFA_state::epsilon({0xFF,0,0,0});

//NFA transitions

NFA_transition::NFA_transition(const code_point& _start, const code_point& _stop)
{
    start=_start;
    stop=_stop;
}

NFA_transition::NFA_transition(const code_point& _start, const code_point& _stop, unsigned int first_state)
{
    start=_start;
    stop=_stop;
    new_states.push_back(first_state);
}

NFA_transition::NFA_transition(const code_point& _start, const code_point& _stop, const std::list<unsigned int>& _new_states)
{
    start=_start;
    stop=_stop;
    new_states=_new_states;
}

bool NFA_transition::in_span(const code_point& val) const
{
    return (val==start or val==stop or (val>start and val<stop));
}

bool NFA_transition::is_lesser(const code_point& val) const
{
    return val<start;
}

bool NFA_transition::is_greater(const code_point& val) const
{
    return val>stop;
}

bool NFA_transition::equivalent(const code_point& _start, const code_point& _stop ) const
{
    return start==_start and stop==_stop;
}

//NFA_state
NFA_state::NFA_state()
{
    accepting_info=-1;
}

NFA_state::NFA_state(int _accepting_info)
{
    accepting_info=_accepting_info;
}

void NFA_state::add_transition(const code_point& _start, const code_point& _stop, unsigned int new_state)
{
    auto iter=transitions.begin(), end=transitions.end();
    for( ; iter!=end; ++iter)
    {
        if(iter->equivalent(_start, _stop))
        {
            iter->new_states.push_back(new_state);
            return;
        }
        else if( _start>iter->start )
        {
            ++iter;
            break;
        }
    }

    transitions.emplace(iter, _start, _stop, new_state);
}

void NFA_state::add_transition(const code_point& val, unsigned int new_state)
{
    add_transition(val, val, new_state);
}

std::list<unsigned int> NFA_state::get_transitions(const code_point& val)
{
    std::list<unsigned int> ret;

    for(NFA_transition& tran : transitions)
    {
        if(tran.in_span(val))
        {
            ret.insert(ret.end(), tran.new_states.begin(), tran.new_states.end());
        }
    }

    return ret;
}

void NFA_state::print()
{
    cout<<"  accepting:"<<accepting_info<<endl;
    for(NFA_transition& tran : transitions)
    {
        if(tran.start==NFA_state::epsilon)
            cout<<"  on epsilon";
        else
            cout<<"  from "<<tran.start<<" to "<<tran.stop;
        cout<<" transition to: ";
        for(uint ste : tran.new_states)
        {
            cout<<ste<<" ";
        }
        cout<<endl;
    }
}

//NFA automation
NFA_automation::NFA_automation(std::list< std::shared_ptr<NFA_state> >&  _states)
{
    num_states=_states.size();
    states.reserve( num_states );
    states.insert(states.begin(), _states.begin(),  _states.end());

    current_states.insert(current_states.begin(), num_states, false);
    new_states.insert(new_states.begin(), num_states, false);
}

bool NFA_automation::enter_states(std::vector< bool >& state_list, std::list<unsigned int> transitions)
{
    bool transition_made=false;
    for(unsigned int I : transitions)
    {
        if(not state_list[I])
        {
            transition_made=true;
            state_list[I]=true;
        }
    }
    return transition_made;
}

std::pair<int, int> NFA_automation::run(const utf8_string& input, bool print_status)
{
    uint position_matched=0;
    int accepting_info=-1;
    uint current_position=0;
    fill(current_states.begin(), current_states.end(), false);
    current_states[0]=true;
    while(true)
    {
        // test if we want to continue
        bool cont=false;
        for(bool in_state : current_states)
        {
            if(in_state)
            {
                cont=true;
                break;
            }
        }
        if(not cont) break; //if cont is false, then we have no active states
        if(print_status) cout<<"ITER"<<endl;

        // make epsilon transitions
        bool made_epsilon_transition=false;
        for(uint state_index=0; state_index!=num_states; state_index+=1)
        {
            if(not current_states[state_index]) continue;

            if(states[state_index]->accepting_info>-1 and (current_position>position_matched or states[state_index]->accepting_info<accepting_info ) )
            {
                position_matched=current_position;
                accepting_info=states[state_index]->accepting_info;
                if( print_status) cout<<"accepted:"<<accepting_info<<" at "<<position_matched<<endl;
            }

            if(print_status) cout<<"transitions on epsilon:"<<endl;
            made_epsilon_transition=made_epsilon_transition or enter_states(current_states, states[state_index]->get_transitions(NFA_state::epsilon) );
        }
        if(made_epsilon_transition) continue; //repeat last bit until no epsilon transitions

        //make transitions based upon next charector
        fill(new_states.begin(), new_states.end(), false);
        if(current_position<input.get_length())
        {
            for(uint state_index=0; state_index!=num_states; state_index+=1)
            {
                if(not current_states[state_index]) continue;

                if(print_status) cout<<"Transitions on "<<input[current_position]<<endl;
                enter_states(new_states, states[state_index]->get_transitions(input[current_position]));
            }
            current_position+=1;
        }

        current_states=new_states;
    }

    return make_pair(position_matched, accepting_info);
}

void NFA_automation::print_states()
{
    int i=0;
    for(auto& ste: states)
    {
        cout<<"state "<<i<<endl;
        ste->print();
        i++;
    }
}

//regex AST nodes
void csu::increment_states(list< shared_ptr<NFA_state> >& states, uint incre)
//increment all the state transitions. Usefull for get_NFA algorithms
{
    for(shared_ptr<NFA_state>& ste : states)
    {
        for(NFA_transition& tran : ste->transitions)
        {
            //for(uint i=0; i<tran.new_states.size(); i++)
            //{
                //tran.new_states[i]+=incre;
            //}

            for(auto iter=tran.new_states.begin(), end=tran.new_states.end(); iter!=end; ++iter)
            {
                (*iter)+=incre;
            }
        }
    }
}

//multi_span
multi_span::multi_span( const std::list<code_point>& _initial_points, const std::list<code_point>& _final_points)
{
    initial_points=_initial_points;
    final_points=_final_points;
}

multi_span::multi_span(const std::list<code_point>& _points)
{
    initial_points=_points;
    final_points=_points;
}

multi_span::multi_span(code_point pnt)
{
    initial_points.push_back(pnt);
    final_points.push_back(pnt);
}

std::list< std::shared_ptr<NFA_state> > multi_span::get_NFA()
{
    list< shared_ptr<NFA_state> > ret;
    shared_ptr<NFA_state> init_state(new NFA_state());
    shared_ptr<NFA_state> final_state(new NFA_state());
    ret.push_back(init_state);
    ret.push_back(final_state);

    //for(uint i=0; i<initial_points.size(); i++)
    //{
        //init_state->add_transition(initial_points[i], final_points[i], 1);
    //}

    auto final_points_iter=final_points.begin();
    for(auto initial_pnts_iter=initial_points.begin(), initial_pnts_end=initial_points.end();  initial_pnts_iter!=initial_pnts_end;  ++initial_pnts_iter )
    {
        init_state->add_transition(*initial_pnts_iter, *final_points_iter, 1);

        ++final_points_iter;
    }

    return ret;
}

utf8_string multi_span::repr()
{
    utf8_string ret(initial_points.size()*5+2);
    ret.append("(");

    auto final_points_iter=final_points.begin();
    for(auto& init_points : initial_points)
    {
        ret.append(init_points);
        ret.append("->");
        ret.append(*final_points_iter);
        ++final_points_iter;
    }
    ret.append(")");
    return ret;
}

//kleane_closure
kleane_closure::kleane_closure( std::shared_ptr<regex_node> _expression )
{
    expression=_expression;
}

std::list< std::shared_ptr<NFA_state> > kleane_closure::get_NFA()
{
    auto exp_states=expression->get_NFA();
    increment_states(exp_states, 2);

    shared_ptr<NFA_state> first_ste(new NFA_state);
    shared_ptr<NFA_state> final_ste(new NFA_state);

    first_ste->add_transition(NFA_state::epsilon, 1);
    first_ste->add_transition(NFA_state::epsilon, 2);

    auto& final_exp_state=*(++exp_states.begin());
    final_exp_state->add_transition(NFA_state::epsilon, 1);
    final_exp_state->add_transition(NFA_state::epsilon, 2);

    exp_states.push_front(final_ste);
    exp_states.push_front(first_ste);

    return exp_states;
}

utf8_string kleane_closure::repr()
{
    utf8_string ret(expression->repr());
    ret.append("*");
    return ret;
}

//partial_closure
partial_closure::partial_closure( std::shared_ptr<regex_node> _expression )
{
    expression=_expression;
}

std::list< std::shared_ptr<NFA_state> > partial_closure::get_NFA()
{
    auto exp_states=expression->get_NFA();
    increment_states(exp_states, 2);

    shared_ptr<NFA_state> first_ste(new NFA_state);
    shared_ptr<NFA_state> final_ste(new NFA_state);

    first_ste->add_transition(NFA_state::epsilon, 2);

    auto& final_exp_state=*(++exp_states.begin());
    final_exp_state->add_transition(NFA_state::epsilon, 1);
    final_exp_state->add_transition(NFA_state::epsilon, 2);

    exp_states.push_front(final_ste);
    exp_states.push_front(first_ste);

    return exp_states;
}

utf8_string partial_closure::repr()
{
    utf8_string ret(expression->repr());
    ret.append("+");
    return ret;
}

//option_node
option_node::option_node( std::shared_ptr<regex_node> _expression )
{
    expression=_expression;
}

std::list< std::shared_ptr<NFA_state> > option_node::get_NFA()
{

    auto exp_states=expression->get_NFA();
    increment_states(exp_states, 2);

    shared_ptr<NFA_state> first_ste(new NFA_state);
    shared_ptr<NFA_state> final_ste(new NFA_state);

    first_ste->add_transition(NFA_state::epsilon, 1);
    first_ste->add_transition(NFA_state::epsilon, 2);

    auto& final_exp_state=*(++exp_states.begin());
    final_exp_state->add_transition(NFA_state::epsilon, 1);

    exp_states.push_front(final_ste);
    exp_states.push_front(first_ste);

    return exp_states;
}

utf8_string option_node::repr()
{
    utf8_string ret(expression->repr());
    ret.append("?");
    return ret;
}

//union_node
union_node::union_node( std::shared_ptr<regex_node> _LHS, std::shared_ptr<regex_node> _RHS )
{
    LHS=_LHS;
    RHS=_RHS;
}

std::list< std::shared_ptr<NFA_state> > union_node::get_NFA()
{
    auto LHS_states=LHS->get_NFA();
    auto RHS_states=RHS->get_NFA();
    uint LHS_length=LHS_states.size();

    increment_states(LHS_states, 2);
    increment_states(RHS_states, 2+LHS_length);

    auto& final_LHS_state=*(++LHS_states.begin());
    auto& final_RHS_state=*(++RHS_states.begin());
    final_LHS_state->add_transition(NFA_state::epsilon, 1);
    final_RHS_state->add_transition(NFA_state::epsilon, 1);


    shared_ptr<NFA_state> first_state(new NFA_state);
    shared_ptr<NFA_state> final_state(new NFA_state);
    first_state->add_transition(NFA_state::epsilon, 2);
    first_state->add_transition(NFA_state::epsilon, 2+LHS_length);

    LHS_states.push_front(final_state);
    LHS_states.push_front(first_state);
    LHS_states.insert(LHS_states.end(), RHS_states.begin(), RHS_states.end());

    return LHS_states;
}

utf8_string union_node::repr()
{
    utf8_string LHS_str(LHS->repr());
    utf8_string RHS_str(RHS->repr());
    return LHS_str+"|"+RHS_str;
}

//concat_node
concat_node::concat_node( )
{
}

concat_node::concat_node( std::list< std::shared_ptr<regex_node> > _nodes )
{
    nodes=_nodes;
}

void concat_node::add_node(std::shared_ptr<regex_node> new_node)
{
    nodes.push_back(new_node);
}

std::list< std::shared_ptr<NFA_state> > concat_node::get_NFA()
{
    shared_ptr<NFA_state> first_state(new NFA_state);
    shared_ptr<NFA_state> final_state(new NFA_state);

    list< shared_ptr<NFA_state> > ret;
    ret.push_back(first_state);
    ret.push_back(final_state);

    shared_ptr<NFA_state> tran_state=first_state;
    uint transition_to=2;
    for(auto& con_node : nodes)
    {
        tran_state->add_transition(NFA_state::epsilon, transition_to);

        auto new_states=con_node->get_NFA();
        transition_to+=new_states.size();
        tran_state=*(++new_states.begin());

        increment_states(new_states, ret.size());

        ret.insert(ret.end(), new_states.begin(), new_states.end());
    }
    tran_state->add_transition(NFA_state::epsilon, 1);

    return ret;
}

utf8_string concat_node::repr()
{
    utf8_string ret;
    for(auto nde : nodes)
    {
        ret+="("+nde->repr()+")";
    }
    return ret;
}

//functions for parsing regex into a regex AST
void csu::count_whitespace(const utf8_string& regex, uint& position)
//counts the amount of whitespace following(including) position
{
    for( ; position<regex.get_length(); position++ )
    {
        if(not (regex[position]=="\n" or  regex[position]=="\r" or regex[position]==" ") )
        {
            return;
        }
    }
}

shared_ptr<regex_node> csu::parse_literal(const utf8_string& regex, uint& position)
//parse a regex literal
{
    list< shared_ptr<regex_node> > chars;
    //list<code_point> chars;
    while(position<regex.get_length())
    {
        if(regex[position]=="\"")
        {
            return shared_ptr<regex_node>(new concat_node(chars));
        }
        else if(regex[position]=="\\")//escape char
        {
            position+=1;
            if(position==regex.get_length())
            {
                throw gen_exception("REGEX ERROR: escape charector at end of string");
            }
            else if( regex[position]=="\"" )
            {
                shared_ptr<regex_node> new_node(new multi_span(code_point(0x0022)));
                chars.push_back(new_node);
            }
            else if(regex[position]=="\\")
            {
                shared_ptr<regex_node> new_node(new multi_span(code_point(0x005C)));
                chars.push_back(new_node);
            }
            else if(regex[position]=="n")
            {
                shared_ptr<regex_node> new_node(new multi_span(code_point(0x000A)));
                chars.push_back(new_node);
            }
            else if(regex[position]=="t")
            {
                shared_ptr<regex_node> new_node(new multi_span(code_point(0x0009)));
                chars.push_back(new_node);
            }
            else
            {
                throw gen_exception("REGEX ERROR: unrecognized escape charector");
            }
        }
        else
        {
            shared_ptr<regex_node> new_node(new multi_span(regex[position]));
            chars.push_back(new_node);
            position+=1;
        }
    }

    throw gen_exception("REGEX ERROR: literal not terminated by \"");
}

shared_ptr<regex_node> csu::parse_class(const utf8_string& regex, uint& position)
//parse a regex class
{
    uint regex_len=regex.get_length();
    //bool invert=false;
    list<code_point> span_begin;
    list<code_point> span_end;

    count_whitespace(regex, position);

    if(position==regex_len)
    {
        throw gen_exception("REGEX ERROR: class not terminated by ]");
    }

    //if(regex[position]=="^")
    //{
        //position+=1;
        //invert=false;
    //}
    //else

    //check charectors at beginning
    bool in_beginning=true;
    while(in_beginning)
    {
        in_beginning=false;
        if(regex[position]=="-" or regex[position]=="]") //special literal charectors
        {
            span_begin.push_back(regex[position]);
            span_end.push_back(regex[position]);
            position+=1;
            in_beginning=true;
        }
        else if(regex[position]=="#")//special span
        {
            auto A=code_point(0xC0);
            auto B=code_point(0x1FFFFF);
            span_begin.push_back( A );
            span_end.push_back( B );
            position+=1;
            in_beginning=true;
        }
    }

    while(position<regex_len)
    {
        count_whitespace(regex, position);

        //check end
        if(regex[position]=="]")
        {
            //need to do invert here
            return shared_ptr<regex_node>(new multi_span(span_begin, span_end));
        }

        //check range
        else if((not ((position+2)>=regex_len )) and regex[position+1]=="-")
        {
            const code_point& first_char( regex[position] );
            const code_point& second_char( regex[position+2] );

            if(second_char>first_char)
            {
                span_begin.push_back(first_char);
                span_end.push_back(second_char);
                position+=3;
            }
            else
            {
                throw gen_exception("REGEX ERROR: range start is after range end in class");
            }
        }

        //everything else
        else
        {
            span_begin.push_back( regex[position] );
            span_end.push_back( regex[position] );
            position+=1;
        }
    }

    throw gen_exception("REGEX ERROR: class not terminated by ]");
}

shared_ptr<regex_node> csu::parse_single_node(const utf8_string& regex, uint& position)
//parse regex for single regexes. Will raise a gen_exception if regex can't be read
{
    count_whitespace(regex, position);
    std::shared_ptr<regex_node> new_node;

    if( regex[position]=="\"" ) //we have a literal
    {
        new_node=parse_literal(regex, position);
        if(not new_node)
        {
            return new_node;
        }
        if(not (regex[position]=="\""))
        {
            throw gen_exception("REGEX ERROR: literal is not ended bya '\"'");
        }
        position+=1;
    }
    else if(regex[position]=="(") //we have a group
    {
        position+=1;
        new_node=parse_concat_node(regex, position);
        if(not new_node)
        {
            throw gen_exception("REGEX ERROR: expected a regex group after '('");
        }
        if(not (regex[position]==")"))
        {
            throw gen_exception("REGEX ERROR: regex group not ended with a ')'");
        }
        position+=1;
    }
    else if(regex[position]=="[") //we have a class
    {
        position+=1;
        new_node=parse_class(regex, position);
        if(not new_node)
        {
            throw gen_exception("REGEX ERROR: expected a regex class inside of '['");
        }
        if(not (regex[position]=="]"))
        {
            throw gen_exception("REGEX ERROR: regex class not ended with a ']'");
        }
        position+=1;
    }
    else if(not regex[position].in(special_regex_charectors) ) //just a charector with no special significance
    {
        new_node=shared_ptr<regex_node>( new multi_span(regex[position]) );
        position+=1;
    }
    else if(regex[position]=="\\") //escape char
    {
        if(regex[position+1].in(special_regex_charectors))
        {
            new_node=shared_ptr<regex_node>( new multi_span(regex[position]) );
            position+=2;
        }
        else if(regex[position+1]=="t")
        {
            code_point tab(0x0009);
            new_node=shared_ptr<regex_node>( new multi_span(tab) );
            position+=2;
        }
        else if(regex[position+1]=="n")
        {
            code_point NL(0x000A);
            new_node=shared_ptr<regex_node>( new multi_span(NL) );
            position+=2;
        }
        else
        {
            throw gen_exception("REGEX ERROR: unrecognized escape charector");
        }
    }
    else if(regex[position]==".")
    {
        code_point B(0x000000);
        code_point E(0x1FFFFF);
        new_node=shared_ptr<regex_node>( new multi_span({B},{E}) );
        position+=1;
    }
    else
    {
        throw gen_exception("REGEX EROR: unrecognized charector in regex");
    }

    count_whitespace(regex, position);
    if(position==regex.get_length())
    {
        return new_node;
    }

    //check for trailing operators
    if(regex[position]=="?")
    {
        position+=1;
        new_node=shared_ptr<regex_node>( new option_node(new_node) );
        return new_node;
    }
    else if(regex[position]=="*")
    {
        position+=1;
        new_node=shared_ptr<regex_node>( new kleane_closure(new_node) );
        return new_node;
    }
    else if(regex[position]=="+")
    {
        position+=1;
        new_node=shared_ptr<regex_node>( new partial_closure(new_node) );
        return new_node;
    }
    else if(regex[position]=="|")
    {
        position+=1;
        auto out=parse_single_node(regex, position);
        if(not out)
        {
            return out;
        }
        new_node=shared_ptr<regex_node>( new union_node(new_node, out) );
        return new_node;
    }
    //else if(regex[position]=="{")    repeat node unimplemented
    //{
        //
    //}
    else
    {
        return new_node;
    }
}

shared_ptr<regex_node> csu::parse_concat_node(const utf8_string& regex, uint& position)
//parses a series of regex nodes, forming them into a concat_node Will raise a gen_exception if regex can't be read
{
    shared_ptr<concat_node> ret(new concat_node());

    while(position<regex.get_length())
    {
        if(regex[position]==")")
        {
            break;
        }
        shared_ptr<regex_node> out=parse_single_node(regex, position);
        if(not out)
        {
            return out;
        }
        ret->add_node(out);
    }

    return ret;
}

std::shared_ptr<regex_node> csu::parse_regex(const utf8_string& regex_pattern, uint& chars_counted)
//parse a regex patern, and return the representitive regex parse tree. Will raise a gen_exception if regex can't be read
{
    uint symbols_parsed=0;
    std::shared_ptr<regex_node> ret =parse_concat_node(regex_pattern, symbols_parsed);

    count_whitespace(regex_pattern, symbols_parsed);
    chars_counted=symbols_parsed;
    return ret;
}

//helper functions for producing a NFA
NFA_automation csu::compile_regex_NFA(std::list<utf8_string> patterns)
{
    list< shared_ptr<NFA_state> > total_NFA;
    shared_ptr<NFA_state> first_state(new NFA_state);
    total_NFA.push_back(first_state);

    int pattern_number=0;
    for(auto& pattern : patterns)
    {
        uint chars_counted=0;
        auto regex_tree=parse_regex(pattern, chars_counted);
        if( chars_counted< pattern.get_length() )
            throw gen_exception("full regex cannot be parsed");
        if( not regex_tree)
            throw gen_exception("could not parse regex");

        //get new states, make end state as accepting
        list< shared_ptr<NFA_state> > new_states=regex_tree->get_NFA();
        (*(++new_states.begin()))->accepting_info=pattern_number;
        pattern_number+=1;

        //add to total NFA
        increment_states(new_states, total_NFA.size());
        first_state->add_transition(NFA_state::epsilon, total_NFA.size());
        total_NFA.insert(total_NFA.end(), new_states.begin(), new_states.end());
    }
    return NFA_automation(total_NFA);
}

//helper functions for producing a DFA

list< shared_ptr<DFA_state> > csu::NFA_to_DFA(const list< shared_ptr<NFA_state> >& NFA_states)
{
    typedef shared_ptr<list<unsigned int>> NFA_IN_DFA_TYPE;

    //epsilon closure on some DFA state. No return
    struct EC{ void operator() (NFA_IN_DFA_TYPE dfa_state)
    {
        for( auto iter=dfa_state->begin(); iter!= dfa_state->end(); ++iter ) //recalculating 'end' is inefficient, but the iterator may change?
        {
            auto NFA_ste_iter=*next(_NFA_states.begin(), (*iter));//pointer to the NFA_state
            auto new_states=NFA_ste_iter->get_transitions(NFA_state::epsilon); //get epsilon states
            dfa_state->insert(dfa_state->end(), new_states.begin(), new_states.end()); //insert the new states
        }
    }
    //magic for capture
    EC(const list< shared_ptr<NFA_state> >& __NFA_states) :_NFA_states(__NFA_states) {}
    const list< shared_ptr<NFA_state> >& _NFA_states;
    } epsilon_closure(NFA_states);

    //check if state_to_find is in a_DFA_state. If so, return location. If not, add the state to the state_list and return location
    struct { uint operator() (list<NFA_IN_DFA_TYPE>& all_DFA_states, NFA_IN_DFA_TYPE state_to_find)
    {
        uint state_index=0;
        for(auto current_DFA_state : all_DFA_states)
        {
            if(*current_DFA_state==*state_to_find)
            {
                //states are equal!
                return state_index;
            }
            state_index+=1;
        }

        //could not find the state
        all_DFA_states.push_back(state_to_find);
        return state_index;
    }} find_state;

    //initallize some data structures
    list<NFA_IN_DFA_TYPE> current_DFA_states;
    list< shared_ptr<DFA_state> >  return_DFA_states;

    NFA_IN_DFA_TYPE start_state( new list<unsigned int>({0}));
    epsilon_closure(start_state);
    current_DFA_states.push_back(start_state);

    //loop over each DFA of NFA
    for(auto dfa_of_nfa_state : current_DFA_states)
    {
        //filter all the NFA transitions out of this state
        list< NFA_transition > DFA_transitions;
        for(unsigned int NFA_ste : *dfa_of_nfa_state)
        {
            for(auto NFA_tran_ : (*next(NFA_states.begin(),NFA_ste))->transitions )
            {
                if(NFA_tran_.start==NFA_state::epsilon)
                {
                    continue; //ignore epsilon transitions.
                }

                NFA_transition NFA_tran(NFA_tran_.start,NFA_tran_.stop, NFA_tran_.new_states) ; //copy the NFA transition, as it will be modified
                //now we compare NFA_tran to each of the transitions in the DFA
                list< NFA_transition > new_DFA_transitions;
                bool add_NFA_tran=true;
                for(auto& DFA_tran : DFA_transitions) //we are trying to add NFA_tran into DFA_transitions, but we need to remove all overlap
                {

                    if(DFA_tran.is_lesser(NFA_tran.stop) or DFA_tran.is_greater(NFA_tran.start))//new transition is out of range of old transition
                    {
                        //do nothing
                    }
                    else if(DFA_tran.start==NFA_tran.start) //check three cases where they have the same start
                    {
                        if(DFA_tran.stop==NFA_tran.stop)//the spans are equivalent. Combine them, and discard one...somehow.
                        {
                            DFA_tran.new_states.insert(DFA_tran.new_states.end(), NFA_tran.new_states.begin(), NFA_tran.new_states.end() ); //combine transitions
                            add_NFA_tran=false; //do not add NFA_tran
                            break; //NFA_tran will not interfere with any other transitions
                        }
                        else if(DFA_tran.stop<NFA_tran.stop)
                        {
                            //insert states to DFA_tran.
                            DFA_tran.new_states.insert(DFA_tran.new_states.end(), NFA_tran.new_states.begin(), NFA_tran.new_states.end() );

                            //modify NFA_tran
                            auto DFA_end=DFA_tran.stop.to_UTF32();
                            NFA_tran.start=code_point(DFA_end+1);
                        }
                        else  // DFA_tran.stop>NFA_tran.stop
                        {
                            //insert states to NFA_tran.
                            NFA_tran.new_states.insert(NFA_tran.new_states.end(), DFA_tran.new_states.begin(), DFA_tran.new_states.end() );

                            //modify DFA_tran
                            auto NFA_end=NFA_tran.stop.to_UTF32();
                            DFA_tran.start=code_point(NFA_end+1);
                        }
                    }
                    else if(DFA_tran.stop==NFA_tran.stop)  //check two cases where they have the same end
                    {
                        if(DFA_tran.start<NFA_tran.start)
                        {
                            //insert states to NFA_tran.
                            NFA_tran.new_states.insert(NFA_tran.new_states.end(), DFA_tran.new_states.begin(), DFA_tran.new_states.end() );

                            //modify DFA_tran
                            auto NFA_start=NFA_tran.start.to_UTF32();
                            DFA_tran.stop=code_point(NFA_start-1);
                        }
                        else if(DFA_tran.start>NFA_tran.start)
                        {
                            //insert states to DFA_tran.
                            DFA_tran.new_states.insert(DFA_tran.new_states.end(), NFA_tran.new_states.begin(), NFA_tran.new_states.end() );

                            //modify DFA_tran
                            auto DFA_start=DFA_tran.start.to_UTF32();
                            NFA_tran.stop=code_point(DFA_start-1);
                        }

                    }
                    else if(DFA_tran.in_span(NFA_tran.start) and not DFA_tran.in_span(NFA_tran.stop)) //begining of new transition is at end of old transtion
                    {
                        //create the new transition
                        NFA_transition new_tran(NFA_tran.start, DFA_tran.stop, DFA_tran.new_states); //new transition, with DFA states
                        new_tran.new_states.insert(new_tran.new_states.end(), NFA_tran.new_states.begin(), NFA_tran.new_states.end()); //add NFA states
                        new_DFA_transitions.push_back(new_tran);

                        //modify DFA_tran
                        auto end_char=NFA_tran.start.to_UTF32();
                        DFA_tran.stop=code_point(end_char-1);

                        //modify NFA_tran
                        auto start_char=DFA_tran.stop.to_UTF32();
                        NFA_tran.start=code_point(start_char+1);
                    }
                    else if(DFA_tran.in_span(NFA_tran.stop) and not DFA_tran.in_span(NFA_tran.start)) //end of new transition is at begining of old transtion
                    {
                        //create the new transition
                        NFA_transition new_tran(DFA_tran.start, NFA_tran.stop, DFA_tran.new_states); //new transition, with DFA states
                        new_tran.new_states.insert(new_tran.new_states.end(), NFA_tran.new_states.begin(), NFA_tran.new_states.end()); //add NFA states
                        new_DFA_transitions.push_back(new_tran);

                        //modify NFA_tran
                        auto end_char=DFA_tran.start.to_UTF32();
                        NFA_tran.stop=code_point(end_char-1);

                        //modify DFA_tran
                        auto start_char=NFA_tran.stop.to_UTF32();
                        DFA_tran.start=code_point(start_char+1);
                    }
                    else if(DFA_tran.in_span(NFA_tran.stop) and DFA_tran.in_span(NFA_tran.start)) //new transition is in old transition
                    {
                        //append DFA_tran states to NFA_tran
                        NFA_tran.new_states.insert(NFA_tran.new_states.end(), DFA_tran.new_states.begin(),  DFA_tran.new_states.end());
                        auto NFA_start=NFA_tran.start.to_UTF32();
                        auto NFA_end=NFA_tran.stop.to_UTF32();

                        //new_transition
                        NFA_transition new_tran(code_point(NFA_end+1), DFA_tran.stop, DFA_tran.new_states); //new transition, with DFA states
                        new_DFA_transitions.push_back(new_tran);

                        //modify DFA_tran
                        DFA_tran.stop=code_point(NFA_start-1);

                    }
                    else if(NFA_tran.in_span(DFA_tran.stop) and NFA_tran.in_span(DFA_tran.start)) //old transition is in new transition
                    {
                        //append NFA_tran states to DFA_tran
                        DFA_tran.new_states.insert(DFA_tran.new_states.end(), NFA_tran.new_states.begin(),  NFA_tran.new_states.end());
                        auto DFA_start=DFA_tran.start.to_UTF32();
                        auto DFA_end=DFA_tran.stop.to_UTF32();

                        //new_transition
                        NFA_transition new_tran(code_point(DFA_end+1), NFA_tran.stop, NFA_tran.new_states); //new transition, with DFA states
                        new_DFA_transitions.push_back(new_tran);

                        //modify NFA_tran
                        NFA_tran.stop=code_point(DFA_start-1);
                    }
                    else
                    {
                        //need to throw a general exception
                        throw general_exception("algorithm error");
                    }
                }
                //insert the new transitions
                DFA_transitions.insert(DFA_transitions.end(), new_DFA_transitions.begin(), new_DFA_transitions.end());
                if(add_NFA_tran) DFA_transitions.push_back(NFA_tran);
            }
        }

        ////now, DFA_transitions is a set of non-overlaping transitions to sets of NFA states

        //find accepting info
        int accepting_info=-1;
        for(unsigned int NFA_ste_index : *dfa_of_nfa_state)
        {
            auto NFA_ste=*next(NFA_states.begin(),NFA_ste_index); //iterator to the NFA_state
            if(NFA_ste->accepting_info!=-1)
            {
                if(accepting_info==-1)
                {
                    accepting_info=NFA_ste->accepting_info;
                }
                else if( NFA_ste->accepting_info<accepting_info ) //if accepting_info is already set, then check precedance
                {
                    accepting_info=NFA_ste->accepting_info;
                }
            }
        }

        shared_ptr<DFA_state> new_DFA_state(new DFA_state(accepting_info));
        //loop over each NFA_transition, making the appropriate DFA states and transitions
        for(auto& DFA_tran : DFA_transitions)
        {
            NFA_IN_DFA_TYPE current_transition_state( new list<unsigned int>(DFA_tran.new_states));
            epsilon_closure(current_transition_state);
            uint new_DFA_state_index=find_state(current_DFA_states, current_transition_state  );

            //need to see if the new transition can be appended onto an old transition
            bool found=false;
            for(auto& old_tran : new_DFA_state->transitions)
            {
                if(old_tran.new_state==new_DFA_state_index) //if the transitions have the same final state
                {
                    if( DFA_tran.start.to_UTF32() == (old_tran.stop.to_UTF32()+1)) //new transition follows old transition
                    {
                        old_tran.stop=DFA_tran.stop;
                        found=true;
                    }
                    else if( DFA_tran.stop.to_UTF32() == (old_tran.start.to_UTF32()-1) ) //new transition is before old transition
                    {
                        old_tran.start=DFA_tran.start;
                        found=true;
                    }
                }
            }
            if(not found)
            {
                new_DFA_state->add_transition(DFA_tran.start, DFA_tran.stop, new_DFA_state_index);
            }
        }

        return_DFA_states.push_back(new_DFA_state);
    }
    return return_DFA_states;
}

DFA_automation csu::compile_regex_DFA(std::list<utf8_string> patterns)
{
    list< shared_ptr<NFA_state> > total_NFA;
    shared_ptr<NFA_state> first_state(new NFA_state);
    total_NFA.push_back(first_state);

    int pattern_number=0;
    for(auto& pattern : patterns)
    {
        uint chars_counted=0;
        auto regex_tree=parse_regex(pattern, chars_counted);
        if( chars_counted< pattern.get_length() )
            throw gen_exception("full regex cannot be parsed");
        if( not regex_tree)
            throw gen_exception("could not parse regex");

        //get new states, make end state as accepting
        list< shared_ptr<NFA_state> > new_states=regex_tree->get_NFA();
        (*(++new_states.begin()))->accepting_info=pattern_number;
        pattern_number+=1;

        //add to total NFA
        increment_states(new_states, total_NFA.size());
        first_state->add_transition(NFA_state::epsilon, total_NFA.size());
        total_NFA.insert(total_NFA.end(), new_states.begin(), new_states.end());
    }

    auto DFA_states=NFA_to_DFA(total_NFA);

    //print out states before minimization?
    DFA_states=DFA_minimization(DFA_states);
    //print states after minimization?

    return DFA_automation(DFA_states);
}

list< shared_ptr<DFA_state> > csu::DFA_minimization(const list< shared_ptr<DFA_state> >& _DFA_states)
{
    class state_index//a class used to store a DFA_state, and its' index simulaniously
    {
        public:
        shared_ptr<DFA_state> state;
        uint index;

        state_index(shared_ptr<DFA_state> _state, uint _index)
        {
            state=_state;
            index=_index;
        }
    };

    list< list<state_index> > partitions;

    //sort each state into partitions, according to accepting_state
    uint state_i=0;
    for(auto state : _DFA_states)
    {
        bool found=false;
        for(auto& group : partitions)
        {
            if(group.begin()->state->accepting_info == state->accepting_info)
            {
                found=true;
                group.push_back(state_index(state, state_i));
                break;
            }
        }

        if(not found)
        {
            partitions.push_back(list<state_index>({ state_index(state, state_i) }));
        }
        state_i++;
    }

    //break each partition into smaller partitions
    //two states are distinguishable if there is an input that leads to a different state
    bool continue_partitioning=true;
    while(continue_partitioning)
    {
        list<list<state_index> > new_partitions;
        for(auto& group : partitions) //partition group, is a list of state_index
        {
            list<list<state_index> > new_partitions_of_group;//the new partitions of the group

            for(auto& group_state_index : group) //place each state in group into a partition in new_partitions_of_group
            {
                bool found=false;

                for(auto& new_group : new_partitions_of_group)//search each of the new partitions
                {
                    if(not group_state_index.state->distinguishable(new_group.begin()->state) )
                    {
                        found=true;
                        new_group.push_back(group_state_index);
                        break;
                    }
                }

                if(not found)
                {
                    new_partitions_of_group.push_back( list<state_index>( { group_state_index } ));
                }
            }

            new_partitions.insert(new_partitions.end(), new_partitions_of_group.begin(), new_partitions_of_group.end()); //place the new_partitions_of_group into all new_partitions
        }

        //check continue partition
        if(new_partitions.size() == partitions.size()) //then no new groups were added
        {
            continue_partitioning=false;
        }
        else
        {
            partitions.swap(new_partitions); //place the new partitions back into partitions, and do it again!!
        }
    }

    //choose a representitive for each group
    list<shared_ptr<DFA_state> >  new_DFA;
    list< list< uint > >  spans; //a list of uints. Each int is a state that is covered by the associated group
    for(auto& group : partitions)
    {
        new_DFA.push_back( group.begin()->state );

        list< uint > states_in_group;
        for(auto& group_state_index : group)
        {
            states_in_group.push_back(group_state_index.index);
        }
        spans.push_back(states_in_group); //probably really slow. Oh Well...
    }

    //now set each transition in each state to point to the correct group
    for(auto& state : new_DFA)
    {
        for(auto& state_tran : state->transitions)
        {
            uint group_num=0;
            for(auto& states_in_group : spans)
            {
                bool found=false;
                for(uint old_state_index : states_in_group)
                {
                    if(state_tran.new_state==old_state_index)
                    {
                        state_tran.new_state=group_num;
                        found=true;
                        break;
                    }
                }
                if(found) break;
                group_num++;
            }
        }
    }

    //remove dead state?  Shouldn't be a dead state with our methods
    return new_DFA;
}

//DFA transition
DFA_transition::DFA_transition(const code_point& _start, const code_point& _stop)
{
    start=_start;
    stop=_stop;
    new_state=0;
    if(start>stop)
    {
        throw gen_exception("start needs to be less than or equal to stop");
    }
}

DFA_transition::DFA_transition(const code_point& _start, const code_point& _stop, unsigned int _new_state)
{
    start=_start;
    stop=_stop;
    new_state=_new_state;
    if(start>stop)
    {
        throw gen_exception("start needs to be less than or equal to stop");
    }
}

bool DFA_transition::in_span(const code_point& val) const
{
    return (val<stop and val>start) or val==start or val==stop;
}

bool DFA_transition::is_lesser(const code_point& val) const
{
    return val<start;
}

bool DFA_transition::is_greater(const code_point& val) const
{
    return val>stop;
}

bool DFA_transition::equivalent(const code_point& _start, const code_point& _stop ) const
{
    return start==_start and stop==_stop;
}

//DFA state
DFA_state::DFA_state()
{
    accepting_info=-1;
}

DFA_state::DFA_state(int _accepting_info)
{
    accepting_info=_accepting_info;
}

void DFA_state::add_transition(const code_point& _start, const code_point& _stop, unsigned int new_state)
{
    if(get_transition(_start)!=-1)
    {
        throw gen_exception("the new transition cannot be in any other transition");
    }
    if(get_transition(_stop)!=-1)
    {
        throw gen_exception("the new transition cannot be in any other transition");
    }

    transitions.emplace(transitions.end(), _start, _stop, new_state);
}

int DFA_state::get_transition(const code_point& val)
{
    for(auto& tran : transitions)
    {
        if(tran.in_span(val))
        {
            return tran.new_state;
        }
    }
    return -1;
}

void DFA_state::print()
{
    for(auto& tran : transitions)
    {
        cout<<"from "<<tran.start<<" to "<<tran.stop<<" transition to "<<tran.new_state<<endl;
    }
    cout<<"   accepting: "<<accepting_info<<endl;
}

bool DFA_state::distinguishable(std::shared_ptr<DFA_state> LHS)
{
    for(auto& this_tran : transitions)
    {
        bool found_same_transition=false;
        for(auto& LHS_tran : LHS->transitions)
        {
            if(LHS_tran.new_state==this_tran.new_state and LHS_tran.start==this_tran.start and LHS_tran.stop==this_tran.stop)
            {
                found_same_transition=true;
                break;
            }
        }
        if(not found_same_transition) return true;
    }
    return false;
}

//DFA automation

DFA_automation::DFA_automation(list<shared_ptr<DFA_state>>& _DFA_states)
{
    DFA_states.insert(DFA_states.begin(), _DFA_states.begin(), _DFA_states.end());
}

void DFA_automation::print_states()
//prints the states to std::out
{
    for(uint i=0; i<DFA_states.size(); i++)
    {
        cout<<"state "<<i<<endl;
        cout<<"    ";
        DFA_states[i]->print();
    }
}

pair<int, int> DFA_automation::run(const utf8_string& input, bool print_status)
//tests the input string with the states. Returns the length that was accepted, and the accepting_info of the accepting state. is 0,-1 if not accepting
{
    if(print_status)
    {
        cout<<"starting state: 0"<<endl;
    }
    uint state_index=0;
    auto state=DFA_states[state_index];
    uint chars_read=0;
    int last_accepting_chars=0;
    int last_accepting_state=-1;
    int last_accepting_info=-1;
    uint Nchars=input.get_length();

    for(uint i=0; i<Nchars; i++)
    {
        if(state->accepting_info != -1)
        {
            last_accepting_chars=chars_read;
            last_accepting_state=state_index;
            last_accepting_info=state->accepting_info;

            if(print_status)
            {
                cout<<"accepted state: "<<last_accepting_state<<endl;
            }
        }

        int new_state=state->get_transition(input[i]);
        if(new_state==-1)
        {
            cout<<"no transition on "<<input[i]<<". End iteration"<<endl;
            break;
        }

        state_index=new_state;
        state=DFA_states[state_index];
        chars_read++;
        cout<<"on "<<input[i]<<" transition to "<<state_index<<endl;
    }

    if(state->accepting_info != -1)
    {
        last_accepting_chars=chars_read;
        last_accepting_state=state_index;
        last_accepting_info=state->accepting_info;

        if(print_status)
        {
            cout<<"accepted state: "<<last_accepting_state<<endl;
        }
    }

    return make_pair(last_accepting_chars, last_accepting_info);
}
