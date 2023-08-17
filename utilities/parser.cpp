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

#include <algorithm>
#include "parser.hpp"
#include "logger.hpp"
#include "serialize.hpp"

using namespace csu;
using namespace std;

//parser return action

//lexer_function_generic
lexer_function_generic::lexer_function_generic(unsigned int _ID, bool _return_data)
{
    terminal_ID=_ID;
    return_data=_return_data;
}

token_data lexer_function_generic::operator()( utf8_string& data, location_span& loc, lexer<token_data>* lex)
{
    lex->continue_lexing(not return_data);
    dyn_holder new_data(data);
    token_data ret(terminal_ID, new_data, loc);
    return ret;
}


//begin token class
ostream& csu::operator<<(ostream& os, const token& dt)
{
    os<<dt.name;
    return os;
}
//end token class

//terminal
terminal::terminal(utf8_string _name, unsigned int ID, lexer_generator<token_data>* _lex_gen)
{
    name=_name;
    is_terminal=true;
    token_ID=ID;
    lex_gen=_lex_gen;
}

terminal::terminal(utf8_string _name)
{
    name=_name;
    is_terminal=true;
    token_ID=0;
}

void terminal::add_pattern(utf8_string pattern, bool return_data)
{
    lexer_function_generic new_lex_func(token_ID, return_data);
    lexer_generator<token_data>::lex_func_t lex_func(new_lex_func);
    lex_gen->add_pattern(pattern, lex_func);
}
//end terminal class

//start token_string_converter class
token_string_converter::token_string_converter(const char* _data)
{
    data=token_ptr(new terminal(_data));
}

token_string_converter::token_string_converter(token_ptr _data)
{
    data=_data;
}

token_string_converter::token_string_converter(terminal_ptr _data)
{
    data=_data;
}

token_string_converter::token_string_converter( non_terminal_ptr _data)
{
    data=_data;
}


token_string_converter::operator token_ptr() const
{
    return data;
}

//end token_string_converter

//non_terminal
non_terminal::non_terminal(utf8_string _name, unsigned int ID, parser_generator* _par_gen)
{
    name=_name;
    par_gen=_par_gen;
    is_terminal=false;
    token_ID=ID;
}

production& non_terminal::add_production(std::initializer_list<token_string_converter> tokens)
{
    vector<token_ptr> _tokens;
    _tokens.reserve(tokens.size());
    //check for unknown tokens
    for(token_string_converter _token : tokens)
    {
        token_ptr new_token(_token);

        if(new_token->token_ID==0)//found an unknown token
        {
            par_gen->resolve_unknown_terminal(new_token);//make it known
        }

        _tokens.push_back(new_token);
    }
    //make the new production
    std::shared_ptr< production > new_production(new production(this, _tokens));
    productions.push_back(new_production);
    return *new_production.get();
}
//end non_terminal class

//start token_data class

token_data::token_data(unsigned int _ID, dyn_holder _data, location_span _span)
{
    token_ID=_ID;
    t_data=_data;
    span=_span;
}

location_span token_data::loc(){return span;}
unsigned int token_data::get_ID(){return token_ID;}

//end token_data class

// production
unsigned int production::next_production_ID=0;
unsigned int production::next_precedence_level=1;
production::production(non_terminal* _L_val, std::vector<token_ptr>& _tokens)
{
    L_val=_L_val;
    tokens=_tokens;
    id=next_production_ID;
    next_production_ID++;
    assoc=NONE;
    precedence=0;
    action = nullptr;
}

production& production::set_return_action(int i)
{
    parser_function_ptr new_parser_func(new parser_return_action(i));
    action=new_parser_func;
    return *this;
}

production& production::set_associativity(association _assoc)
{
    assoc=_assoc;
    return *this;
}

production& production::set_associativity(utf8_string _assoc)
{
    if(_assoc=="NONE")
    {
        assoc=NONE;
    }
    else if(_assoc=="LEFT")
    {
        assoc=LEFT;
    }
    else if(_assoc=="RIGHT")
    {
        assoc=RIGHT;
    }
    else
    {
        throw gen_exception("unknown production association");
    }
    return *this;
}

bool production::is_left_associative()
{
    return assoc==LEFT;
}

bool production::is_right_associative()
{
    return assoc==RIGHT;
}

production& production::set_precedence()
{
    precedence=next_precedence_level;
    next_precedence_level++;
    return *this;
}

std::shared_ptr<production_info> production::get_info()
{
    return std::shared_ptr<production_info>(new production_info(L_val->token_ID, L_val->name, tokens.size(), action));
}

ostream& csu::operator<<(std::ostream& os, const production& dt)
{
     os<<dt.L_val->name<<" -> ";
     for(auto _token : dt.tokens)
     {
         os<<" "<<_token->name;
     }
     return os;
}

//end production class

//start production_info class
production_info::production_info(unsigned int _L_val_ID, utf8_string& _L_val_name, unsigned int _num_tokens, parser_function_ptr _action)
{
    L_val_ID=_L_val_ID;
    L_val_name=_L_val_name;
    num_tokens=_num_tokens;
    action=_action;
}

ostream& csu::operator<<(ostream& os, const production_info& dt)
{
    os<<dt.L_val_name<<":"<<dt.num_tokens;
    return os;
}

//end production_info

// start item class
item::item(std::shared_ptr<production> _prod, unsigned int _loc)
{
    prod=_prod;
    loc=_loc;
}

item::item(std::shared_ptr<production> _prod, unsigned int _loc, token_ptr _lookahead)
{
    prod=_prod;
    loc=_loc;
    lookahead=_lookahead;
}

bool item::is_kernal( non_terminal_ptr augmented_start)
{
    if(loc!=0) return true;

    if( augmented_start->token_ID == prod->L_val->token_ID )
    {
        return true;
    }
    return false;
}

bool item::is_LR0()
{
    return not lookahead;
}

vector<token_ptr> item::get_postTokens()
{
    return vector<token_ptr>( prod->tokens.begin()+loc, prod->tokens.end());
}

