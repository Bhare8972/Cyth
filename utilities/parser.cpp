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

using namespace csu;
using namespace std;



//begin token class
token::token(utf8_string _name)
{
    name=_name;
    is_terminal=true;
    token_ID=0;
}

ostream& csu::operator<<(ostream& os, const token& dt)
{
    os<<dt.name;
    return os;
}
//end token class

//terminal
terminal::terminal(utf8_string _name, unsigned int ID, parser_generator* _par_gen)
{
    name=_name;
    is_terminal=true;
    token_ID=ID;
    par_gen=_par_gen;
}

void add_pattern(utf8_string pattern, bool return_data=true)
{
    lexer_function_generic new_lex_func(token_ID, return_data)
    par_gen->add_lexer_pattern(pattern, new_lex_func);
}

template<typename ret_type>
void add_pattern(utf8_string pattern, function<ret_type(utf8_string, lexer<token_data>*)> _func)
{
    lexer_functional<ret_type> new_lex_func(token_ID, _func);
    par_gen->add_lexer_pattern(pattern, new_lex_func);
}

template<typename ret_type>
void add_patterns(initializer_list<utf8_string> patterns, function<ret_type(utf8_string, lexer<token_data>*)> _func)
{
    lexer_functional<ret_type> new_lex_func(token_ID, _func);
    for(auto& pattern : patterns)
    {
        par_gen->add_lexer_pattern(pattern, new_lex_func);
    }
}
//end terminal class

//non_terminal
non_terminal::non_terminal(utf8_string _name, unsigned int ID, parser_generator* _par_gen)
{
    name=_name;
    par_gen=_par_gen;
    is_terminal=false;
    token_ID=ID;
}

production& non_terminal::add_production(std::initializer_list<token_ptr> tokens)
{
    std_vector<token_ptr> _tokens(tokens);
    //check for unknown tokens
    for(token_ptr _token : _tokens)
    {
        if(_token->token_ID==0)//found an unknown token
        {
            _par_gen->resolve_unknown_terminal(_token);//make it known
        }
    }
    //make the new production
    std::shared_ptr< production > new_production(new production(this, _tokens);
    productions.push_back(new_production);
    return *new_production.get();
}
//end non_terminal class

//start token_data class
class token_data
//holds data passed around by the parser
{
    unsigned int token_ID;
    dyn_holder t_data;
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

token_data::token_data(unsigned int _ID, dyn_holder _data, location_span _span)
{
    token_ID=_ID;
    t_data=_data;
    span=_span;
}

template<typename ret_T>
ret_T token_data::data()
{
    return t_data.cast<ret_T>();
}

template<typename ret_T>
void token_data::data(ret_T& ret_data)
{
    t_data.cast(ret_data);
}

location_span token_data::loc(){return span;}
unsigned int token_data::get_ID(){return token_ID;}

//end token_data class

// production
production::next_production_ID=0;
production::production(non_terminal* _L_val, std::vector<token_ptr>& _tokens)
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
    parser_function_ptr new_parser_func(new parser_functional<ret_type> new_parser_func(_func));
    action=new_parser_func
    return *this;
    
}

production& production::assoc(association _assoc)
{
    assoc=_assoc;
    return *this;
}

production& production::assoc(utf8_string _assoc)
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
    return &this;
}

production_info production::get_info()
{
    return production_info(L_val->token_ID, L_val->name, tokens.size(), action);
}

std::ostream& operator<<(std::ostream& os, const production& dt)
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
    friend std::ostream& operator<<(std::ostream& os, const production_info& dt);
public:
    unsigned int L_val_ID;
    utf8_string L_val_name;
    unsigned int num_tokens;
    parser_function_ptr action; //will need to see if we really need this
    
production_info::production_info(unsigned int _L_val_ID, utf8_string& _L_val_name, unsigned int _num_tokens, parser_function_ptr _action)
{
    L_val_ID=_L_val_ID;
    L_val_name=_L_val_name;
    num_tokens=_num_tokens;
    action=_action;
}

std::ostream& operator<<(std::ostream& os, const production_info& dt)
{
    os<<dt.L_val_name<<":"<<dt.num_tokens;
    return os;
}

//end production_info

