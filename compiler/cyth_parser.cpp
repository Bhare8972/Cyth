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

module_AST_ptr module_add_function( parser_function_class::data_T& data )
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

funcParamList_AST_ptr funcParamList_addParam( parser_function_class::data_T& data )
{
    return 4;
}

//block
block_AST_ptr make_block_AST ( parser_function_class::data_T& data )
{
    return 5;
}

block_AST_ptr block_add_statement ( parser_function_class::data_T& data )
{
    return 6;
}

//statements
statement_AST_ptr make_statement_expression_AST ( parser_function_class::data_T& data )
{
    return 7;
}

//expression
expression_AST_ptr make_expression_funcCall_AST ( parser_function_class::data_T& data )
{
    return 8;
}

expression_AST_ptr make_intLiteral_AST ( parser_function_class::data_T& data )
{
    return 9;
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

funcArgument_AST_ptr funcArgument_addArgument_AST ( parser_function_class::data_T& data )
{
    return 12;
}

make_cyth_parser::make_cyth_parser() : cyth_parser_generator("./cyth_parser_table", "./cyth_lexer_table")
{
////// Define the Cyth grammer!! //////

    //// get info from par_gen ////
    auto EOF_term=cyth_parser_generator.get_EOF_terminal();
    auto ERROR_term=cyth_parser_generator.get_ERROR_terminal();
    auto lex_gen=cyth_parser_generator.get_lexer_generator();

    //// define terminals ////
    //keywords
    auto DEF_keyword=cyth_parser_generator.new_terminal("def");

    //symbols
    auto LPAREN_symbol=cyth_parser_generator.new_terminal("(");
    auto RPAREN_symbol=cyth_parser_generator.new_terminal(")");
    auto COMA_symbol=cyth_parser_generator.new_terminal(",");
    auto LBRACE_symbol=cyth_parser_generator.new_terminal("{");
    auto RBRACE_symbol=cyth_parser_generator.new_terminal("}");
    auto NEWLINE_symbol=cyth_parser_generator.new_terminal("\n");

    //other
    auto INT_term=cyth_parser_generator.new_terminal("integer");
    auto IDENTIFIER_term=cyth_parser_generator.new_terminal("identifier");

    //// set terminal paterns ////
    DEF_keyword->add_pattern("def");

    LPAREN_symbol->add_pattern("[(]");
    RPAREN_symbol->add_pattern("[)]");
    COMA_symbol->add_pattern(",");
    LBRACE_symbol->add_pattern("[{]");
    RBRACE_symbol->add_pattern("[}]");
    NEWLINE_symbol->add_pattern("\\n");

    INT_term->add_pattern("[0-9]+");
    IDENTIFIER_term->add_pattern("[# a-z A-Z]+[# 0-9 a-z A-Z]*");

    //// define non-terminals ////
    auto MODULE_nonterm  =cyth_parser_generator.new_nonterminal("module"); //start non-term
    auto FUNC_DEF_nonterm=cyth_parser_generator.new_nonterminal("function definition");
    auto FUNC_PARAMLIST_nonterm = cyth_parser_generator.new_nonterminal("function parameter list");
    auto BLOCK_nonterm   =cyth_parser_generator.new_nonterminal("block");


    auto STATEMENT_nonterm =cyth_parser_generator.new_nonterminal("statement");
    auto EXPRESSION_nonterm=cyth_parser_generator.new_nonterminal("expression");

    auto FUNC_CALL_nonterm=cyth_parser_generator.new_nonterminal("function call");
    auto FUNC_ARGUMENT_nonterm=cyth_parser_generator.new_nonterminal("function argument list");

    //define non-term productions//

    MODULE_nonterm->add_production({ })  .set_action<module_AST_ptr>( make_module_AST );
    MODULE_nonterm->add_production({ MODULE_nonterm, NEWLINE_symbol})  .set_return_action( 0 );
    MODULE_nonterm->add_production({ MODULE_nonterm, FUNC_DEF_nonterm }).set_action<module_AST_ptr>( module_add_function) ;

    FUNC_DEF_nonterm->add_production({ DEF_keyword, IDENTIFIER_term, LPAREN_symbol, FUNC_PARAMLIST_nonterm, RPAREN_symbol,
                                NEWLINE_symbol, RBRACE_symbol, BLOCK_nonterm, LBRACE_symbol}).set_action<funcDef_AST_ptr>( make_function_AST) ;

    FUNC_PARAMLIST_nonterm->add_production({  }).set_action<funcParamList_AST_ptr>( make_funcParamList_AST );
    FUNC_PARAMLIST_nonterm->add_production({ IDENTIFIER_term, IDENTIFIER_term }).set_action<funcParamList_AST_ptr>( make_funcParamList_AST );
    FUNC_PARAMLIST_nonterm->add_production({ FUNC_PARAMLIST_nonterm, COMA_symbol, IDENTIFIER_term, IDENTIFIER_term }).set_action<funcParamList_AST_ptr>( funcParamList_addParam );

    BLOCK_nonterm->add_production({ }).set_action<block_AST_ptr>( make_block_AST );
    BLOCK_nonterm->add_production({ BLOCK_nonterm, STATEMENT_nonterm, NEWLINE_symbol }).set_action<block_AST_ptr>( block_add_statement );
    BLOCK_nonterm->add_production({ BLOCK_nonterm, NEWLINE_symbol }).set_return_action( 0 );

    STATEMENT_nonterm->add_production({ EXPRESSION_nonterm }) .set_action<statement_AST_ptr>( make_statement_expression_AST );

    EXPRESSION_nonterm->add_production({ FUNC_CALL_nonterm }) .set_action<expression_AST_ptr>( make_expression_funcCall_AST );
    EXPRESSION_nonterm->add_production({ INT_term })   .set_action<expression_AST_ptr>( make_intLiteral_AST );

    FUNC_CALL_nonterm->add_production({ IDENTIFIER_term, LPAREN_symbol, FUNC_ARGUMENT_nonterm, RPAREN_symbol }).set_action<funcCall_AST_ptr>( make_funcCall_AST );

    FUNC_ARGUMENT_nonterm->add_production({ })   .set_action<funcArgument_AST_ptr>( make_funcArgument_AST );
    FUNC_ARGUMENT_nonterm->add_production({FUNC_ARGUMENT_nonterm, COMA_symbol, EXPRESSION_nonterm }) .set_action<funcArgument_AST_ptr>( funcArgument_addArgument_AST );

    //// set other lexer actions ////
    lex_gen->add_nonreturn_pattern("\" \"");//to eat spaces

    get_parser(); //force a build of the parser
}

std::shared_ptr<parser> make_cyth_parser::get_parser()
{
    return cyth_parser_generator.get_parser();
}