item item::copy()
{
    return item(prod, loc, lookahead);
}

item item::copy(token_ptr new_lookahead)
{
    return item(prod, loc, new_lookahead);
}

bool item::operator==(const item& RHS) const
{
    if(not lookahead)
    {
        if(not RHS.lookahead)
            return prod->id==RHS.prod->id  and loc==RHS.loc;
        else
            return false;
    }
    else
    {
        if(not RHS.lookahead)
            return false;
        else
            return prod->id==RHS.prod->id  and loc==RHS.loc  and lookahead->token_ID==RHS.lookahead->token_ID;
    }
}

ostream& csu::operator<<(ostream& os, const item& dt)
{
    os<<dt.prod->L_val->name<<"->";
    for(unsigned int i=0; i<dt.prod->tokens.size(); i++)
    {
        if(i==dt.loc)
        {
            os<<'.';
        }
        os<<(dt.prod->tokens[i]->name)<<" ";
    }

    if(dt.loc==dt.prod->tokens.size()) os<<".";

    if(dt.lookahead) os<<'['<<dt.lookahead->name<<']';
    return os;
}

bool csu::operator<(const item &LHS, const item &RHS)
{
    if(LHS.prod<RHS.prod) return true;
    if(LHS.prod>RHS.prod) return false;

    if(LHS.loc<RHS.loc) return true;
    if(LHS.loc>RHS.loc) return false;

    if(LHS.lookahead<RHS.lookahead) return true;
    if(LHS.lookahead>RHS.lookahead) return false;

    //they are equal
    return false;
}

//end item class

// start item_set class
item_set::item_set()
{
    id=0;
}

item_set::item_set(unsigned int _id)
{
    id=_id;
}

item_set::item_set(unsigned int _id, item first_item)
{
    id=_id;
    items.push_back(first_item);
}

void item_set::append(item new_item)
{
    items.push_back(new_item);
}

void item_set::add_goto(token_ptr _token, std::shared_ptr<item_set> goto_set)
{
    goto_table[_token->token_ID]=goto_set;
}

shared_ptr<item_set> item_set::get_goto(token_ptr _token)
{
    return goto_table[_token->token_ID];
}

bool item_set::operator==(item_set& RHS)
{
    if(RHS.size()!=size())
    {
        return false;
    }

    for(auto this_iter=items.begin(), RHS_iter=RHS.items.begin(), this_end=items.end(); this_iter!=this_end; ++this_iter)
    {
        if(not ((*this_iter)==(*RHS_iter)))
        {
            return false;
        }
        ++RHS_iter;
    }

    return true;
}

item_set::iterator item_set::begin()
{
    return items.begin();
}

item_set::iterator item_set::end()
{
    return items.end();
}

unsigned int item_set::size()
{
    return items.size();
}

bool item_set::has_item(item& RHS)
{
    for(auto& has_item : items)
    {
        if(has_item==RHS)
        {
            return true;
        }
    }
    return false;
}

ostream& csu::operator<<(std::ostream& os, const item_set& dt)
{
    os<<"SET: "<<dt.id<<endl;
    for(auto& has_item : dt.items )
    {
        os<<" "<<has_item<<endl;
    }
    os<<"END SET"<<endl;
    return os;
}

//end item_set

//start propagation_table
propagation_table::table_iterator::table_iterator(pair<propagation_table::table_iterator::iterator, propagation_table::table_iterator::iterator> _iters)
{
    iters=_iters;
}

propagation_table::table_iterator::iterator& propagation_table::table_iterator::begin()
{
    return iters.first;
}

propagation_table::table_iterator::iterator& propagation_table::table_iterator::end()
{
    return iters.second;
}

void propagation_table::add_propagation(item_set_ptr from_set, item& from_item, item_set_ptr to_set, item& to_item)
{

    FROM_T from_pair(from_set->id, from_item);
    TO_T to_pair(to_set, to_item);

    table.insert( make_pair(from_pair, to_pair));
}

propagation_table::table_iterator propagation_table::get_propagation(item_set_ptr from_set, item& from_item)
{
    FROM_T from_pair(from_set->id, from_item);
    table_iterator ret( table.equal_range(from_pair) );
    return ret;
}

void propagation_table::print()
{
    for(auto& FROM_TO_PAIR : table)
    {
        cout<<"propagate from set "<<FROM_TO_PAIR.first.first<<" item: "<<FROM_TO_PAIR.first.second<<endl;
        cout<<"  to set "<<FROM_TO_PAIR.second.first->id<<" item: "<<FROM_TO_PAIR.second.second<<endl<<endl;
    }
}

//end propagation_table


//start parser_action
parser_action::parser_action(action_type _todo, unsigned int _data)
{
    action_todo=_todo;
    data=_data;
}

parser_action::parser_action()
{
    action_todo=NONE;
    data=0;
}

parser_action parser_action::get_error()
{
    return parser_action(ERROR, 0);
}

parser_action parser_action::get_accept()
{
    return parser_action(ACCEPT, 0);
}

parser_action parser_action::get_shift(unsigned int state)
{
    return parser_action(SHIFT, state);
}

parser_action parser_action::get_reduce(unsigned int production_ID)
{
    return parser_action(REDUCE, production_ID);
}

parser_action parser_action::get_none()
{
    return parser_action(NONE, 0);
}

bool parser_action::is_error() const {return action_todo==ERROR;}
bool parser_action::is_shift() const {return action_todo==SHIFT;}
bool parser_action::is_accept() const {return action_todo==ACCEPT;}
bool parser_action::is_reduce() const {return action_todo==REDUCE;}
bool parser_action::is_none() const {return action_todo==NONE;}

unsigned int parser_action::get_data() const
{
    return data;
}

bool parser_action::operator==(parser_action& RHS)
{
    return action_todo==RHS.action_todo and data==RHS.data;
}

