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
    return make_shared<module_AST_node>();
}


module_AST_ptr pass_module( parser_function_class::data_T& data )
{
    return  data[0].data<module_AST_ptr>();
}

//module_AST_ptr module_add_statement_AST( parser_function_class::data_T& data )
//{
//    auto module_ptr = data[0].data<module_AST_ptr>();
//    auto statement_ptr = data[1].data<statement_AST_ptr>();
//    module_ptr->add_AST_node( statement_ptr );
//    return  module_ptr;
//}

//module_AST_ptr module_add_ASTnode( parser_function_class::data_T& data )
//{
//    auto module_ptr = data[0].data<module_AST_ptr>();
//    auto node_to_add = data[1].data<AST_node_ptr>();
//    module_ptr->add_AST_node( node_to_add );
//    return  module_ptr;
//}

template< class T >
module_AST_ptr module_add_ASTnode( parser_function_class::data_T& data )
{
    auto module_ptr = data[0].data<module_AST_ptr>();
    auto node_to_add = data[1].data<T>();
    module_ptr->add_AST_node( node_to_add );
    return  module_ptr;
}

//module_AST_ptr module_add_function_AST( parser_function_class::data_T& data )
//{
//    auto module_ptr = data[0].data<module_AST_ptr>();
//    auto function_ptr = data[1].data<function_AST_ptr>();
//    module_ptr->add_AST_node( function_ptr );
//    return  module_ptr;
//}





// imports
AST_node_ptr complete_import_AST( parser_function_class::data_T& data )
{
    auto import_ptr = data[0].data<import_AST_ptr>();
    return  static_pointer_cast<AST_node>( import_ptr ) ;
}

AST_node_ptr complete_ASimport_AST( parser_function_class::data_T& data )
{
    auto import_ptr = data[0].data<import_AST_ptr>();
    import_ptr->set_usage_name( data[2].data<utf8_string>() );
    return  static_pointer_cast<AST_node>( import_ptr ) ;
}

import_AST_ptr starting_Cimport_AST( parser_function_class::data_T& data )
{
    auto import_name = data[1].data<utf8_string>();
    auto new_import_AST = make_shared<import_C_AST_node>( import_name, data[1].loc() );

    return  static_pointer_cast<import_AST_node>( new_import_AST );
}


// AST node block



//function definition
// DEF_keyword, IDENTIFIER_term, LPAREN_symbol, RPAREN_symbol, BLOCK_nonterm
function_AST_ptr make_function_AST( parser_function_class::data_T& data )
{
    auto function_name = data[1].data<utf8_string>();
    auto function_block = data[4].data<block_AST_ptr>();
    auto LOC = data[0].loc() + data[3].loc();


    return make_shared<function_AST_node>( function_name, LOC, function_block );
}


////function_AST_node::parameter_list_ptr make_empty_funcParamList_AST( parser_function_class::data_T& data )
//int make_empty_funcParamList_AST( parser_function_class::data_T& data )
//{
//    //return make_shared<function_AST_node::parameter_list_ptr>();
//    return 2;
//}
//
////function_AST_node::parameter_list_ptr make_funcParamList_AST( parser_function_class::data_T& data )
//int make_funcParamList_AST( parser_function_class::data_T& data )
//{
////    return make_shared<function_AST_node::parameter_list_ptr>();
////
////    auto type_name = data[0].data<utf8_string>();
////    auto var_name = data[1].data<utf8_string>();
////
////    auto new_param_list =  make_shared<function_AST_node::parameter_list_ptr>();
//
//    return 3;
//}
//
////funcParamList_AST_ptr funcParamList_addParam_AST( parser_function_class::data_T& data )
//int funcParamList_addParam_AST( parser_function_class::data_T& data )
//{
//    return 4;
//}


////block

// NEW_BLOCK_term
block_AST_ptr make_block_AST( parser_function_class::data_T& data )
{
    return make_shared<block_AST_node>( data[0].loc );
}

// BLOCKlist_nonterm, THING<T>, junk...
template< class T >
block_AST_ptr block_add_ASTnode( parser_function_class::data_T& data )
{
    auto block_ptr = data[0].data<block_AST_ptr>();
    auto node_to_add = data[1].data<T>();
    block_ptr->add_AST_node( node_to_add, data[1].loc );
    return  block_ptr;
}

