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

This file defines the Cyth syntax and produces the lexer
*/

#include "cyth_parser.hpp"

using namespace std;
using namespace csu;

//// functions used in parsing ////

//module
module_AST_ptr make_module_AST( parser_function_class::data_T& data )
{
    return 0;
}

module_AST_ptr module_add_function_AST( parser_function_class::data_T& data )
{
    return 1;
}

//function definition
funcDef_AST_ptr make_function_AST( parser_function_class::data_T& data )
{
    return 2;
}

funcParamList_AST_ptr make_funcParamList_AST( parser_function_class::data_T& data )
{
    return 3;
}

funcParamList_AST_ptr funcParamList_addParam_AST( parser_function_class::data_T& data )
{
    return 4;
}

//block
block_AST_ptr make_block_AST ( parser_function_class::data_T& data )
{
    return 5;
}

block_AST_ptr block_add_statement_AST ( parser_function_class::data_T& data )
{
    return 6;
}

block_AST_ptr make_blocklist_AST ( parser_function_class::data_T& data )
{
    return 6;
}

block_AST_ptr extend_blocklist_AST ( parser_function_class::data_T& data )
{
    return 6;
}

block_AST_ptr make_blockitem_stmt_AST ( parser_function_class::data_T& data )
{
    return 6;
}

//statements
statement_AST_ptr make_statement_expression_AST ( parser_function_class::data_T& data )
{
    return 7;
}

statement_AST_ptr make_statement_definition_AST ( parser_function_class::data_T& data )
{
    return 13;
};

//expression
expression_AST_ptr make_expression_funcCall_AST ( parser_function_class::data_T& data )
{
    return 8;
}

expression_AST_ptr make_intLiteral_AST ( parser_function_class::data_T& data )
{
    return 9;
}

expression_AST_ptr make_expression_reference_AST ( parser_function_class::data_T& data )
{
    return 14;
}

//function calls
funcCall_AST_ptr make_funcCall_AST ( parser_function_class::data_T& data )
{
    return 10;
}

funcArgument_AST_ptr make_funcArgument_AST ( parser_function_class::data_T& data )
{
    return 11;
}

funcArgument_AST_ptr make_emptyFuncArgument_AST ( parser_function_class::data_T& data )
{
    return 15;
}

funcArgument_AST_ptr funcArgument_addArgument_AST ( parser_function_class::data_T& data )
{
    return 12;
}


////// newline manager
//newline_manager::newline_manager(terminal& STMT_term, terminal& NEW_BLOCK_TERM, terminal& END_BLOCK_TERM)
//{
//    STMT_id = STMT_term.token_ID;
//    NEW_BLOCK_id = NEW_BLOCK_TERM.token_ID;
//    END_BLOCK_id = END_BLOCK_TERM.token_ID;
//
//    reset();
//}
//
//void newline_manager::reset()
//{
//    expect_block = false;
//    num_chars.clear()
//    num_chars.push_back(0);
//}
//
//void newline_manager::expect_new_block(utf8_string& data, lexer<token_data>* lex)
//{
//    expect_block = true;
//}
//
//token_data newline_manager::parse(utf8_string& data, location_span& location, lexer<token_data>* lex)
//{
//
//}

//// lexer
cython_lexer::cython_lexer(lex_func_t _EOF_action, std::shared_ptr< std::vector< std::shared_ptr<csu::DFA_state> > > _state_table,
                std::shared_ptr< std::vector< lex_func_t > > _actions, std::shared_ptr< std::vector< unsigned int> > _lexer_states) :
lexer<token_data>(_EOF_action, _state_table, _actions, _lexer_states)
{
    block_lengths.push_back(1);
    expecting_block = false;
}

void cython_lexer::set_identifiers(int _STMT_id, int _NEW_BLOCK_id, int _END_BLOCK_id)
{
    STMT_id = _STMT_id;
    NEW_BLOCK_id = _NEW_BLOCK_id;
    END_BLOCK_id = _END_BLOCK_id;
}

void cython_lexer::expect_block()
{
    expecting_block = true;
}