ostream& csu::operator<<(ostream& os, const parser_action& dt)
{
    if(dt.is_accept())
    {
        os<<"accept";
    }
    else if(dt.is_shift())
    {
        os<<"shift to "<<dt.get_data();
    }
    else if(dt.is_reduce())
    {
        os<<"reduce by "<<dt.get_data();
    }
    else if(dt.is_error())
    {
        os<<"error";
    }
    else
    {
        os<<"no action";
    }
    return os;
}

void parser_action::binary_write(std::ostream& output)
{
    unsigned int action_num=0;
    switch( action_todo )
    {
        case ERROR: action_num=1;
        break;
        case SHIFT: action_num=2;
        break;
        case ACCEPT: action_num=3;
        break;
        case REDUCE: action_num=4;
        break;
        case NONE: action_num=5;
        break;
    }
    ::binary_write(output, action_num);
    ::binary_write(output, data);
}


void parser_action::binary_read(std::istream& input)
{
    unsigned int action_num=0;
    ::binary_read(input, action_num);
    ::binary_read(input, data);

    switch(action_num)
    {
        case 1: action_todo=ERROR;
        break;
        case 2: action_todo=SHIFT;
        break;
        case 3: action_todo=ACCEPT;
        break;
        case 4: action_todo=REDUCE;
        break;
        case 5: action_todo=NONE;
        break;
    }
}

//end parser_action

//start parser_state class
parser_state::parser_state()
{
}

void parser_state::add_goto(unsigned int _token_ID, unsigned int _state)
{
    GOTO[_token_ID]=_state;
}

void parser_state::add_action(unsigned int _non_term, parser_action _action)
{
    ACTION[_non_term]=_action;
}

void parser_state::set_default(parser_action _default)
{
    default_action=_default;
}

unsigned int parser_state::get_goto(unsigned int _token_ID)
{
    //do we need to check to see if _token_ID exists?
    return GOTO[_token_ID];
}

parser_action parser_state::get_action(unsigned int non_term)
{
    auto iter=ACTION.find(non_term);
    if(iter==ACTION.end())
    {
        return default_action;
    }
    else
    {
        return iter->second;
    }
}
//end parser_state

//parser_generator
parser_generator::parser_generator(utf8_string _parser_table_file_name, utf8_string _lexer_table_file_name)
{
    parser_table_file_name =_parser_table_file_name;
    parser_table_generated = false;

    lex_gen=std::shared_ptr<lexer_generator<token_data> >(new lexer_generator<token_data> (_lexer_table_file_name) );

    ERROR_terminal=terminal_ptr(new terminal("ERROR", error_token_id, lex_gen.get()));
    next_token_num=error_token_id+1;
    terminals["ERROR"] = ERROR_terminal;

    EOF_terminal=terminal_ptr(new terminal("EOF", next_token_num, lex_gen.get()));
    next_token_num++;
    terminals["EOF"] = EOF_terminal;

    lex_gen->set_EOF_action(lexer_function_generic(EOF_terminal->token_ID, true));
}

terminal_ptr parser_generator::get_EOF_terminal()
{
    return EOF_terminal;
}

terminal_ptr parser_generator::get_ERROR_terminal()
{
    return ERROR_terminal;
}

std::shared_ptr<lexer_generator<token_data> > parser_generator::get_lexer_generator()
{
    return lex_gen;
}

terminal_ptr parser_generator::new_terminal(utf8_string name)
{
    if(terminals.find(name)!=terminals.end())
    {
        throw gen_exception("cannot make a new terminal with the same name as another terminal: ", name);
    }
    if(non_terminals.find(name)!=non_terminals.end())
    {
        throw gen_exception("cannot make a new terminal with the same name as a non-terminal: ", name);
    }

    terminal_ptr new_term(new terminal(name, next_token_num, lex_gen.get()));
    next_token_num++;
    terminals[name]=new_term;
    return new_term;
}

non_terminal_ptr parser_generator::new_nonterminal(utf8_string name)
{
    if(terminals.find(name)!=terminals.end())
    {
        throw gen_exception("cannot make a new non-terminal with the same name as a terminal: ", name);
    }
    if(non_terminals.find(name)!=non_terminals.end())
    {
        throw gen_exception("cannot make a new non-terminal with the same name as another non-terminal: ", name);
    }

    non_terminal_ptr new_nonterm(new non_terminal(name, next_token_num, this));
    next_token_num++;
    if(not start_nonterm)
    //set the first non_term to be the start non_term.
    //this can be overridden
    {
        start_nonterm=new_nonterm;
    }
    non_terminals[name]=new_nonterm;
    return new_nonterm;
}

void parser_generator::set_start_nonterm(non_terminal_ptr _start_nonterm)
{
    start_nonterm=_start_nonterm;
}

void parser_generator::print_grammer()
{
    for(auto name_nonterm_pair : non_terminals)
    {
        cout<<name_nonterm_pair.first;
        string prefix("");
        for(int i=0; i<name_nonterm_pair.first.get_length(); i++) prefix+=string(" ");
        prefix+=string("|");

        for(auto prod : name_nonterm_pair.second->productions)
        {
            cout<<prefix;
            for(auto _token : prod->tokens)
            {
                cout<<_token->name;
            }
            cout<<endl;
        }
        cout<<endl;
    }
}

void parser_generator::reset_table()
{
    parser_table_generated=false;
    term_map.reset();
    production_information.reset();
    state_table.reset();
}

shared_ptr<parser> parser_generator::get_parser(bool do_file_IO)
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

    auto new_lex = lex_gen->get_lexer(do_file_IO);
    return make_shared< parser >(new_lex, term_map, production_information, state_table);
}

void parser_generator::resolve_unknown_terminal(token_ptr _new_terminal) //this is for when the terminal is referanced by a string
{
    if(non_terminals.find(_new_terminal->name)!=non_terminals.end())
    {
        throw gen_exception("cannot referance a non-terminal by string");
    }

    auto term_iter=terminals.find(_new_terminal->name);
    if(term_iter==terminals.end())
    {
        //it is a new terminal
        _new_terminal->token_ID=next_token_num;
        next_token_num++;
        lex_gen->add_pattern(_new_terminal->name, lexer_function_generic(_new_terminal->token_ID, true));
        terminals[_new_terminal->name]=dynamic_pointer_cast<terminal>(_new_terminal);
    }
    else
    {
        //it is a known terminal
        _new_terminal->token_ID=(*term_iter).second->token_ID;
    }
}