// BLOCKlist_nonterm, junk...
block_AST_ptr pass_block( parser_function_class::data_T& data )
{
    return  data[0].data<block_AST_ptr>();
}



// names

// IDENTIFIER_term
varType_ASTrepr_ptr make_typename_simple_AST ( parser_function_class::data_T& data )
{
    auto type_name = data[0].data<utf8_string>();
    auto new_varType_repr =  make_shared<varType_ASTrepr_node>( type_name, data[0].loc() );
    return new_varType_repr;
}




//// statements ///
statement_AST_ptr make_statement_expression_AST ( parser_function_class::data_T& data )
{
    auto expression = data[0].data<expression_AST_ptr>();
    auto new_statement =  make_shared<expression_statement_AST_node>( expression );
    return static_pointer_cast<statement_AST_node>( new_statement );
}

//TYPENAME_nonterm, IDENTIFIER_term
statement_AST_ptr make_statement_definition_AST ( parser_function_class::data_T& data )
{
    auto var_type = data[0].data<varType_ASTrepr_ptr>();
    auto loc_0 = data[0].loc();
    auto var_name = data[1].data<utf8_string>();
    auto loc_1 = data[1].loc();

    auto new_statement =  make_shared<definition_statement_AST_node>( var_type, var_name, loc_0+loc_1 );
    return static_pointer_cast<statement_AST_node>( new_statement );
}

//IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm
statement_AST_ptr make_statement_assignment_AST ( parser_function_class::data_T& data )
{
    auto var_name = data[0].data<utf8_string>();
    auto loc_0 = data[0].loc();
    auto expression = data[2].data<expression_AST_ptr>();
    auto loc_2 = data[2].loc();

    auto new_statement =  make_shared<assignment_statement_AST_node>( var_name, expression, loc_0+loc_2 );
    return static_pointer_cast<statement_AST_node>( new_statement );
}

// EXPRESSION_nonterm, LPAREN_symbol, RPAREN_symbol
statement_AST_ptr make_statement_funcCall_AST ( parser_function_class::data_T& data )
{
    auto loc_0 = data[0].loc();
    auto expression = data[0].data<expression_AST_ptr>();
    auto loc_2 = data[2].loc();

    auto new_statement =  make_shared<functionCall_statement_AST_node>(expression, loc_0+loc_2 );
    return static_pointer_cast<statement_AST_node>( new_statement );
}



//// EXPRESSIONS ////

// INT_term
expression_AST_ptr make_intLiteral_AST ( parser_function_class::data_T& data )
{
    auto int_string = data[0].data<utf8_string>();
    auto new_expression =  make_shared<intLiteral_expression_AST_node>( int_string, data[0].loc() );
    return static_pointer_cast<expression_AST_node>( new_expression );
}

//EXPRESSION_nonterm PLUS EXPRESSION_nonterm
expression_AST_ptr make_expression_addition_AST ( parser_function_class::data_T& data )
{
    auto left_operand = data[0].data<expression_AST_ptr>();
    auto right_operand = data[2].data<expression_AST_ptr>();

    auto new_expression = make_shared<binOperator_expression_AST_node>( left_operand, binOperator_expression_AST_node::addition_t, right_operand);
    return static_pointer_cast<expression_AST_node>( new_expression );
}

//IDENTIFIER_term
expression_AST_ptr make_expression_varRef_AST ( parser_function_class::data_T& data )
{
    auto varname_string = data[0].data<utf8_string>();
    auto new_expression =  make_shared<varReferance_expression_AST_node>( varname_string, data[0].loc() );
    return static_pointer_cast<expression_AST_node>( new_expression );
}







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

csu::token_data cython_lexer::lex_eatnewline(csu::utf8_string& data, location_span& loc)
{
    utf8_string newline("\n");
    unput(newline);

    dyn_holder new_data( 0 );
    return token_data(STMT_id, new_data, loc);
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
            throw lexer_exception("Expected indented block at ", loc);
        }
    }
    else if( data_length == block_lengths.back() )
    {
        dyn_holder new_data( 0 );
        return token_data(STMT_id, new_data, loc);
    }
    else if( data_length > block_lengths.back() )
    {
        throw lexer_exception("Indentation does not match any indentation level ", loc);
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
            throw lexer_exception("Indentation does not match any indentation level ", loc);
        }
    }
}