//start parser_action
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

bool parser_action::is_error(){return action_todo==ERROR;}
bool parser_action::is_shift(){return action_todo==SHIFT;}
bool parser_action::is_accept(){return action_todo==ACCEPT;}
bool parser_action::is_reduce(){return action_todo==REDUCE;}

unsigned int parser_action::get_data()
{
    return data;
}
bool parser_action::operator==(parser_action& RHS)
{
    return action_todo==RHS.action_todo and data==RHS.data;
}
//end parser_action

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
    
    if( (*augmented_start->productions.begin())->production_ID == prod->production_ID )
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

bool item::operator==(item& RHS)
{
    if(not lookahead)
    {
        if(not RHS.lookahead)
            return prod->production_ID==RHS.prod->production_ID  and loc==RHS.loc;
        else
            return false;
    }
    else
    {
        if(not RHS.lookahead)
            return false
        else
            return prod->production_ID==RHS.prod->production_ID  and loc==RHS.loc  and lookahead->token_ID==RHS.lookahead->token_ID;
    }
}

std::ostream& operator<<(std::ostream& os, const item& dt)
{
    cout<<prod->L_val_name<<"->";
    for(unsigned int i=0; i<prod->tokens.size(); i++)
    {
        if(i==loc)
        {
            cout<<'.';
        }
        cout<<(prod->tokens[i]->name)<<" ";
    }
    
    if(loc==prod->tokens.size()) cout<<".";
    
    if(lookahead) cout<<'['<<lookahead->name<<']';
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

void item_set::append(item new_item);
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
    if(RHS.size()!=size()) return false;
    
    for(auto this_iter=items.begin(), RHS_iter=RHS.items.begin(), this_end=items.end; this_iter!=this_end; ++this_iter)
    {
        if(not (*this_iter)==(*RHS_iter))
        {
            return false;
        }
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

ostream& operator<<(std::ostream& os, const item_set& dt)
{
    cout<<"SET: "<<id<<endl;
    for(auto& has_item : items )
    {
        cout<<" "<<has_item<<endl;
    }
    cout<<"END SET"<<endl;
}

//end item_set

//start propagation_table
void propagation_table::add_propagation(item_set_ptr from_set, item& from_item, item_set_ptr to_set, item& to_item)
{
    PAIR_T from_pair(from_set, from_item);
    PAIR_T to_pair(to_set, to_item);
    
    table.insert(make_pair(from_pair, to_pair));
}
//end propagation_table

//start parser_state class
void parser_state::add_goto(unsigned int _token_ID, unsigned int _state)
{
    GOTO[_token_ID]==_state;
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
    return GOTO[_token_ID];
}

parser_function_ptr parser_state::get_action(unsigned int non_term)
{
    return ACTION[non_term]
}
//end parser_state

//parser_generator
parser_generator::parser_generator(utf8_string _parser_table_file_name, utf8_string _lexer_table_file_name)
{
    lex_gen=std::shared_ptr<lexer_generator>(new lexer_generator(_lexer_table_file_name) );
    EOF_terminal=terminal_ptr(new terminal("EOF", lex_gen.get()));
    EPSILON_terminal=terminal_ptr(new terminal("EPSILON", lex_gen.get()));
    lex_gen->set_EOF_action(lexer_function_generic(EOF_terminal->token_ID, true));

    parser_table_file_name=_parser_table_file_name;
    parser_table_generated=false;
    next_token_num=1;
}

terminal_ptr parser_generator::get_EOF_terminal()
{
    return EOF_terminal;
}

terminal_ptr parser_generator::get_EPSILON_terminal()
{
    return EPSILON_terminal;
}

std::shared_ptr<lexer_generator> parser_generator::get_lexer_generator()
{
    return lex_gen;
}

terminal_ptr parser_generator::new_terminal(utf8_string name)
{
    if(terminals.find(name)!=terminals.end())
    {
        throw gen_exception("cannot make a new terminal with the same name as anouther terminal");
    }
    if(non_terminals.find(name)!=non_terminals.end())
    {
        throw gen_exception("cannot make a new terminal with the same name as a non-terminal");
    }
    
    terminal_ptr new_term(new terminal(name, next_token_num, this));
    next_token_num++;
    terminals[name]=new_term;
    return new_term;
}

non_terminal_ptr parser_generator::new_nonterminal(utf8_string name)
{
    if(terminals.find(name)!=terminals.end())
    {
        throw gen_exception("cannot make a new non-terminal with the same name as a terminal");
    }
    if(non_terminals.find(name)!=non_terminals.end())
    {
        throw gen_exception("cannot make a new non-terminal with the same name as anouther non-terminal");
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

void set_precedence(std::initializer_list<non_terminal_ptr> operators)
{
    //need to fill this in
    throw gen_exception("not implemented");
}

void parser_generator::print_grammer()
{
    for(auto non_term : non_terminals)
    {
        cout<<non_term->name;
        string prefix("");
        for(int i=0; i<non_term->name.get_length(); i++) prefix+=string(" ");
        prefix+=string("|");
        
        for(auto prod : non_term->productions)
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

std::shared_pointer<parser> parser_generator::get_parser()
{
    //need to fill this in too
    throw gen_exception("not implemented");
}

void parser_generator::add_lexer_pattern(utf8_string& regex_pattern, lexer_function_class func)
{
    lex_gen->add_pattern(regex_pattern, func);
}

void parser_generator::resolve_unknown_terminal(token_ptr _new_terminal) //this is for when the terminal is referanced by a string
{
    if(non_terminals.find(_new_terminal->name)!=non_terminals.end())
    {
        throw gen_exception("cannot referance a non-terminal by string");
    }
    
    auto term_iter=terminals.find(_new_terminal->name)
    if(term_iter==terminals.end())
    {
        //it is a new terminal
        _new_terminal->token_ID=next_token_num;
        next_token_num++;
        lex_gen->add_pattern(_new_terminal->name, lexer_function_generic(_new_terminal->token_ID, true));
        terminals[_new_terminal->name]=_new_terminal;
    }
    else
    {
        //it is a known terminal
        _new_terminal->token_ID=(*term_iter)->token_ID;
    }
}

list<token_ptr> parser_generator::first(list<token_ptr>& tokens)
{
    list<token_ptr> ret;
    list<token_ptr> empty;
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
    if(has_epsilon){ ret.push_back(EPSILON_terminal);}
    return ret;
}

list<token_ptr> parser_generator::first_single(token_ptr _token, list<unsigned int>& exclusion_productions, bool& output_contains_epsilon)
{
    list<token_ptr> ret;
    
    if(_token->token_ID==EPSILON_terminal->token_ID)
    {
        output_contains_epsilon=true;
        return ret;
    }    
    else if(_token->is_terminal)
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
            if( find(exclusion_productions.begin(), exclusion_productions.end(), prod->production_ID) != exclusion_productions.end() )
            {
                //if production is in exclusion_productions, then skip it
                continue;
            }
            
            bool production_is_all_epsilons=true;
            for(auto p_token : prod->tokens)
            {
                list<token_ptr> p_token_first;
                bool p_token_epsilon;
                
                if(p_token->token_ID==_token->token_ID)
                {
                    list<token_ptr> new_exclusions=exclusion_productions;
                    new_exclusions.push_back(prod->production_ID);
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
        for(auto& input_item : input_set)
        {
            auto post_tokens=input_item.get_postTokens();
            auto post_tokens_iterator=post_tokens.begin();
            if(post_tokens.size()>0 and not (*post_tokens_iterator)->is_terminal)
            {
                non_terminal_ptr next_token( dynamic_pointer_cast<non_terminal>(*post_tokens_iterator) );
                list<token_ptr> following_tokens(++post_tokens_iterator, post_tokens.end());
                auto first_data=first(following_tokens);
                for(auto prod : next_token->productions )
                {
                    for(auto first_terminal : first_data)
                    {
                        item new_item(prod, 0, first_terminal)
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
    while(items_added)
    {
        items_added=false;
        for(auto input_item : input_set)
        {
            list<token_ptr> post_tokens( input_item->get_postTokens() );
            if( post_tokens.size()>0 and  not (*post_tokens.begin())->is_terminal)
            {
                non_terminal_ptr next_token= dynamic_pointer_cast<non_terminal>(*post_tokens.begin());
                
                for(auto next_token_production : next_token->productions )
                {
                    item new_item(next_token_production, 0);
                    if(not input_set->has_item(new_item))
                    {
                        input_set->append(new_item);
                        items_added=true;
                    }
                }
            }
        }
    }
}

item_set_ptr parser_generator::goto_LR0(item_set_ptr input_set, token_ptr _token)
{
    item_set_ptr ret( new item_set());
    for(auto& input_item : input_set)
    {
        auto post_tokens=input_item->get_postTokens();
        if(post_tokens.size()>0 and (*post_tokens.begin())->token_ID==_token->token_ID)
        {
            auto new_item=input_item.copy();
            new_item.loc+=1;
            ret.append(new_item);
        }
    }
    
    closure_LR0(ret);
    return ret;
}

list<item_set_ptr> parser_generator::LR0_itemsets(non_terminal_ptr _start_token)
{
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
    }
    
    
    item_set_ptr set_zero(new item_set(0));
    item start_item(*_start_token->productions.begin(), 0);
    set_zero.append(start_item);
    closure_LR0(set_zero);
    list<item_set_ptr> item_sets({set_zero});
    
    unsigned int next_set_ID=1;
    for(auto set : item_sets)
    {
        for(auto&  name_term_pair : terminals)
        {
            auto term=name_term_pair.second();
            auto new_goto=goto_LR0(set, term);
            item_set_ptr _in_list=in_list(item_sets, new_goto)
            if(new_goto->size()>0 and not _in_list)
            {
                new_goto.id=next_set_ID;
                next_set_ID++;
                set->add_goto(term, new_goto);
                item_sets.push_back(new_goto);
            }
            else if( _in_list )
            {
                set->add_goto(term, _in_list);
            }
        }
        
        for(auto&  name_nonterm_pair : non_terminals)
        {
            auto nonterm=name_nonterm_pair.second();
            auto new_goto=goto_LR0(set, nonterm);
            item_set_ptr _in_list=in_list(item_sets, new_goto)
            if(new_goto->size()>0 and not _in_list)
            {
                new_goto.id=next_set_ID;
                next_set_ID++;
                set->add_goto(nonterm, new_goto);
                item_sets.push_back(new_goto);
            }
            else if( _in_list )
            {
                set->add_goto(nonterm, _in_list);
            }
        }
    }
    return item_sets;
}

NEED TO SOLVE EPSILON PROBLEM

void parser_generator::generate_parser_table()
{
    logger log();
    
    ////augment the grammer////
    if(not start_nonterm)
    {
        throw gen_exception("need at least one production");
    }
    non_terminal_ptr augmented_start= new_nonterminal("Augmented Start Token");
    augmented_start->add_production({start_nonterm});
    
    ////basic accounting////
    //terminal map
    log("terminals:");
    for(auto name_term_pair : terminals)
    {
        log(name_term_pair.second()->token_ID, ": ", name_term_pair->first());
        term_map[name_term_pair.second()->token_ID]=name_term_pair->first();
    }
    log();
    
    //terminal error recovery table
    //not doing this
    
    //log the nonterms
    log("nonterminals:");
    map<unsigned int, utf8_string> nonterm_map;
    unsigned int num_productions;
    for(auto name_nonterm_pair : nonterminals)
    {
        log(name_nonterm_pair.second()->token_ID, ": ", name_nonterm_pair->first());
        nonterm_map[name_nonterm_pair.second()->token_ID]=name_nonterm_pair->first();
        num_productions+=name_nonterm_pair.second()->productions.size();
    }
    log();
    
    //set production information    
    log("grammer productions:");
    production_information.resize(num_productions);
    for(auto name_nonterm_pair : nonterminals)
    {
        for(auto prod : name_nonterm_pair.second()->productions)
        {
            log(prod->id, ': ', prod);
            production_information[prod->id]=prod.get_info();
        }
    }
    log();
    
    ///// generate the parse table /////
    auto LR0_item_sets=LR0_itemsets(augmented_start);
    
    //remove kernal items ?? (there is an extra not in here... it is removing non-kernal items)
    for( auto set : LR0_item_sets)
    {
        set->items.erase( remove_if(set->items.begin(), set->items.end(), [&](item& it){return not it.is_kernal(augmented_start);}), set->items.end() );
    }
    
    //create new LR1 item stes from old LR0 item sets
    vector< item_set_ptr > LR1_item_sets;
    LR1_item_sets.resize(LR0_item_sets.size());
    for(auto LR0_set : LR0_item_sets) //make all the LR1 sets
    {
        LR1_item_sets[LR0_set->id]=item_set_ptr(new item_set(set->id));
    }
    for(auto LR0_set : LR0_item_sets) //duplicate the goto tables
    {
        for(auto& tokenID_set : LR0_set)
        {
            auto LR1_goto_set=LR1_item_sets[ tokenID_set.second()->id ]
            LR1_item_sets[LR0_set->id]->goto_table[ tokenID_set.first() ]=LR1_goto_set;
        }
    }
    
    //algorithm 4.62 Generate propagation table and spontanious lookaheads
    propagation_table prop_table();
    terminal_ptr fake_terminal("fakeTerm", next_token_num, this);
    next_token_num++;
    for(auto set_K : LR0_item_sets)
    {
        for(auto& name_term_pair : terminals)
        {
            auto term_X=name_term_pair.second();
            auto LR0_goto_set=set_K->get_goto(term_X);
            if(not LR0_goto_set) continue;
            auto LR1_set=LR1_item_sets[LR0_goto_set->id];
            
            for(item& K_item : *set_K)
            {
                item_set_ptr set_J( new item_set(0, K_item->copy(fake_terminal) ) );
                closure_LR1(set_J);
                for(item& J_item : *set_J)
                {
                    auto post_tokens=J_item->get_postTokens();
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
                                LR1_set.append(J_item_copy);
                            }
                        }
                    }
                }
            }
        }
        for(auto& name_nonterm_pair : terminals)
        {
            auto nonterm_X=name_nonterm_pair.second();
            auto LR0_goto_set=set_K->get_goto(nonterm_X);
            if(not LR0_goto_set) continue;
            auto LR1_set=LR1_item_sets[LR0_goto_set->id];
            
            for(item& K_item : *set_K)
            {
                item_set_ptr set_J( new item_set(0, K_item->copy(fake_terminal) ) );
                closure_LR1(set_J);
                for(item& J_item : *set_J)
                {
                    auto post_tokens=J_item->get_postTokens();
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
                                LR1_set.append(J_item_copy);
                            }
                        }
                    }
                }
            }
        }
    }
    
    //add EOF to augmented_start production
    for(auto& LR0_item : *(*LR0_item_sets.begin()))
    {
        if(LR0_item.production->id==augmented_start->id and LR0_item->loc==0)
        {
            item new_item=LRO_item.copy(EOF_terminal);
            (*LR0_item_sets.begin())->append(new_item);
        }
    }
    
    //propagate lookahead items to form all LALR(1) items
    bool items_added=true;
    I AM HERE
}
            
                        
        ##propagate lookahead items to form all LALR(1) items
        items_added=True
        while items_added: <- IS HERE
            items_added=False
            
            for  LR1set in LR1_item_sets:
                LR0_set=LR0_item_sets[ LR1set.id ]
                for LR1item in LR1set:
                    LR0item=LR1item.copy(None)
                    propagate_to=prop_table.get_propagations(LR0_set,LR0item )
                    for to_set, to_item in propagate_to:
                        new_to_item=to_item.copy( LR1item.lookahead )
                        if new_to_item not in to_set:
                            to_set.append(new_to_item)
                            items_added=True
                
        
        ##closure on each set of items to get non-kernal items
        [ self.closure_LR1(set) for set in LR1_item_sets ]

        ##generate table using algoritm 4.56
        self.states=[ state() for x in LR1_item_sets] ## a state for each item set
        accept_item=item(augmented_start.productions[0], 1, self.EOF_terminal)
        is_ambiguous=False
        for state_i in xrange(len(self.states)):
            current_state=self.states[state_i]
            set=LR1_item_sets[state_i]
            ##set the actions
            new_action=None
            action_items={}
            for set_item in set:
                post_tokens=set_item.get_postTokens()
                if set_item==accept_item:
                    ##accept
                    new_action=action.accept()
                    action_token=self.EOF_terminal
                elif len(post_tokens)==0 and set_item.lookahead.is_terminal:
                    ##reduce
                    new_action=action.reduce(set_item.production.id)
                    action_token=set_item.lookahead
                elif post_tokens[0].is_terminal:
                    ##shift
                    set_to_shift_to=set.goto(post_tokens[0])
                    new_action=action.shift(set_to_shift_to.id)
                    action_token=post_tokens[0]
                
                if new_action:
                    tmp_ambiguous=False
                    ##check for conflicts
                    old_action=current_state.action(action_token.id)
                    if (not old_action.is_error()) and not old_action==new_action:
                        tmp_ambiguous=True
                        ##check to see if conflict can be solved by associativity and precidance
                        if (old_action.is_shift() and new_action.is_reduce()) or (new_action.is_shift() and old_action.is_reduce()): ##make sure we have a shift-reduce conflict
                            ##order the shift and reduce actions
                            if old_action.is_shift():
                                shift_action=old_action
                                reduce_action=new_action
                                reduce_item=set_item
                                shift_item=action_items[action_token.id]
                            else:
                                shift_action=new_action,
                                reduce_action=old_action
                                shift_item=set_item
                                reduce_item=action_items[action_token.id]
                                
                            if reduce_item.production==shift_item.production: ##try associativity
                                if reduce_item.production.assoc=='left':
                                    new_action=reduce_action
                                    set_item=reduce_item
                                    tmp_ambiguous=False
                                elif reduce_item.production.assoc=='right':
                                    new_action=shift_action
                                    set_item=shift_item
                                    tmp_ambiguous=False
                                    
                            elif (reduce_item.production in self.opperator_precedence) and (shift_item.production in self.opperator_precedence): ##try precidance
                                reduce_index=self.opperator_precedence.index(reduce_item.production)
                                shift_index=self.opperator_precedence.index(shift_item.production)
                                if shift_index<reduce_index:
                                    new_action=shift_action
                                    set_item=shift_item
                                    tmp_ambiguous=False
                                elif reduce_index<shift_index:
                                    new_action=reduce_action
                                    set_item=reduce_item
                                    tmp_ambiguous=False
                                    
                        if tmp_ambiguous:
                            ##not able to resolve ambiguity
                            log("AMBIGUOUS GRAMMER in state ",state_i,". conflict between action: ", old_action,
                                " on item:",action_items[action_token.id], ", and action: ",new_action," on item:",set_item)
                            is_ambiguous=True ##we do not return here, becouse we want to list out all ambiguities
                    
                    if not tmp_ambiguous:
                        current_state.add_action(action_token.id, new_action)
                        action_items[action_token.id]=set_item
                        
            ##set the goto table
            for nonterm in self.nonterms:
                goto=set.goto(nonterm)
                if goto:
                    current_state.add_goto(nonterm.id, goto.id)
        
        ##log action table
        log()
        for state_i in xrange(len(self.states)):
            log('STATE: ',state_i)
            log()
            for ST_item in LR1_item_sets[state_i].items:
                log('  ', ST_item)
            log()
            for term_id, token_action in self.states[state_i].ACTION.iteritems():
                log('  on ',self.term_map[term_id],' ',token_action)
            log()
            for nonterm_id, goto_state in self.states[state_i].GOTO.iteritems():
                log('  goto ',goto_state,' on ',nonterm_map[nonterm_id])
            log()
            
#        if is_ambiguous:
#            return
                        
        ##compact action table
#        for current_state in self.states:
#            reduce_action=None
#            to_remove=[]
#            for token_id,token_action in current_state.ACTION.iteritems():
#                if token_action.is_reduce():
#                    to_remove.append(token_id)
#                    if reduce_action==None:
#                        reduce_action=token_action
#                    elif not reduce_action==token_action:
#                        print "ERROR: different reduces in same state:", current_state.
#            if reduce_action != None:
#                current_state.default_action=reduce_action
#            for rem in to_remove:
#                del current_state.ACTION[rem]
                
                        
        ##compact goto table (see pg 276-277) 
                        
        self.parser_info_generated=True

//end parser_generator class