void parser_generator::load_from_file()
{
    //check if file exists
    std::ifstream fin(parser_table_file_name.to_cpp_string(), std::ios_base::binary);
    if( not fin) return;

    //make term map
    term_map= shared_ptr< map<unsigned int, utf8_string> >( new map<unsigned int, utf8_string>());
    for(auto& name_term_pair : terminals)
    {
        (*term_map)[name_term_pair.second->token_ID]=name_term_pair.first;
    }

    //count number of productions
    unsigned int num_productions=0;
    for(auto& name_nonterm_pair : non_terminals)
    {
        num_productions+=name_nonterm_pair.second->productions.size();
    }

    //make production information
    production_information=shared_ptr< vector<production_info_ptr> >( new vector<production_info_ptr>() );
    production_information->resize(num_productions);
    for(auto name_nonterm_pair : non_terminals)
    {
        for(auto prod : name_nonterm_pair.second->productions)
        {
            (*production_information)[prod->id]=prod->get_info();
        }
    }

    //read state_table
    unsigned int num_states=0;
    binary_read(fin, num_states);

    state_table=shared_ptr< vector<parser_state> >( new vector<parser_state>() );
    state_table->resize(num_states);
    for(unsigned int state_i=0; state_i<num_states; state_i++)
    {
        auto& current_state=(*state_table)[state_i];

        //read GOTO table
        unsigned int goto_size=0;
        binary_read(fin, goto_size);
        for(unsigned int goto_i=0; goto_i<goto_size; goto_i++)
        {
            unsigned int first=0;
            unsigned int second=0;
            binary_read(fin, first);
            binary_read(fin, second);
            current_state.GOTO[first]=second;
        }

        //read action_table
        unsigned int action_size=0;
        binary_read(fin, action_size);
        for(unsigned int action_i=0; action_i<action_size; action_i++)
        {
            unsigned int state=0;
            binary_read(fin, state);
            current_state.ACTION[state].binary_read(fin);
        }

        //read default action
        current_state.default_action.binary_read(fin);
    }
    parser_table_generated=true;
}

void parser_generator::load_to_file()
{
    if(not parser_table_generated) return;
    std::ofstream fout(parser_table_file_name.to_cpp_string(), std::ios_base::binary);

    //save state_table to the file
    unsigned int num_states = state_table->size();
    binary_write(fout, num_states);
    for(parser_state& state : *state_table)
    {
        //save the GOTO table
        unsigned int goto_size = state.GOTO.size();
        binary_write(fout, goto_size);
        for(auto& state_pair : state.GOTO)
        {
            unsigned int first = state_pair.first;
            unsigned int second = state_pair.second;
            binary_write(fout, first);
            binary_write(fout, second);
        }
        //save the action table
        unsigned int action_size = state.ACTION.size();
        binary_write(fout, action_size);
        for(auto& stateaction_pair : state.ACTION)
        {
            unsigned int first = stateaction_pair.first;
            binary_write(fout, first);
            stateaction_pair.second.binary_write(fout);
        }
        //save teh default action
        state.default_action.binary_write(fout);
    }
}

list<token_ptr> parser_generator::first(list<token_ptr>& tokens)
{
    list<token_ptr> ret;
    list<unsigned int> empty;
    bool has_epsilon=false;
    bool all_previous_have_epsilon=true;

    for(auto _toke : tokens)
    {
        bool token_has_epsilon=false;
        auto token_firsts=first_single(_toke, empty, token_has_epsilon);
        ret.insert(ret.end(), token_firsts.begin(), token_firsts.end());
        has_epsilon=has_epsilon or token_has_epsilon;
        if(not token_has_epsilon)
        {
            all_previous_have_epsilon=false;
            break;
        }
    }

    if(all_previous_have_epsilon)
    {
        has_epsilon=true;
    }

    //remove duplicates
    for(auto iter=ret.begin(), end=ret.end(); iter!=end; ++iter)
    {
        auto iter_copy=iter;
        while(iter_copy!=end)
        {
            ++iter_copy;
            if(iter_copy==end) break;
            else if((*iter_copy)->token_ID==(*iter)->token_ID)
            {
                ret.erase(iter_copy--);
                ++iter_copy;
            }
        }
    }

    //return
    //if(has_epsilon){ ret.push_back(EPSILON_terminal);}
    return ret;
}

list<token_ptr> parser_generator::first_single(token_ptr _token, list<unsigned int>& exclusion_productions, bool& output_contains_epsilon)
{
    list<token_ptr> ret;

    if(_token->is_terminal)
    {
        output_contains_epsilon=false;
        ret.push_back(_token);
        return ret;
    }
    else//not a terminal
    {
        output_contains_epsilon=false;
        non_terminal_ptr non_term( dynamic_pointer_cast<non_terminal>(_token) );
        for( auto prod : non_term->productions )
        {
            if( find(exclusion_productions.begin(), exclusion_productions.end(), prod->id) != exclusion_productions.end() )
            {
                //if production is in exclusion_productions, then skip it
                continue;
            }

            exclusion_productions.push_back(prod->id);

            bool production_is_all_epsilons=true;
            for(auto p_token : prod->tokens)
            {
                list<token_ptr> p_token_first;
                bool p_token_epsilon;

                if(p_token->token_ID==_token->token_ID)
                {
                    list<unsigned int> new_exclusions=exclusion_productions;
                    //new_exclusions.push_back(prod->id);
                    p_token_first=first_single(p_token, new_exclusions, p_token_epsilon);
                }
                else
                {
                    p_token_first=first_single(p_token, exclusion_productions, p_token_epsilon);
                }

                if(production_is_all_epsilons)
                {
                    ret.insert(ret.end(), p_token_first.begin(), p_token_first.end() );
                    output_contains_epsilon=output_contains_epsilon or p_token_epsilon;
                }

                if(not p_token_epsilon)
                {
                    production_is_all_epsilons=false;
                    break;
                }
            }

            if(production_is_all_epsilons)
            {
                output_contains_epsilon=true;
            }
        }
        return ret;
    }
}