//// make the cyth parser ////
utf8_string LEX_make_expect_block(utf8_string data, lexer<token_data>* lex)
// a helper function. Use as Lexing action to tell lexer to expect a new block
// otherwise just passes data through
{
    dynamic_cast<cython_lexer*>(lex)->expect_block();
    return data;
}

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


    //lex_gen->add_nonreturn_pattern("\\n(\" \")*\\n");//to eat empty lines

    //to eat empty lines
    lex_gen->add_pattern("\\n(\" \")*\\n", [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
                         cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
                         return cylex->lex_eatnewline(data, loc); });

    //STMT_term->add_pattern("\\n(\" \")*\\n");
    //normal lines
    lex_gen->add_pattern("\\n(\" \")*", [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
                         cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
                         return cylex->lex_newline(data, loc); });




    //keywords
    auto IMPORT_keyword = cyth_parser_generator.new_terminal("import");
    auto CIMPORT_keyword = cyth_parser_generator.new_terminal("cimport");
    auto FROM_keyword = cyth_parser_generator.new_terminal("from");
    auto AS_keyword = cyth_parser_generator.new_terminal("as");
    auto DEF_keyword = cyth_parser_generator.new_terminal("def");


    //symbols
    auto PLUS_symbol = cyth_parser_generator.new_terminal("+");
    auto EQUALS_symbol=cyth_parser_generator.new_terminal("=");
    auto LPAREN_symbol = cyth_parser_generator.new_terminal("(");
    auto RPAREN_symbol=cyth_parser_generator.new_terminal(")");

    //other
    auto INT_term = cyth_parser_generator.new_terminal("integer");
    auto IDENTIFIER_term=cyth_parser_generator.new_terminal("identifier");



    //// set terminal patterns ////
    AS_keyword->add_pattern("as");
    FROM_keyword->add_pattern("from");
    CIMPORT_keyword->add_pattern("cimport");
    IMPORT_keyword->add_pattern("import");

    //DEF_keyword->add_pattern("def");
    DEF_keyword->add_pattern<utf8_string>("def", LEX_make_expect_block );

    PLUS_symbol->add_pattern("[+]");
    EQUALS_symbol->add_pattern("=");
    LPAREN_symbol->add_pattern("[(]");
    RPAREN_symbol->add_pattern("[)]");

    INT_term->add_pattern("[0-9]+");
    IDENTIFIER_term->add_pattern("[# _ a-z A-Z]+[# _ 0-9 a-z A-Z]*");


    //// define non-terminals ////
    auto MODULE_nonterm = cyth_parser_generator.new_nonterminal("module"); //start non-term

    // imports
    auto completeImport_nonterm = cyth_parser_generator.new_nonterminal("completeImport"); // can include "as **"
    auto startingImport_nonterm = cyth_parser_generator.new_nonterminal("startingImport"); // import **m, from ** import **


    //statment block
    auto BLOCK_nonterm = cyth_parser_generator.new_nonterminal("statementBlock");
    auto BLOCKlist_nonterm = cyth_parser_generator.new_nonterminal("statementBlock_list");

    // functions
    auto FUNC_DEF_nonterm = cyth_parser_generator.new_nonterminal("function_definition");

    // types of names
    auto TYPENAME_nonterm = cyth_parser_generator.new_nonterminal("typeName");

    // logical basics
    auto STATEMENT_nonterm = cyth_parser_generator.new_nonterminal("statement");
    auto EXPRESSION_nonterm = cyth_parser_generator.new_nonterminal("expression");



    //define non-term productions//
    MODULE_nonterm->add_production({ }).set_action<module_AST_ptr>( make_module_AST );
    MODULE_nonterm->add_production({ MODULE_nonterm, STMT_term }).set_action<module_AST_ptr>( pass_module );
    //MODULE_nonterm->add_production({ MODULE_nonterm, STATEMENT_nonterm, STMT_term }).set_action<module_AST_ptr>( module_add_statement_AST );
    //MODULE_nonterm->add_production({ MODULE_nonterm, completeImport_nonterm, STMT_term }).set_action<module_AST_ptr>( module_add_ASTnode );
    //MODULE_nonterm->add_production({ MODULE_nonterm, FUNC_DEF_nonterm }).set_action<module_AST_ptr>( module_add_function_AST );
    MODULE_nonterm->add_production({ MODULE_nonterm, FUNC_DEF_nonterm }).set_action<module_AST_ptr>( module_add_ASTnode<function_AST_ptr> );
    MODULE_nonterm->add_production({ MODULE_nonterm, completeImport_nonterm, STMT_term }).set_action<module_AST_ptr>( module_add_ASTnode<AST_node_ptr> );
    MODULE_nonterm->add_production({ MODULE_nonterm, STATEMENT_nonterm, STMT_term }).set_action<module_AST_ptr>( module_add_ASTnode<statement_AST_ptr> );

    //IMPORTS
    completeImport_nonterm->add_production({ startingImport_nonterm }).set_action< AST_node_ptr >( complete_import_AST );
    completeImport_nonterm->add_production({ startingImport_nonterm, AS_keyword, IDENTIFIER_term }).set_action<AST_node_ptr>( complete_ASimport_AST );

    startingImport_nonterm->add_production({ CIMPORT_keyword, IDENTIFIER_term }).set_action<import_AST_ptr>( starting_Cimport_AST );

    //FUNCTIONS
    // functions and classes are not statements, becouse they can be nested, which complicates their writing to C
    // they don't ACT like statements... (whereas loops and conditionals do??)
    //FUNC_DEF_nonterm->add_production({ DEF_keyword, IDENTIFIER_term, LPAREN_symbol, RPAREN_symbol, NEW_BLOCK_term, STATEMENT_nonterm, STMT_term, END_BLOCK_term }) .set_action<function_AST_ptr>( make_function_AST );
    FUNC_DEF_nonterm->add_production({ DEF_keyword, IDENTIFIER_term, LPAREN_symbol, RPAREN_symbol, BLOCK_nonterm }) .set_action<function_AST_ptr>( make_function_AST );


    //General code block
    BLOCK_nonterm->add_production({ BLOCKlist_nonterm, END_BLOCK_term }) .set_action<block_AST_ptr>( pass_block );
    BLOCKlist_nonterm->add_production({ NEW_BLOCK_term }) .set_action<block_AST_ptr>( make_block_AST );
    BLOCKlist_nonterm->add_production({ BLOCKlist_nonterm, STATEMENT_nonterm, STMT_term }) .set_action<block_AST_ptr>( block_add_ASTnode<statement_AST_ptr> );


    // types of names
    TYPENAME_nonterm->add_production({ IDENTIFIER_term }) .set_action<varType_ASTrepr_ptr>( make_typename_simple_AST );

    //Statements
    STATEMENT_nonterm->add_production({ EXPRESSION_nonterm }) .set_action<statement_AST_ptr>( make_statement_expression_AST );
    STATEMENT_nonterm->add_production({ TYPENAME_nonterm, IDENTIFIER_term}) .set_action<statement_AST_ptr>( make_statement_definition_AST );
    STATEMENT_nonterm->add_production({ IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm}) .set_action<statement_AST_ptr>( make_statement_assignment_AST ); // eventually this should use generic LHS symbols
    STATEMENT_nonterm->add_production({ EXPRESSION_nonterm, LPAREN_symbol, RPAREN_symbol}) .set_action<statement_AST_ptr>( make_statement_funcCall_AST ); // eventually this will be an expression!

    //Expressions
    EXPRESSION_nonterm->add_production({ INT_term }) .set_action<expression_AST_ptr>( make_intLiteral_AST );
    EXPRESSION_nonterm->add_production({ IDENTIFIER_term }) .set_action<expression_AST_ptr>( make_expression_varRef_AST );
    EXPRESSION_nonterm->add_production({ EXPRESSION_nonterm, PLUS_symbol, EXPRESSION_nonterm }) .set_action<expression_AST_ptr>( make_expression_addition_AST ).set_associativity( "LEFT" );




    //// set other lexer actions ////
    lex_gen->add_nonreturn_pattern("\" \"");//to eat spaces

    get_parser(do_file_IO); //force a build of the parser