csu::token_data cython_lexer::lex_newline(csu::utf8_string& data, location_span& loc)
{
    int data_length = data.get_length();

    if( expecting_block )
    {
        if( data_length > block_lengths.back() )
        {
            expecting_block = false;
            block_lengths.push_back( data_length );

            dyn_holder new_data( 0 );
            return token_data(NEW_BLOCK_id, new_data, loc);
        }
        else
        {
            throw lexer_exception("Expected indented block at", loc);
        }
    }
    else if( data_length == block_lengths.back() )
    {
        dyn_holder new_data( 0 );
        return token_data(STMT_id, new_data, loc);
    }
    else if( data_length > block_lengths.back() )
    {
        throw lexer_exception("Indentation does not match any indentation level", loc);
    }
    else
    {
        block_lengths.pop_back();
        if( data_length <= block_lengths.back() )
        {
            if( data_length < block_lengths.back() )
            {
                unput(data);
            }

            dyn_holder new_data( 0 );
            return token_data(END_BLOCK_id, new_data, loc);
        }
        else
        {
            throw lexer_exception("Indentation does not match any indentation level", loc);
        }
    }
}


//// make the cyth parser ////
make_cyth_parser::make_cyth_parser(bool do_file_IO) : cyth_parser_generator("./cyth_parser_table", "./cyth_lexer_table")
{
////// Define the Cyth grammer!! //////

    //// get info from par_gen ////
    auto EOF_term = cyth_parser_generator.get_EOF_terminal();
    auto ERROR_term = cyth_parser_generator.get_ERROR_terminal();
    auto lex_gen = cyth_parser_generator.get_lexer_generator();

    //// define terminals ////
    //python-style newline funky-ness
    auto STMT_term = cyth_parser_generator.new_terminal("STMT");
    auto NEW_BLOCK_term = cyth_parser_generator.new_terminal("NEW_BLOCK");
    auto END_BLOCK_term = cyth_parser_generator.new_terminal("END_BLOCK");
    STMT_id = STMT_term->token_ID;
    NEW_BLOCK_id = NEW_BLOCK_term->token_ID;
    END_BLOCK_id = END_BLOCK_term->token_ID;


    lex_gen->add_nonreturn_pattern("\\n(\" \")*\\n");//to eat empty lines
    lex_gen->add_pattern("\\n(\" \")*", [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
                         cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
                         return cylex->lex_newline(data, loc);
                         });

    //keywords
    auto DEF_keyword=cyth_parser_generator.new_terminal("def");

    //symbols
    auto LPAREN_symbol=cyth_parser_generator.new_terminal("(");
    auto RPAREN_symbol=cyth_parser_generator.new_terminal(")");
    auto COMA_symbol=cyth_parser_generator.new_terminal(",");
    auto LBRACE_symbol=cyth_parser_generator.new_terminal("{");
    auto RBRACE_symbol=cyth_parser_generator.new_terminal("}");
    auto NEWLINE_symbol=cyth_parser_generator.new_terminal("newline");
    auto EQUALS_symbol=cyth_parser_generator.new_terminal("=");

    //other
    auto INT_term=cyth_parser_generator.new_terminal("integer");
    auto IDENTIFIER_term=cyth_parser_generator.new_terminal("identifier");

    //// set terminal patterns ////
    DEF_keyword->add_pattern<int>("def", [](utf8_string data, lexer<token_data>* lex){ dynamic_cast<cython_lexer*>(lex)->expect_block(); return 0; } ); //note, this cannot return void

    LPAREN_symbol->add_pattern("[(]");
    RPAREN_symbol->add_pattern("[)]");
    COMA_symbol->add_pattern(",");
    LBRACE_symbol->add_pattern("[{]");
    RBRACE_symbol->add_pattern("[}]");
    NEWLINE_symbol->add_pattern("\\n");
    EQUALS_symbol->add_pattern("=");

    INT_term->add_pattern("[0-9]+");
    IDENTIFIER_term->add_pattern("[# _ a-z A-Z]+[# _ 0-9 a-z A-Z]*");

    //// define non-terminals ////
    auto MODULE_nonterm  =cyth_parser_generator.new_nonterminal("module"); //start non-term
    auto FUNC_DEF_nonterm=cyth_parser_generator.new_nonterminal("function_definition");
    auto FUNC_PARAMLIST_nonterm = cyth_parser_generator.new_nonterminal("function_parameter_list");
    auto BLOCK_nonterm   =cyth_parser_generator.new_nonterminal("block");
    auto BLOCKLIST_nonterm   =cyth_parser_generator.new_nonterminal("block_list");
    auto BLOCKITEM_nonterm   =cyth_parser_generator.new_nonterminal("block_ITEM");


    auto STATEMENT_nonterm =cyth_parser_generator.new_nonterminal("statement");
    auto EXPRESSION_nonterm=cyth_parser_generator.new_nonterminal("expression");

    auto FUNC_CALL_nonterm=cyth_parser_generator.new_nonterminal("function_call");
    auto FUNC_ARGUMENT_nonterm=cyth_parser_generator.new_nonterminal("function_argument_list");

    //define non-term productions//
    MODULE_nonterm->add_production({ })  .set_action<module_AST_ptr>( make_module_AST );
    MODULE_nonterm->add_production({ MODULE_nonterm, STMT_term})  .set_return_action( 0 );
    MODULE_nonterm->add_production({ MODULE_nonterm, FUNC_DEF_nonterm }).set_action<module_AST_ptr>( module_add_function_AST) ;

    //FUNCTIONS
    FUNC_DEF_nonterm->add_production({ DEF_keyword, IDENTIFIER_term, LPAREN_symbol, FUNC_PARAMLIST_nonterm, RPAREN_symbol, BLOCK_nonterm}).set_action<funcDef_AST_ptr>( make_function_AST) ;

    FUNC_PARAMLIST_nonterm->add_production({  }).set_action<funcParamList_AST_ptr>( make_funcParamList_AST );
    FUNC_PARAMLIST_nonterm->add_production({ IDENTIFIER_term, IDENTIFIER_term }).set_action<funcParamList_AST_ptr>( make_funcParamList_AST );
    FUNC_PARAMLIST_nonterm->add_production({ FUNC_PARAMLIST_nonterm, COMA_symbol, IDENTIFIER_term, IDENTIFIER_term }).set_action<funcParamList_AST_ptr>( funcParamList_addParam_AST );

    //Blocks
    BLOCK_nonterm->add_production({ NEW_BLOCK_term, BLOCKLIST_nonterm, END_BLOCK_term }).set_action<block_AST_ptr>( make_block_AST );
    BLOCK_nonterm->add_production({ NEW_BLOCK_term, BLOCKITEM_nonterm, END_BLOCK_term }).set_action<block_AST_ptr>( make_block_AST );
    BLOCK_nonterm->add_production({ NEW_BLOCK_term, BLOCKLIST_nonterm, BLOCKITEM_nonterm, END_BLOCK_term }).set_action<block_AST_ptr>( make_block_AST );

    BLOCKLIST_nonterm->add_production({ BLOCKITEM_nonterm, STMT_term }).set_action<block_AST_ptr>( make_blocklist_AST );
    BLOCKLIST_nonterm->add_production({ BLOCKLIST_nonterm, BLOCKITEM_nonterm, STMT_term }).set_action<block_AST_ptr>( extend_blocklist_AST );

    BLOCKITEM_nonterm->add_production({ STATEMENT_nonterm }).set_action<block_AST_ptr>( make_blockitem_stmt_AST );

    //Statments
    STATEMENT_nonterm->add_production({ EXPRESSION_nonterm }) .set_action<statement_AST_ptr>( make_statement_expression_AST );
    STATEMENT_nonterm->add_production({ IDENTIFIER_term, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm}) .set_action<statement_AST_ptr>( make_statement_definition_AST);

    //Expressions
    EXPRESSION_nonterm->add_production({ FUNC_CALL_nonterm }) .set_action<expression_AST_ptr>( make_expression_funcCall_AST );
    EXPRESSION_nonterm->add_production({ INT_term })   .set_action<expression_AST_ptr>( make_intLiteral_AST );
    EXPRESSION_nonterm->add_production({ IDENTIFIER_term })   .set_action<expression_AST_ptr>( make_expression_reference_AST );

    FUNC_CALL_nonterm->add_production({ IDENTIFIER_term, LPAREN_symbol, FUNC_ARGUMENT_nonterm, RPAREN_symbol }).set_action<funcCall_AST_ptr>( make_funcCall_AST );

    FUNC_ARGUMENT_nonterm->add_production({ })   .set_action<funcArgument_AST_ptr>( make_emptyFuncArgument_AST );
    FUNC_ARGUMENT_nonterm->add_production({ EXPRESSION_nonterm })   .set_action<funcArgument_AST_ptr>( make_funcArgument_AST );
    FUNC_ARGUMENT_nonterm->add_production({FUNC_ARGUMENT_nonterm, COMA_symbol, EXPRESSION_nonterm }) .set_action<funcArgument_AST_ptr>( funcArgument_addArgument_AST );

    //// set other lexer actions ////
    lex_gen->add_nonreturn_pattern("\" \"");//to eat spaces

    get_parser(do_file_IO); //force a build of the parser
}

std::shared_ptr<parser> make_cyth_parser::get_parser(bool do_file_IO)
{
    auto new_parser = cyth_parser_generator.get_parser<cython_lexer, parser>(do_file_IO);

    auto cylex = dynamic_pointer_cast<cython_lexer>( new_parser->lex );
    cylex->set_identifiers( STMT_id, NEW_BLOCK_id, END_BLOCK_id);

    return new_parser;
}