void parser_generator::closure_LR1(item_set_ptr input_set)
{
    //this algorithm may need improvement
    bool items_added=true;
    while( items_added )
    {
        items_added=false;
        for(auto& input_item : *input_set)
        {
            auto post_tokens=input_item.get_postTokens();
            auto post_tokens_iterator=post_tokens.begin();
            if(post_tokens.size()>0 and not (*post_tokens_iterator)->is_terminal)
            {
                non_terminal_ptr next_token( dynamic_pointer_cast<non_terminal>(*post_tokens_iterator) );
                list<token_ptr> following_tokens(++post_tokens_iterator, post_tokens.end());
                following_tokens.push_back(input_item.lookahead);
                auto first_data=first(following_tokens);
                for(auto prod : next_token->productions )
                {
                    for(auto first_terminal : first_data)
                    {
                        item new_item(prod, 0, first_terminal);
                        if( not input_set->has_item(new_item))
                        {
                            input_set->append(new_item);
                            items_added=true;
                        }
                    }
                }
            }
        }
    }
}

void parser_generator::closure_LR0(item_set_ptr input_set)
{
    bool item_added=true;
    while(item_added)
    {
        item_added=false;
        for(item& input_item : *input_set)
        {
            vector<token_ptr> post_tokens( input_item.get_postTokens() );
            if( post_tokens.size()>0 and not post_tokens[0]->is_terminal)
            {
                non_terminal_ptr next_token= dynamic_pointer_cast<non_terminal>(post_tokens[0]);

                for(auto next_token_production : next_token->productions )
                {
                    item new_item(next_token_production, 0);
                    if(not input_set->has_item(new_item))
                    {
                        input_set->append(new_item);
                        item_added=true;
                    }
                }
            }
        }
    }
}

item_set_ptr parser_generator::goto_LR0(item_set_ptr input_set, token_ptr _token)
{
    item_set_ptr ret( new item_set());
    for(item& input_item : *input_set)
    {
        auto post_tokens=input_item.get_postTokens();
        if(post_tokens.size()>0 and post_tokens[0]->token_ID==_token->token_ID)
        {
            auto new_item=input_item.copy();
            new_item.loc+=1;
            ret->append(new_item);
        }
    }

    closure_LR0(ret);
    return ret;
}

list<item_set_ptr> parser_generator::LR0_itemsets(non_terminal_ptr _start_token)
{
    //use algorithm 4.6.2 to construct LR0_itemsets

    //check if an item_set is in a list
    auto in_list = [](list<item_set_ptr>& chk_list, item_set_ptr set) -> item_set_ptr
    {
        for(auto list_set : chk_list)
        {
            if((*list_set)==(*set))
            {
                return list_set;
            }
        }
        return  item_set_ptr(); //not in the list
    };


    item_set_ptr set_zero(new item_set(0));
    item start_item(*_start_token->productions.begin(), 0);
    set_zero->append(start_item);
    closure_LR0(set_zero);
    list<item_set_ptr> item_sets({set_zero});

    unsigned int next_set_ID=1;
    for(auto set : item_sets)
    {
        for(auto&  name_term_pair : terminals)
        {
            auto term=name_term_pair.second;
            auto new_goto=goto_LR0(set, term);
            if(new_goto->size()==0) continue;

            item_set_ptr _in_list=in_list(item_sets, new_goto);
            if(not _in_list)
            {
                new_goto->id=next_set_ID;
                next_set_ID++;
                set->add_goto(term, new_goto);
                item_sets.push_back(new_goto);
            }
            else
            {
                set->add_goto(term, _in_list);
            }
        }

        for(auto&  name_nonterm_pair : non_terminals)
        {
            auto nonterm=name_nonterm_pair.second;
            auto new_goto=goto_LR0(set, nonterm);
            if(new_goto->size()==0) continue;
            item_set_ptr _in_list=in_list(item_sets, new_goto);
            if(not _in_list)
            {
                new_goto->id=next_set_ID;
                next_set_ID++;
                set->add_goto(nonterm, new_goto);
                item_sets.push_back(new_goto);
            }
            else
            {
                set->add_goto(nonterm, _in_list);
            }
        }
    }
    return item_sets;
}