//    //// define terminals ////
//    //python-style newline funky-ness
//    auto STMT_term = cyth_parser_generator.new_terminal("STMT");
//    auto NEW_BLOCK_term = cyth_parser_generator.new_terminal("NEW_BLOCK");
//    auto END_BLOCK_term = cyth_parser_generator.new_terminal("END_BLOCK");
//    STMT_id = STMT_term->token_ID;
//    NEW_BLOCK_id = NEW_BLOCK_term->token_ID;
//    END_BLOCK_id = END_BLOCK_term->token_ID;
//
//
//    lex_gen->add_nonreturn_pattern("\\n(\" \")*\\n");//to eat empty lines
//    lex_gen->add_pattern("\\n(\" \")*", [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
//                         cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
//                         return cylex->lex_newline(data, loc);
//                         });
//
//    //keywords
//    auto DEF_keyword=cyth_parser_generator.new_terminal("def");
//
//    //symbols
//    auto LPAREN_symbol=cyth_parser_generator.new_terminal("(");
//    auto RPAREN_symbol=cyth_parser_generator.new_terminal(")");
//    auto COMA_symbol=cyth_parser_generator.new_terminal(",");
//    auto LBRACE_symbol=cyth_parser_generator.new_terminal("{");
//    auto RBRACE_symbol=cyth_parser_generator.new_terminal("}");
//    auto NEWLINE_symbol=cyth_parser_generator.new_terminal("newline");
//    auto EQUALS_symbol=cyth_parser_generator.new_terminal("=");
//
//    //other
//    auto INT_term=cyth_parser_generator.new_terminal("integer");
//    auto IDENTIFIER_term=cyth_parser_generator.new_terminal("identifier");
//
//    //// set terminal patterns ////
//    DEF_keyword->add_pattern<int>("def", [](utf8_string data, lexer<token_data>* lex){ dynamic_cast<cython_lexer*>(lex)->expect_block(); return 0; } ); //note, this cannot return void
//
//    LPAREN_symbol->add_pattern("[(]");
//    RPAREN_symbol->add_pattern("[)]");
//    COMA_symbol->add_pattern(",");
//    LBRACE_symbol->add_pattern("[{]");
//    RBRACE_symbol->add_pattern("[}]");
//    NEWLINE_symbol->add_pattern("\\n");
//    EQUALS_symbol->add_pattern("=");
//
//    INT_term->add_pattern("[0-9]+");
//    IDENTIFIER_term->add_pattern("[# _ a-z A-Z]+[# _ 0-9 a-z A-Z]*");
//
//    //// define non-terminals ////
//    auto MODULE_nonterm  =cyth_parser_generator.new_nonterminal("module"); //start non-term
//    auto FUNC_DEF_nonterm=cyth_parser_generator.new_nonterminal("function_definition");
//    auto FUNC_PARAMLIST_nonterm = cyth_parser_generator.new_nonterminal("function_parameter_list");
//    auto BLOCK_nonterm   =cyth_parser_generator.new_nonterminal("block");
//    auto BLOCKLIST_nonterm   =cyth_parser_generator.new_nonterminal("block_list");
//    auto BLOCKITEM_nonterm   =cyth_parser_generator.new_nonterminal("block_ITEM");
//
//
//    auto STATEMENT_nonterm =cyth_parser_generator.new_nonterminal("statement");
//    auto EXPRESSION_nonterm=cyth_parser_generator.new_nonterminal("expression");
//
//    auto FUNC_CALL_nonterm=cyth_parser_generator.new_nonterminal("function_call");
//    auto FUNC_ARGUMENT_nonterm=cyth_parser_generator.new_nonterminal("function_argument_list");
//
//    //define non-term productions//
//    MODULE_nonterm->add_production({ })  .set_action<module_AST_ptr>( make_module_AST );
//    MODULE_nonterm->add_production({ MODULE_nonterm, STMT_term})  .set_return_action( 0 );
//    MODULE_nonterm->add_production({ MODULE_nonterm, FUNC_DEF_nonterm }).set_action<module_AST_ptr>( module_add_function_AST) ;
//
//    //FUNCTIONS
//    FUNC_DEF_nonterm->add_production({ DEF_keyword, IDENTIFIER_term, LPAREN_symbol, FUNC_PARAMLIST_nonterm, RPAREN_symbol, BLOCK_nonterm}).set_action<function_AST_ptr>( make_function_AST) ;
//
//    FUNC_PARAMLIST_nonterm->add_production({  }).set_action<funcParamList_AST_ptr>( make_funcParamList_AST );
//    FUNC_PARAMLIST_nonterm->add_production({ IDENTIFIER_term, IDENTIFIER_term }).set_action<function_AST_node::parameter_list_ptr>( make_funcParamList_AST );
//    FUNC_PARAMLIST_nonterm->add_production({ FUNC_PARAMLIST_nonterm, COMA_symbol, IDENTIFIER_term, IDENTIFIER_term }).set_action<funcParamList_AST_ptr>( funcParamList_addParam_AST );
//
//    //Blocks
//    BLOCK_nonterm->add_production({ NEW_BLOCK_term, BLOCKLIST_nonterm, END_BLOCK_term }).set_action<block_AST_ptr>( make_block_AST );
//    BLOCK_nonterm->add_production({ NEW_BLOCK_term, BLOCKITEM_nonterm, END_BLOCK_term }).set_action<block_AST_ptr>( make_block_AST );
//    BLOCK_nonterm->add_production({ NEW_BLOCK_term, BLOCKLIST_nonterm, BLOCKITEM_nonterm, END_BLOCK_term }).set_action<block_AST_ptr>( make_block_AST );
//
//    BLOCKLIST_nonterm->add_production({ BLOCKITEM_nonterm, STMT_term }).set_action<block_AST_ptr>( make_blocklist_AST );
//    BLOCKLIST_nonterm->add_production({ BLOCKLIST_nonterm, BLOCKITEM_nonterm, STMT_term }).set_action<block_AST_ptr>( extend_blocklist_AST );
//
//    BLOCKITEM_nonterm->add_production({ STATEMENT_nonterm }).set_action<block_AST_ptr>( make_blockitem_stmt_AST );
//
//    //Statements
//    STATEMENT_nonterm->add_production({ EXPRESSION_nonterm }) .set_action<statement_AST_ptr>( make_statement_expression_AST );
//    STATEMENT_nonterm->add_production({ IDENTIFIER_term, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm}) .set_action<statement_AST_ptr>( make_statement_definition_AST);
//
//    //Expressions
//    EXPRESSION_nonterm->add_production({ FUNC_CALL_nonterm }) .set_action<expression_AST_ptr>( make_expression_funcCall_AST );
//    EXPRESSION_nonterm->add_production({ INT_term })   .set_action<expression_AST_ptr>( make_intLiteral_AST );
//    EXPRESSION_nonterm->add_production({ IDENTIFIER_term })   .set_action<expression_AST_ptr>( make_expression_reference_AST );
//
//    FUNC_CALL_nonterm->add_production({ IDENTIFIER_term, LPAREN_symbol, FUNC_ARGUMENT_nonterm, RPAREN_symbol }).set_action<funcCall_AST_ptr>( make_funcCall_AST );
//
//    FUNC_ARGUMENT_nonterm->add_production({ })   .set_action<funcArgument_AST_ptr>( make_emptyFuncArgument_AST );
//    FUNC_ARGUMENT_nonterm->add_production({ EXPRESSION_nonterm })   .set_action<funcArgument_AST_ptr>( make_funcArgument_AST );
//    FUNC_ARGUMENT_nonterm->add_production({FUNC_ARGUMENT_nonterm, COMA_symbol, EXPRESSION_nonterm }) .set_action<funcArgument_AST_ptr>( funcArgument_addArgument_AST );
//
//    //// set other lexer actions ////
//    lex_gen->add_nonreturn_pattern("\" \"");//to eat spaces
//
//    get_parser(do_file_IO); //force a build of the parser
}

std::shared_ptr<parser> make_cyth_parser::get_parser(bool do_file_IO)
{
    auto new_parser = cyth_parser_generator.get_parser<cython_lexer, parser>(do_file_IO);

    auto cylex = dynamic_pointer_cast<cython_lexer>( new_parser->lex );
    cylex->set_identifiers( STMT_id, NEW_BLOCK_id, END_BLOCK_id);

    return new_parser;
}