void parser_generator::generate_parser_table()
{
    cout<<"Generating Parser"<<endl;
    logger log;

    ////augment the grammer////
    if(not start_nonterm)
    {
        throw gen_exception("need at least one production");
    }
    non_terminal_ptr augmented_start= new_nonterminal("start");
    augmented_start->add_production({start_nonterm});

    ////basic accounting////
    //terminal map
    log("terminals:");

    term_map= shared_ptr< map<unsigned int, utf8_string> >( new map<unsigned int, utf8_string>());
    for(auto& name_term_pair : terminals)
    {
        log(name_term_pair.second->token_ID, ": ", name_term_pair.first);
        (*term_map)[name_term_pair.second->token_ID]=name_term_pair.first;
    }
    log();

    //terminal error recovery table
    //not doing this

    //log the nonterms
    log("nonterminals:");
    map<unsigned int, utf8_string> nonterm_map;
    unsigned int num_productions=0;
    for(auto& name_nonterm_pair : non_terminals)
    {
        log(name_nonterm_pair.second->token_ID, ": ", name_nonterm_pair.first);
        nonterm_map[name_nonterm_pair.second->token_ID]=name_nonterm_pair.first;
        num_productions+=name_nonterm_pair.second->productions.size();
    }
    log();

    //set production information
    log("grammer productions:");
    production_information=shared_ptr< vector<production_info_ptr> >( new vector<production_info_ptr>() );
    production_information->resize(num_productions);
    for(auto name_nonterm_pair : non_terminals)
    {
        for(auto prod : name_nonterm_pair.second->productions)
        {
            log(prod->id, ": ", *prod);
            (*production_information)[prod->id]=prod->get_info();
        }
    }
    log();
//cout << 'A' << endl;

    ///// generate the parse table /////
    auto LR0_item_sets=LR0_itemsets(augmented_start);

    //remove non-kernal items
    for( auto set : LR0_item_sets)
    {
        //set->items.erase( remove_if(set->items.begin(), set->items.end(), [&](item& it){return not it.is_kernal(augmented_start);}), set->items.end() );
        set->items.remove_if( [&](item& it){return not it.is_kernal(augmented_start);} );
    }

    //create new LR1 item stes from old LR0 item sets
    vector< item_set_ptr > LR1_item_sets;
    LR1_item_sets.resize(LR0_item_sets.size());
    for(auto LR0_set : LR0_item_sets) //make all the LR1 sets
    {
        LR1_item_sets[LR0_set->id]=item_set_ptr(new item_set(LR0_set->id));
    }
    for(auto LR0_set : LR0_item_sets) //duplicate the goto tables
    {
        auto relavent_LR1_set=LR1_item_sets[LR0_set->id];
        for(auto& tokenID_set_pair : LR0_set->goto_table)
        {
            auto LR1_goto_set=LR1_item_sets[ tokenID_set_pair.second->id ];
            relavent_LR1_set->goto_table[ tokenID_set_pair.first ]=LR1_goto_set;
        }
    }

//cout << "A1" << endl;

    //algorithm 4.62 Generate propagation table and spontanious lookaheads
    propagation_table prop_table;
    terminal_ptr fake_terminal( new terminal("#", next_token_num, lex_gen.get()));
    next_token_num++;
    for(auto set_K : LR0_item_sets)
    {
        for(auto& name_term_pair : terminals)
        {
            auto term_X=name_term_pair.second;
            auto LR0_goto_set=set_K->get_goto(term_X);
            if(not LR0_goto_set) continue;
            auto LR1_set=LR1_item_sets[LR0_goto_set->id];

            for(item& K_item : *set_K)
            {
                item_set_ptr set_J( new item_set(0, K_item.copy(fake_terminal) ) );
                closure_LR1(set_J);
                for(item& J_item : *set_J)
                {
                    auto post_tokens=J_item.get_postTokens();
                    if(post_tokens.size()>0 and (*post_tokens.begin())->token_ID==term_X->token_ID)
                    {
                        auto J_item_copy=J_item.copy();
                        J_item_copy.loc+=1;
                        if(J_item_copy.lookahead->token_ID==fake_terminal->token_ID)
                        {
                            J_item_copy.lookahead.reset();
                            prop_table.add_propagation(set_K, K_item, LR1_set, J_item_copy);
                        }
                        else
                        {
                            if( count(LR1_set->items.begin(), LR1_set->items.end(), J_item_copy)==0 )
                            {
                                LR1_set->append(J_item_copy);
                            }
                        }
                    }
                }
            }
        }
        for(auto& name_nonterm_pair : non_terminals)
        {
            auto nonterm_X=name_nonterm_pair.second;
            auto LR0_goto_set=set_K->get_goto(nonterm_X);
            if(not LR0_goto_set) continue;
            auto LR1_set=LR1_item_sets[LR0_goto_set->id];

            for(item& K_item : *set_K)
            {
                item_set_ptr set_J( new item_set(0, K_item.copy(fake_terminal) ) );
                closure_LR1(set_J);
                for(item& J_item : *set_J)
                {
                    auto post_tokens=J_item.get_postTokens();
                    if(post_tokens.size()>0 and (*post_tokens.begin())->token_ID==nonterm_X->token_ID)
                    {
                        auto J_item_copy=J_item.copy();
                        J_item_copy.loc+=1;
                        if(J_item_copy.lookahead->token_ID==fake_terminal->token_ID)
                        {
                            J_item_copy.lookahead.reset();
                            prop_table.add_propagation(set_K, K_item, LR1_set, J_item_copy);
                        }
                        else
                        {
                            if( count(LR1_set->items.begin(), LR1_set->items.end(), J_item_copy)==0 )
                            {
                                LR1_set->append(J_item_copy);
                            }
                        }
                    }
                }
            }
        }
    }


//cout << "A2" << endl;

    //add EOF to augmented_start production
    for(item& LR0_item : *(*LR0_item_sets.begin()))
    {
        if(LR0_item.prod->L_val->token_ID==augmented_start->token_ID and LR0_item.loc==0)
        {
            item new_item=LR0_item.copy(EOF_terminal);
            LR1_item_sets[0]->append(new_item);
        }
    }


//cout << "A3" << endl;

    //propagate lookahead items to form all LALR(1) items
    bool items_added=true;
    while(items_added)
    {
        items_added=false;
        for(auto LR1set : LR1_item_sets)
        {
            item_set_ptr LR0set=*find_if(LR0_item_sets.begin(), LR0_item_sets.end(), [=](const item_set_ptr A ){return A->id==LR1set->id;});//ouch
            for(item& LR1item : *LR1set)
            {
                item LR0item=LR1item.copy( token_ptr() );
                for(auto& tofrom__pair : prop_table.get_propagation(LR0set, LR0item ) )
                {
                    auto& to_setitem_pair=tofrom__pair.second;
                    item new_to_item=to_setitem_pair.second.copy( LR1item.lookahead );
                    if(not to_setitem_pair.first->has_item(new_to_item) )
                    {
                        to_setitem_pair.first->append(new_to_item);
                        items_added=true;
                    }
                }
            }
        }
    }

//cout << 'B' << endl;

    //closure on each set of items to get the non-kernal items
    for( auto set : LR1_item_sets ) closure_LR1(set);

    //generate table using algorithm 4.56
    state_table=shared_ptr< vector<parser_state> >( new vector<parser_state>() );
    state_table->resize(LR1_item_sets.size());
    item accept_item(*augmented_start->productions.begin(), 1, EOF_terminal);
    bool is_ambiguous=false;
    for( unsigned int state_i=0; state_i<LR1_item_sets.size(); state_i++)
    {
        parser_state& current_state=(*state_table)[state_i];
        auto set=LR1_item_sets[state_i];

        //set the actions
        map<unsigned int, item> action_items;
        for(item& set_item : *set)
        {
            auto post_tokens=set_item.get_postTokens();
            token_ptr action_token;
            parser_action new_action=parser_action::get_none();
            if(set_item==accept_item)
            {
                //accept
                new_action=parser_action::get_accept();
                action_token=EOF_terminal;
            }
            else if( post_tokens.size()==0 and set_item.lookahead->is_terminal )
            {
                //reduce
                new_action=parser_action::get_reduce(set_item.prod->id);
                action_token=set_item.lookahead;
            }
            else if( (*post_tokens.begin())->is_terminal)
            {
                //shift
                auto set_shift_to=set->get_goto( (*post_tokens.begin()) );
                new_action=parser_action::get_shift(set_shift_to->id);
                action_token=(*post_tokens.begin());
            }

            if(not new_action.is_none() ) //the action was set
            {
                bool tmp_ambiguous=false;
                //check for conflicts
                parser_action old_action=current_state.get_action(action_token->token_ID);
                if( (not old_action.is_error()) and (not (old_action==new_action)) )
                //the actions are ambiguous
                {
                    tmp_ambiguous=true;
                    //check to see if ambiguitiy can be solved with precendance or associativvity
                    bool is_shiftReduce = old_action.is_shift() and new_action.is_reduce();
                    bool is_reduceShift = old_action.is_reduce() and new_action.is_shift();
                    if( is_shiftReduce or is_reduceShift)//check for a shift-reduce conflict
                    {
                        parser_action shift_action=parser_action::get_none();
                        parser_action reduce_action=parser_action::get_none();
                        item shift_item;
                        item reduce_item;
                        //order the shift-reduce acctions
                        if(old_action.is_shift())
                        {
                            shift_action=old_action;
                            shift_item=action_items[action_token->token_ID];
                            reduce_action=new_action;
                            reduce_item=set_item;
                        }
                        else
                        {
                            shift_action=new_action;
                            shift_item=set_item;
                            reduce_action=old_action;
                            reduce_item=action_items[action_token->token_ID ];
                        }

                        if(reduce_item.prod->id == shift_item.prod->id) //try associatitivy
                        {
                            if(reduce_item.prod->is_left_associative()) //reduce
                            {
                                new_action=reduce_action;
                                set_item=reduce_item;
                                tmp_ambiguous=false;
                            }
                            else if(reduce_item.prod->is_right_associative())//shift
                            {
                                new_action=shift_action;
                                set_item=shift_item;
                                tmp_ambiguous=false;
                            }
                        }
                        else if(reduce_item.prod->precedence!=0 and shift_item.prod->precedence!=0) //try precedence
                        {
                            if(reduce_item.prod->precedence < shift_item.prod->precedence) //reduce
                            {
                                new_action=reduce_action;
                                set_item=reduce_item;
                                tmp_ambiguous=false;
                            }
                            else if(reduce_item.prod->precedence > shift_item.prod->precedence)//shift
                            {
                                new_action=shift_action;
                                set_item=shift_item;
                                tmp_ambiguous=false;
                            }
                        }
                    }

                    if(tmp_ambiguous)
                    //the grammer is ambiguous
                    {
                        log("AMBIGUOUS GRAMMER in state ",state_i,". conflict between action: ", old_action,
                            " on item:",action_items[action_token->token_ID], ", and action: ",new_action," on item:",set_item);
                        is_ambiguous=true; //we do not return here, becouse we want to list out all ambiguities
                    }
                }

                if(not tmp_ambiguous)
                {
                    current_state.add_action(action_token->token_ID, new_action);
                    action_items[action_token->token_ID]=set_item;
                }
            }
        }

        //set the goto table
        for(auto& name_nonterm_pair : non_terminals)
        {
            auto goto_set = set->get_goto(name_nonterm_pair.second);
            if(goto_set) current_state.add_goto(name_nonterm_pair.second->token_ID, goto_set->id);
        }
    }


//cout << 'C' << endl;


    //log the action table
    log();
    for( unsigned int state_i=0; state_i<LR1_item_sets.size(); state_i++)
    {
        parser_state& current_state=(*state_table)[state_i];
        log("STATE: ", state_i);
        log();

        for( auto& ST_item : LR1_item_sets[state_i]->items)
        {
            log("  ", ST_item);
        }
        log();

        for(auto&  id_action_pair : current_state.ACTION )
        {
            log("  on ", (*term_map)[id_action_pair.first], " ", id_action_pair.second);
        }
        log();

        for(auto& nonterm_state_pair : current_state.GOTO)
        {
            log("  goto ", nonterm_state_pair.second, " on ", nonterm_map[nonterm_state_pair.first]);
        }
        log();
    }

    log.write("cyth_grammer.txt");

    if( is_ambiguous) return;

    //compact action table
    //compact goto table (see pg 276-277)

    cout<<"Parser Generated"<<endl;

    parser_table_generated=true;
}

//end parser_generator class


//begin parser class
parser::parser(std::shared_ptr<lexer<token_data> > _lex, shared_ptr< map<unsigned int, utf8_string> > _term_map,
       shared_ptr< vector<production_info_ptr> > _production_information, shared_ptr< vector<parser_state> > _state_table) : lex(_lex)
{
    lex = _lex;
    term_map = _term_map;
    production_information = _production_information;
    state_table = _state_table;
    error_recovery = 0;
}

shared_ptr<parser> parser::copy()
{
    return shared_ptr<parser>( new parser(lex, term_map, production_information, state_table));
}

dyn_holder parser::parse(bool reporting)
{

    stack.clear();
    stack.push_back( token_data(0, dyn_holder(), location_span() ) ); //push state 0
    next_terminal = (*lex)();
    error_recovery = 0;


    int state=0;
    bool had_error = false;
    //0 is currently parsing. 1 means succsesfull exit. 2 means unsuccsesfull error recovery. 3 means succsesfull error recovery
    while(state==0)
    {
        state = parse_step(reporting);
        if(state==3)
        {
            had_error = true;
            state = 0;
        }
    }
    if(state==1 and not had_error)
    {
        return get_data();
    }
    else
    {
        return dyn_holder();
    }
}

dyn_holder parser::get_data()
{
    return (--stack.end())->data();
}

int parser::parse_step(bool reporting)
{
    if(reporting)
    {
        cout<<"NEXT TERMINAL: "<< (*term_map)[ next_terminal.get_ID() ]<<endl;
    }

    parser_state& state = (*state_table)[ stack.back().get_ID() ];
    parser_action action = state.get_action( next_terminal.get_ID() );

    if( action.is_error() )
    {
        if(error_recovery != 0)//we are still in error recovery anyway.
        {
            auto state_iter = stack.begin();
            for( unsigned int i=0; i<error_recovery; i++) state_iter++;
            state_iter++; //one more to keep the error token
            stack.erase(state_iter, stack.end()); //throw away the states
            if(next_terminal.get_ID() == eof_token_id )
            {
                return 2; //keep from infinite loop
            }
            else
            {
                next_terminal = (*lex)();//throw away affending terminal
                return 3;
            }
        }

        cout<<"parsing error- expected one of: ";
        for(auto& ID_action_pair : state.ACTION)
        {
            cout<< (*term_map)[ ID_action_pair.first ]<< ',';
        }
        cout<<" Got a "<< (*term_map)[next_terminal.get_ID()]<< '.' << next_terminal.loc()<<endl;

        //if( reporting )
        //{
            cout<<"  ERROR"<<endl<<"  STACK ";
            state_string(cout);
            cout<<endl;
        //}

        //// COMENCE ERROR RECOVERY

        //first find a state that allows for shifting an ERROR token
        auto stack_loc = stack.rbegin();
        bool found_state=false;
        error_recovery=stack.size();
        for (auto end=stack.rend(); stack_loc != end; ++stack_loc )
        {
            error_recovery--;
            parser_state& state = (*state_table)[ stack_loc->get_ID() ];
            if( state.get_action( error_token_id ).is_shift() )
            {
                found_state=true;
                break;//found an acceptable state
            }
        }
        if( not found_state) return 2;//if we have not found an error-recovery state. Return!

        //pop states off the stack
        stack.erase(stack_loc.base(), stack.end());
        //push the error token
        parser_state& curr_state = (*state_table)[ stack_loc->get_ID() ];
        unsigned int new_state_id = curr_state.get_action( error_token_id ).get_data();
        token_data new_state(new_state_id, next_terminal.data(), next_terminal.loc() );
        stack.push_back(new_state);
        next_terminal = (*lex)();

        return 3;
    }

    else if( action.is_reduce())
    {
        if( reporting )
        {
            cout<<"  REDUCING BY PRODUCTION: "<< action.get_data()<<endl;
        }

        //// get important data ////
        auto prod_info = (*production_information)[action.get_data()];

        ////pop the states off of the stack
        auto datum = pop_end_elements(stack, prod_info->num_tokens);

        ////run user action
        parser_function_class::data_T arguments(datum);
        if( not prod_info->action )
        {
            throw gen_exception("PRODUCTION DOES NOT HAVE ACTION");
        }
        dyn_holder new_data = prod_info->action->call(arguments);

        //// make new location_span ////
        location_span new_data_span;
        if( prod_info->num_tokens != 0 )
        {
            auto A = datum.front().loc();
            auto B = datum.back().loc();
            new_data_span = A + B;
        }

        if( reporting )
        {
            cout<<"     loc "<<new_data_span<<endl;
        }

        //// make a new state
        unsigned int new_state_id = (*state_table)[stack.back().get_ID()].get_goto( prod_info->L_val_ID );
        token_data new_state(new_state_id, new_data, new_data_span);
        stack.push_back(new_state);

        ////exit error_recovery
        if( error_recovery!=0) error_recovery=0;

        if(reporting)
        {
            cout<<"  STACK: ";
            state_string(cout);
            cout<<endl;
        }

        return 0;
    }

    else if(action.is_shift())
    {
        if(reporting)
        {
            cout<<"  SHIFTING TERMINAL"<<endl;
        }

        unsigned int state_id=action.get_data();
        token_data new_state(state_id, next_terminal.data(), next_terminal.loc() );
        stack.push_back(new_state);
        next_terminal = (*lex)();
        if(reporting)
        {
            cout<<"  STACK: ";
            state_string(cout);
            cout<<endl;
        }
        return 0;
    }
    else if(action.is_accept())
    {
        if(reporting)
        {
            cout<<"  PARSING COMPLETE"<<endl;
            cout<<"  STACK: ";
            state_string(cout);
            cout<<endl;
        }
        return 1;
    }
    else
    {
        throw gen_exception("ERROR: UNSET ACTION");

        if( reporting )
        {
            cout<<"  ERROR: UNSET ACTION"<<endl<<"  STACK";
            state_string(cout);
        }

        //do not try to recover from error, yet
        return 2;
    }
}

void parser::state_string(std::ostream& os)
{
    os<<'[';
    for( auto& state : stack)
    {
        os<<state.get_ID()<<' ';
    }
    os<<']';
}

//void parser::reset_input(utf8_string& file_name)
//{
//    reset();
//    lex->set_input(file_name);
//}

void parser::reset_input(std::istream& _input)
{
    reset();
    lex->set_input(_input);
}

void parser::reset_input(std::istream& _input, utf8_string& file_name)
{
    reset();
    lex->set_input(_input, file_name);
}

void parser::reset()
{
    stack.clear();
    next_terminal=token_data();
    lex->reset();
}

