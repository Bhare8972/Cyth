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
#include "AST_visitor.hpp"

using namespace std;
using namespace csu;

//// functions used in parsing ////

/// module ///
module_AST_ptr make_module_AST( parser_function_class::data_T& data )
{
    return make_shared<module_AST_node>();
}


module_AST_ptr pass_module( parser_function_class::data_T& data )
{
    return  data[0].data<module_AST_ptr>();
}

template< class T, int lvl=1 >
module_AST_ptr module_add_ASTnode( parser_function_class::data_T& data )
{
    auto module_ptr = data[0].data<module_AST_ptr>();
    auto node_to_add = data[lvl].data<T>();
    module_ptr->add_AST_node( node_to_add );
    return  module_ptr;
}





/// imports ///
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

//CIMPORT_keyword, IDENTIFIER_term
import_AST_ptr starting_Cimport_AST( parser_function_class::data_T& data )
{
    auto import_name = data[1].data<utf8_string>();
    auto new_import_AST = make_shared<import_C_AST_node>( import_name, data[1].loc() );

    return  static_pointer_cast<import_AST_node>( new_import_AST );
}

//FROM_keyword, STRING_term, CIMPORT_keyword, IDENTIFIER_term
import_AST_ptr startingFrom_Cimport_AST( parser_function_class::data_T& data )
{
    auto loc = data[0].loc();
    auto file_name = data[1].data<utf8_string>();
    file_name = file_name.slice(1, file_name.get_length()-1);
    auto import_name = data[3].data<utf8_string>();
    loc = loc + data[3].loc();

    return make_shared<import_C_AST_node>( file_name, import_name, loc );
}






/// classes

//TYPENAME_nonterm, IDENTIFIER_term
ClassVarDef_AST_ptr make_simple_ClassVarDef( parser_function_class::data_T& data )
{
    auto var_type_ASTnode = data[0].data<varType_ASTrepr_ptr>();
    auto loc = data[0].loc();
    loc = loc + data[1].loc();
    auto var_name = data[1].data<utf8_string>();

    return make_shared<class_varDefinition_AST_node>(var_type_ASTnode, var_name, loc);
}

//CLASS_keyword, IDENTIFIER_term, NEW_BLOCK_term
class_AST_ptr make_class_AST( parser_function_class::data_T& data )
{
    auto loc = data[0].loc();
    auto name = data[1].data<utf8_string>();
    loc = loc + data[2].loc();

    return make_shared<class_AST_node>(name, loc);
}

//CLASS_BLOCK_nonterm, END_BLOCK_term
class_AST_ptr pass_ClassBlock ( parser_function_class::data_T& data )
{
    return data[0].data<class_AST_ptr>();
}

//CLASS_BLOCK_nonterm, CLASS_VARDEF_nonterm, STMT_term
//CLASS_BLOCK_nonterm, CLASS_VARDEF_nonterm, END_BLOCK_term
class_AST_ptr ClassBlock_add_VarDefinition( parser_function_class::data_T& data )
{
    auto class_def = data[0].data<class_AST_ptr>();
    auto var_def = data[1].data<ClassVarDef_AST_ptr>();

    class_def->add_var_def( var_def );
    return class_def;
}

//CLASS_BLOCK_nonterm, CLASS_METHOD_nonterm
class_AST_ptr ClassBlock_add_Method( parser_function_class::data_T& data )
{
    auto class_def = data[0].data<class_AST_ptr>();
    auto method = data[1].data<method_AST_ptr>();

    class_def->add_method_def( method );
    return class_def;
}






/// function definition ///
//DEF_keyword, IDENTIFIER_term, FUNC_PARAMS_nonterm, BLOCK_nonterm
function_AST_ptr make_function_AST( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto function_name = data[1].data<utf8_string>();
    auto parameter_list = data[2].data<paramList_AST_ptr>();
    LOC = LOC + data[2].loc();
    auto function_block = data[3].data<block_AST_ptr>();

    return make_shared<function_AST_node>( function_name, LOC, parameter_list, function_block );
}


//DEF_keyword, LCURLY_symbol, TYPENAME_nonterm, RCURLY_symbol, IDENTIFIER_term, FUNC_PARAMS_nonterm , BLOCK_nonterm
function_AST_ptr make_function_AST_WRetT ( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto return_type_ASTnode = data[2].data<varType_ASTrepr_ptr>();
    auto function_name = data[4].data<utf8_string>();
    auto parameter_list = data[5].data<paramList_AST_ptr>();
    LOC = LOC + data[5].loc();
    auto function_block = data[6].data<block_AST_ptr>();

    return make_shared<function_AST_node>( function_name, return_type_ASTnode, LOC, parameter_list, function_block );
}


// parameter lists
// LPAREN_symbol, RPAREN_symbol
paramList_AST_ptr make_empty_paramenter_list( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc() + data[1].loc();
    return make_shared<function_parameter_list>( LOC, nullptr, nullptr );
}

//LPAREN_symbol, REQUIRED_PARAMS_nonterm, RPAREN_symbol
paramList_AST_ptr make_paramenter_list_wReq( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc() + data[2].loc();
    auto required_params = data[1].data<function_parameter_list::required_ptr>();
    return make_shared<function_parameter_list>( LOC, required_params, nullptr );
}

//LPAREN_symbol, DEFAULTED_PARAMS_nonterm, RPAREN_symbol
paramList_AST_ptr make_paramenter_list_wDef( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto defaulted_params = data[1].data<function_parameter_list::default_ptr>();
    LOC = LOC + data[2].loc();
    return make_shared<function_parameter_list>( LOC, nullptr, defaulted_params );
}


//LPAREN_symbol, REQUIRED_PARAMS_nonterm, COMMA_symbol, DEFAULTED_PARAMS_nonterm, RPAREN_symbol
paramList_AST_ptr make_paramenter_list_wReqDef( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto required_params = data[1].data<function_parameter_list::required_ptr>();
    auto defaulted_params = data[3].data<function_parameter_list::default_ptr>();
    LOC = LOC + data[4].loc();
    return make_shared<function_parameter_list>( LOC, required_params, defaulted_params );
}

// required parameters
//TYPENAME_nonterm, IDENTIFIER_term
function_parameter_list::required_ptr make_required_paramenter_list( parser_function_class::data_T& data )
{
    auto var_type_ASTnode = data[0].data<varType_ASTrepr_ptr>();
    auto loc = data[0].loc();
    auto var_name = data[1].data<utf8_string>();
    loc = loc + data[1].loc();

    auto new_req_params = make_shared<function_parameter_list::required_params>( loc );
    new_req_params->add_typed_param(var_type_ASTnode, var_name, loc);
    return new_req_params;
}

//REQUIRED_PARAMS_nonterm, COMMA_symbol, TYPENAME_nonterm, IDENTIFIER_term
function_parameter_list::required_ptr addTo_required_paramenter_list( parser_function_class::data_T& data )
{
    auto req_params_list = data[0].data<function_parameter_list::required_ptr>();
    auto var_type_ASTnode = data[2].data<varType_ASTrepr_ptr>();
    auto loc = data[2].loc();
    auto var_name = data[3].data<utf8_string>();
    loc = loc + data[3].loc();

    req_params_list->add_typed_param(var_type_ASTnode, var_name, loc);
    return req_params_list;
}

// defaulting parameters
//TYPENAME_nonterm, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm
function_parameter_list::default_ptr make_defaulted_paramenter_list( parser_function_class::data_T& data )
{
    auto var_type_ASTnode = data[0].data<varType_ASTrepr_ptr>();
    auto loc = data[0].loc();
    auto var_name = data[1].data<utf8_string>();
    auto default_exp = data[3].data<expression_AST_ptr>();
    loc = loc + data[3].loc();

    auto new_def_params = make_shared<function_parameter_list::defaulted_params>( loc );
    new_def_params->add_typed_param(var_type_ASTnode, var_name, default_exp, loc);
    return new_def_params;
}


//DEFAULTED_PARAMS_nonterm, COMMA_symbol, TYPENAME_nonterm, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm
function_parameter_list::default_ptr  addTo_defaulted_paramenter_list( parser_function_class::data_T& data )
{
    auto def_params_list = data[0].data<function_parameter_list::default_ptr>();
    auto var_type_ASTnode = data[2].data<varType_ASTrepr_ptr>();
    auto loc = data[2].loc();
    auto var_name = data[3].data<utf8_string>();
    auto default_exp = data[5].data<expression_AST_ptr>();
    loc = loc + data[5].loc();

    def_params_list->add_typed_param(var_type_ASTnode, var_name, default_exp, loc);
    return def_params_list;
}




// calling arguments
//LPAREN_symbol, RPAREN_symbol
argumentList_AST_ptr make_empty_argument_list( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc() + data[1].loc();
    return make_shared<call_argument_list>( LOC, nullptr, nullptr );
}

//LPAREN_symbol, UNNAMED_ARGUMENTS_nonterm, RPAREN_symbol
argumentList_AST_ptr make_arguments_wUn( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto unnamed_arguments = data[1].data<call_argument_list::un_arguments_ptr>();
    LOC = LOC + data[2].loc();
    return make_shared<call_argument_list>( LOC, unnamed_arguments, nullptr );
}

//LPAREN_symbol, NAMED_ARGUMENTS_nonterm, RPAREN_symbol
argumentList_AST_ptr make_arguments_wNam( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto named_arguments = data[1].data<call_argument_list::named_arguments_ptr>();
    LOC = LOC + data[2].loc();
    return make_shared<call_argument_list>( LOC, nullptr, named_arguments );
}

//LPAREN_symbol, UNNAMED_ARGUMENTS_nonterm, COMMA_symbol, NAMED_ARGUMENTS_nonterm, RPAREN_symbol
argumentList_AST_ptr make_arguments_wUnNam( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto unnamed_arguments = data[1].data<call_argument_list::un_arguments_ptr>();
    auto named_arguments = data[3].data<call_argument_list::named_arguments_ptr>();
    LOC = LOC + data[4].loc();
    return make_shared<call_argument_list>( LOC, unnamed_arguments, named_arguments );
}

// un-named arguments
//EXPRESSION_nonterm
call_argument_list::un_arguments_ptr make_unnamed_arguments( parser_function_class::data_T& data )
{
    auto data_exp = data[0].data<expression_AST_ptr>();
    auto loc = data[0].loc();

    auto new_arguments = make_shared<call_argument_list::unnamed_arguments_T>( loc );
    new_arguments->add_argument(data_exp, loc);
    return new_arguments;
}

//UNNAMED_ARGUMENTS_nonterm, COMMA_symbol, EXPRESSION_nonterm
call_argument_list::un_arguments_ptr extend_unnamed_arguments( parser_function_class::data_T& data )
{
    auto argument_list = data[0].data<call_argument_list::un_arguments_ptr>();
    auto data_exp = data[2].data<expression_AST_ptr>();
    auto loc = data[2].loc();

    argument_list->add_argument(data_exp, loc);
    return argument_list;
}

// named arguments
//IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm
call_argument_list::named_arguments_ptr make_named_arguments( parser_function_class::data_T& data )
{
    auto loc = data[0].loc();
    auto ID = data[0].data<utf8_string>();
    auto data_exp = data[2].data<expression_AST_ptr>();
    loc = loc + data[2].loc();

    auto new_arguments = make_shared<call_argument_list::named_arguments_T>( loc );
    new_arguments->add_argument(ID, data_exp, loc);
    return new_arguments;
}

//NAMED_ARGUMENTS_nonterm, COMMA_symbol, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm
call_argument_list::named_arguments_ptr extend_named_arguments( parser_function_class::data_T& data )
{
    auto argument_list = data[0].data<call_argument_list::named_arguments_ptr>();
    auto loc = data[2].loc();
    auto ID = data[2].data<utf8_string>();
    auto data_exp = data[4].data<expression_AST_ptr>();
    loc = loc + data[4].loc();

    argument_list->add_argument(ID, data_exp, loc);
    return argument_list;
}


/// methods ///

//DEF_keyword, IDENTIFIER_term, FUNC_PARAMS_nonterm , BLOCK_nonterm
method_AST_ptr make_method_AST( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto function_name = data[1].data<utf8_string>();
    auto parameter_list = data[2].data<paramList_AST_ptr>();
    LOC = LOC + data[2].loc();
    auto function_block = data[3].data<block_AST_ptr>();

    return make_shared<method_AST_node>( function_name, LOC, parameter_list, function_block );
}

//DEF_keyword, LCURLY_symbol, TYPENAME_nonterm, RCURLY_symbol, IDENTIFIER_term, FUNC_PARAMS_nonterm , BLOCK_nonterm
method_AST_ptr make_method_AST_WRetT( parser_function_class::data_T& data )
{
    auto LOC = data[0].loc();
    auto return_type_ASTnode = data[2].data<varType_ASTrepr_ptr>();
    auto function_name = data[4].data<utf8_string>();
    auto parameter_list = data[5].data<paramList_AST_ptr>();
    LOC = LOC + data[5].loc();
    auto function_block = data[6].data<block_AST_ptr>();

    return make_shared<method_AST_node>( function_name, return_type_ASTnode, LOC, parameter_list, function_block );
}








/// block ///

// NEW_BLOCK_term
block_AST_ptr make_block_AST( parser_function_class::data_T& data )
{
    return make_shared<block_AST_node>( data[0].loc() );
}

// BLOCKlist_nonterm, THING<T>, junk...
template< class T >
block_AST_ptr block_add_ASTnode( parser_function_class::data_T& data )
{
    auto block_ptr = data[0].data<block_AST_ptr>();
    auto node_to_add = data[1].data<T>();
    block_ptr->add_AST_node( node_to_add, data[1].loc() );
    return  block_ptr;
}

// BLOCKlist_nonterm, junk...
block_AST_ptr pass_block( parser_function_class::data_T& data )
{
    return  data[0].data<block_AST_ptr>();
}




/// names ///
// IDENTIFIER_term
varType_ASTrepr_ptr make_typename_simple_AST ( parser_function_class::data_T& data )
{
    auto type_name = data[0].data<utf8_string>();
    auto new_varType_repr =  make_shared<varType_ASTrepr_node>( type_name, data[0].loc() );
    return new_varType_repr;
}

/// LHS references ///

// note there is a major limitation of LALR(1) parser, in that it cannot generically distinguish between RHS expressions and LHS references
// The only case where it can is when an LHS reference is used in a specific context, but this is very limited.
// HOWEVER, we want to distinguish between LHS references and RHS expressions in the AST.
// Therefore we exploit the symmetry between LHS and RHS expressions to transform the RHS expression into a LHS reference
// This symmetry, however, is broken in two ways:
//     1) not all RHS expressions are LHS references.
//     2) there is a difference between the "top level" and lower levels of a LHS reference.
// (2) is a fact, but shouldn't be a problem. (1) could cause an issue if a RHS expression is written that cannot be transformed into a LHS reference.
// if this happens, a compiler error is created. (print to screen, verification set to 0).
// Personally, I think this is all really cool, lots of interesting mathematical properties exploited to extend the LALR(1) language.

//EXPRESSION_nonterm
LHSref_AST_ptr RHSexp_to_LHSref( parser_function_class::data_T& data )
{
    // first we need a visitor.
    // assumes this is a lower level LHS reference. If this assumption is wrong, change at a later stage.

    class expression_transformation_visitor : public AST_visitorTree
    {
    public:
        LHSref_AST_ptr current_node;
        bool is_good; // set to false initialy. If not set to good on way DOWN, then an error is made

        expression_transformation_visitor()
        {
            current_node = nullptr;
            is_good = false;
        }

        shared_ptr< AST_visitor_base > make_child(int number) override
        {
            return make_shared<expression_transformation_visitor>();
        }

        /// in case there is a problem ///
        void ASTnode_down(AST_node* ASTnode) override
        {
            if( not is_good )
            {
                // note that we want to express the error, but not block compilation.

                cout << "LHS expression is too general " << ASTnode->loc << endl;

                // make a dummy node
                current_node = make_shared<LHS_reference_AST_node>();
                current_node->verification_state = 0;
            }
        }

        bool apply_to_children() override { return is_good; }

        void set_good() { is_good=true; } // call this on way down for good nodes.

        /// now we express the nodes that can be parsed. Note that both DOWN and UP are needed

        void varReferance_down(varReferance_expression_AST_node* varRefExp) override { set_good(); }

        void varReferance_up(varReferance_expression_AST_node* varRefExp) override
        {
            current_node = make_shared< LHS_varReference >( varRefExp->var_name, varRefExp->loc );
            current_node->level = 1;
        }


        void accessorExp_down(accessor_expression_AST_node* accessorExp) override { set_good(); }

        void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor) override
        {
            auto child_vstr = dynamic_cast<expression_transformation_visitor*>( expChild_visitor );
            auto child_node = child_vstr->current_node;

            if( not child_node)
            {
                throw gen_exception("ERROR in RHSexp_to_LHSref expression_transformation_visitor::accessorExp_up. This should not be reached.");
            }

            current_node = make_shared< LHS_accessor_AST_node >( child_node, accessorExp->name, accessorExp->loc );
            current_node->level = 1;
        }
    };

    auto expression = data[0].data<expression_AST_ptr>();
    expression_transformation_visitor transformation_central;
    expression->apply_visitor( &transformation_central );

    auto new_LHS_reference = transformation_central.current_node;
    new_LHS_reference->level = 0; // this is the top level!
    return new_LHS_reference;
}






/// statements ///
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

//LHS_REFERENCE_nonterm, EQUALS_symbol, EXPRESSION_nonterm
statement_AST_ptr make_statement_assignment_AST ( parser_function_class::data_T& data )
{
    auto LHS_ref = data[0].data<LHSref_AST_ptr>();
    auto loc_0 = data[0].loc();
    auto expression = data[2].data<expression_AST_ptr>();
    auto loc_2 = data[2].loc();

    return make_shared<assignment_statement_AST_node>( LHS_ref, expression, loc_0+loc_2 );
}

// AUTO_keyword, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm
statement_AST_ptr make_auto_definition_AST ( parser_function_class::data_T& data )
{
    auto loc_0 = data[0].loc();
    auto var_name = data[1].data<utf8_string>();
    auto expression = data[3].data<expression_AST_ptr>();
    auto loc_3 = data[3].loc();

    auto new_statement =  make_shared<auto_definition_statement_AST_node>(var_name, expression, loc_0+loc_3 );
    return static_pointer_cast<statement_AST_node>( new_statement );
}

// RETURN_keyword, EXPRESSION_nonterm
statement_AST_ptr make_statement_return_AST ( parser_function_class::data_T& data )
{
    auto loc = data[0].loc();
    auto expression = data[1].data<expression_AST_ptr>();
    loc = loc + data[1].loc();

    auto new_statement =  make_shared<return_statement_AST_node>( expression, loc );
    return new_statement;
}






/// EXPRESSIONS ///

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

// parenthesis grouping
//LPAREN_symbol, EXPRESSION_nonterm, RPAREN_symbol
expression_AST_ptr make_parenGrouped_expression_AST ( parser_function_class::data_T& data )
{
    auto loc = data[0].loc();
    auto expression = data[1].data<expression_AST_ptr>();
    loc = loc + data[2].loc();

    return make_shared<ParenGrouped_expression_AST_node>( expression, loc );
}

//function call
//EXPRESSION_nonterm, CALL_ARGUMENTS_nonterm
expression_AST_ptr make_expression_funcCall_AST ( parser_function_class::data_T& data )
{
    auto loc = data[0].loc();
    auto expression = data[0].data<expression_AST_ptr>();
    auto arguments = data[1].data<argumentList_AST_ptr>();
    loc = loc + data[1].loc();

    return make_shared<functionCall_expression_AST_node>(expression, arguments, loc );
}

//EXPRESSION_nonterm, DOT_symbol, IDENTIFIER_term
expression_AST_ptr make_accessor_expression_AST( parser_function_class::data_T& data )
{
    auto loc = data[0].loc();
    auto expression = data[0].data<expression_AST_ptr>();
    auto varname_string = data[2].data<utf8_string>();
    loc = loc + data[2].loc();

    return make_shared<accessor_expression_AST_node>(expression, varname_string, loc );
}







//// lexer
cython_lexer::cython_lexer(lex_func_t _EOF_action, std::shared_ptr< std::vector< std::shared_ptr<csu::DFA_state> > > _state_table,
                std::shared_ptr< std::vector< lex_func_t > > _actions, std::shared_ptr< std::vector< unsigned int> > _lexer_states) :
lexer<token_data>(_EOF_action, _state_table, _actions, _lexer_states)
{
    block_lengths.push_back(1);
    expecting_block = false;
}

void cython_lexer::set_identifiers(int _STMT_id, int _NEW_BLOCK_id, int _END_BLOCK_id, int _EOF_id)
{
    STMT_id = _STMT_id;
    NEW_BLOCK_id = _NEW_BLOCK_id;
    END_BLOCK_id = _END_BLOCK_id;
    EOF_id = _EOF_id;
}

void cython_lexer::expect_block()
{
    expecting_block = true;
}

csu::token_data cython_lexer::lex_eatnewline(csu::utf8_string& data, location_span& loc)
{
    utf8_string newline("\n");
    unput(newline);

    continue_lexing(true);

    dyn_holder new_data( 0 );// this will get thrown away
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

token_data cython_lexer::lex_EOF(location_span& loc)
{
    if( block_lengths.size() ==1 )
    {
        dyn_holder new_data( 0 );
        return token_data(EOF_id, new_data, loc);
    }
    else
    {
        block_lengths.pop_back();
        dyn_holder new_data( 0 );
        return token_data(END_BLOCK_id, new_data, loc);
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


    /// define terminals ///

    //python-style newline funky-ness
    auto STMT_term = cyth_parser_generator.new_terminal("STMT");
    auto NEW_BLOCK_term = cyth_parser_generator.new_terminal("NEW_BLOCK");
    auto END_BLOCK_term = cyth_parser_generator.new_terminal("END_BLOCK");
    STMT_id = STMT_term->token_ID;
    NEW_BLOCK_id = NEW_BLOCK_term->token_ID;
    END_BLOCK_id = END_BLOCK_term->token_ID;
    EOF_id = EOF_term->token_ID;


    //to eat empty lines
    lex_gen->add_pattern("\\n(\" \")*\\n", [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
                         cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
                         return cylex->lex_eatnewline(data, loc); });

    // comments
    lex_gen->add_pattern(" \\n? (\" \")* # [^ \\n]* \\n", [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
                         cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
                         return cylex->lex_eatnewline(data, loc); });

    //normal lines
    lex_gen->add_pattern("\\n(\" \")*", [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
                         cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
                         return cylex->lex_newline(data, loc); });

    // end of file
    lex_gen->set_EOF_action( [](utf8_string& data, location_span& loc, lexer<token_data>* lex){
                          cython_lexer* cylex = dynamic_cast<cython_lexer*>(lex);
                           return cylex->lex_EOF(loc); } );


    //keywords
    auto IMPORT_keyword = cyth_parser_generator.new_terminal("import");
    auto CIMPORT_keyword = cyth_parser_generator.new_terminal("cimport");
    auto FROM_keyword = cyth_parser_generator.new_terminal("from");
    auto AS_keyword = cyth_parser_generator.new_terminal("as");
    auto DEF_keyword = cyth_parser_generator.new_terminal("def");
    auto AUTO_keyword = cyth_parser_generator.new_terminal("auto");
    auto RETURN_keyword = cyth_parser_generator.new_terminal("return");
    auto CLASS_keyword = cyth_parser_generator.new_terminal("class");

    //symbols
    auto PLUS_symbol = cyth_parser_generator.new_terminal("+");
    auto EQUALS_symbol = cyth_parser_generator.new_terminal("=");
    auto LPAREN_symbol = cyth_parser_generator.new_terminal("(");
    auto RPAREN_symbol = cyth_parser_generator.new_terminal(")");
    auto LCURLY_symbol = cyth_parser_generator.new_terminal("{");
    auto RCURLY_symbol = cyth_parser_generator.new_terminal("}");
    auto COMMA_symbol = cyth_parser_generator.new_terminal(",");
    auto DOT_symbol = cyth_parser_generator.new_terminal("D");

    //other
    auto INT_term = cyth_parser_generator.new_terminal("integer");
    auto STRING_term = cyth_parser_generator.new_terminal("string");
    auto IDENTIFIER_term = cyth_parser_generator.new_terminal("identifier");
    //auto FILENAME_term = cyth_parser_generator.new_terminal("file_name");




    /// set terminal patterns ///
    AS_keyword->add_pattern("as");
    FROM_keyword->add_pattern("from");
    CIMPORT_keyword->add_pattern("cimport");
    IMPORT_keyword->add_pattern("import");
    AUTO_keyword->add_pattern("auto");
    RETURN_keyword->add_pattern("return");
    CLASS_keyword->add_pattern<utf8_string>("class", LEX_make_expect_block );
    DEF_keyword->add_pattern<utf8_string>("def", LEX_make_expect_block );

    PLUS_symbol->add_pattern("[+]");
    EQUALS_symbol->add_pattern("=");
    LPAREN_symbol->add_pattern("[(]");
    RPAREN_symbol->add_pattern("[)]");
    LCURLY_symbol->add_pattern("[{]");
    RCURLY_symbol->add_pattern("[}]");
    COMMA_symbol->add_pattern(",");
    DOT_symbol->add_pattern("[.]");

    INT_term->add_pattern("[0-9]+");
    IDENTIFIER_term->add_pattern("[# _ a-z A-Z]+[# _ 0-9 a-z A-Z]*");
    //FILENAME_term->add_pattern("[# _ a-z A-Z 0-9 . \\\\ ]+");
    STRING_term->add_pattern(" < [# _ a-z A-Z 0-9 . \\\\ ]+  > "); // not correct. Fijn for now



    /// define non-terminals ///
    auto MODULE_nonterm = cyth_parser_generator.new_nonterminal("module"); //start non-term

    // imports
    auto completeImport_nonterm = cyth_parser_generator.new_nonterminal("completeImport"); // can include "as **"
    auto startingImport_nonterm = cyth_parser_generator.new_nonterminal("startingImport"); // import **m, from ** import **

    //statment block
    auto BLOCK_nonterm = cyth_parser_generator.new_nonterminal("statementBlock");
    auto BLOCKlist_nonterm = cyth_parser_generator.new_nonterminal("statementBlock_list");

    // functions
    auto FUNC_DEF_nonterm = cyth_parser_generator.new_nonterminal("function_definition");
    auto FUNC_PARAMS_nonterm = cyth_parser_generator.new_nonterminal("parameter_list");
    auto REQUIRED_PARAMS_nonterm = cyth_parser_generator.new_nonterminal("required_parameter_list");
    auto DEFAULTED_PARAMS_nonterm = cyth_parser_generator.new_nonterminal("defaulted_parameter_list");

    auto CALL_ARGUMENTS_nonterm = cyth_parser_generator.new_nonterminal("argument_list");
    auto UNNAMED_ARGUMENTS_nonterm = cyth_parser_generator.new_nonterminal("unnamed_argument_list");
    auto NAMED_ARGUMENTS_nonterm = cyth_parser_generator.new_nonterminal("named_argument_list");

    // CLASSES
    auto CLASS_DEF_nonterm = cyth_parser_generator.new_nonterminal("class_definition");
    auto CLASS_BLOCK_nonterm = cyth_parser_generator.new_nonterminal("class_block");
    auto CLASS_VARDEF_nonterm = cyth_parser_generator.new_nonterminal("class_variable_definition");
    auto CLASS_METHOD_nonterm = cyth_parser_generator.new_nonterminal("class_method_definition");

    // types of names
    auto TYPENAME_nonterm = cyth_parser_generator.new_nonterminal("type_name");
    auto LHS_REFERENCE_nonterm = cyth_parser_generator.new_nonterminal("LHS_reference");

    // logical basics
    auto STATEMENT_nonterm = cyth_parser_generator.new_nonterminal("statement");
    auto EXPRESSION_nonterm = cyth_parser_generator.new_nonterminal("expression");


    ///define non-term productions///

    // module
    MODULE_nonterm->add_production({ }).set_action<module_AST_ptr>( make_module_AST );
    MODULE_nonterm->add_production({ MODULE_nonterm, STMT_term }).set_action<module_AST_ptr>( pass_module );

    MODULE_nonterm->add_production({ MODULE_nonterm, FUNC_DEF_nonterm }).set_action<module_AST_ptr>( module_add_ASTnode<function_AST_ptr> );
    MODULE_nonterm->add_production({ MODULE_nonterm, CLASS_DEF_nonterm }).set_action<module_AST_ptr>( module_add_ASTnode<class_AST_ptr> );
    MODULE_nonterm->add_production({ MODULE_nonterm, completeImport_nonterm, STMT_term }).set_action<module_AST_ptr>( module_add_ASTnode<AST_node_ptr> );
    MODULE_nonterm->add_production({ MODULE_nonterm, completeImport_nonterm, EOF_term }).set_action<module_AST_ptr>( module_add_ASTnode<AST_node_ptr> );
    MODULE_nonterm->add_production({ MODULE_nonterm, STATEMENT_nonterm, STMT_term }).set_action<module_AST_ptr>( module_add_ASTnode<statement_AST_ptr> );
    MODULE_nonterm->add_production({ MODULE_nonterm, STATEMENT_nonterm, EOF_term }).set_action<module_AST_ptr>( module_add_ASTnode<statement_AST_ptr> );



    //IMPORTS
    completeImport_nonterm->add_production({ startingImport_nonterm }).set_action< AST_node_ptr >( complete_import_AST );
    completeImport_nonterm->add_production({ startingImport_nonterm, AS_keyword, IDENTIFIER_term }).set_action<AST_node_ptr>( complete_ASimport_AST );

    startingImport_nonterm->add_production({ CIMPORT_keyword, IDENTIFIER_term }).set_action<import_AST_ptr>( starting_Cimport_AST );
    startingImport_nonterm->add_production({ FROM_keyword, STRING_term, CIMPORT_keyword, IDENTIFIER_term }).set_action<import_AST_ptr>( startingFrom_Cimport_AST );


    // CLASSES
    CLASS_DEF_nonterm->add_production({ CLASS_BLOCK_nonterm, END_BLOCK_term }) .set_action<class_AST_ptr>( pass_ClassBlock );
    CLASS_DEF_nonterm->add_production({ CLASS_BLOCK_nonterm, CLASS_VARDEF_nonterm, END_BLOCK_term }) .set_action<class_AST_ptr>( ClassBlock_add_VarDefinition  );

    CLASS_BLOCK_nonterm->add_production({ CLASS_keyword, IDENTIFIER_term, NEW_BLOCK_term }) .set_action<class_AST_ptr>( make_class_AST );
    CLASS_BLOCK_nonterm->add_production({ CLASS_BLOCK_nonterm, CLASS_VARDEF_nonterm, STMT_term }) .set_action<class_AST_ptr>( ClassBlock_add_VarDefinition );
    CLASS_BLOCK_nonterm->add_production({ CLASS_BLOCK_nonterm, CLASS_METHOD_nonterm }) .set_action<class_AST_ptr>( ClassBlock_add_Method );

    // member variables
    CLASS_VARDEF_nonterm->add_production({ TYPENAME_nonterm, IDENTIFIER_term }) .set_action<ClassVarDef_AST_ptr>( make_simple_ClassVarDef );

    // methods
    CLASS_METHOD_nonterm->add_production({ DEF_keyword, IDENTIFIER_term, FUNC_PARAMS_nonterm , BLOCK_nonterm }) .set_action<method_AST_ptr>( make_method_AST );
    CLASS_METHOD_nonterm->add_production({ DEF_keyword, LCURLY_symbol, TYPENAME_nonterm, RCURLY_symbol, IDENTIFIER_term, FUNC_PARAMS_nonterm , BLOCK_nonterm }) .set_action<method_AST_ptr>( make_method_AST_WRetT );





    //FUNCTIONS
    // functions and classes are not statements, becouse they can be nested, which complicates their writing to C
    // they don't ACT like statements, in the sense that they need to be written to C in a recursive manner, and they have overloads. As opposed to loops and conditionals, which are statements
    FUNC_DEF_nonterm->add_production({ DEF_keyword, IDENTIFIER_term, FUNC_PARAMS_nonterm , BLOCK_nonterm }) .set_action<function_AST_ptr>( make_function_AST );
    FUNC_DEF_nonterm->add_production({ DEF_keyword, LCURLY_symbol, TYPENAME_nonterm, RCURLY_symbol, IDENTIFIER_term, FUNC_PARAMS_nonterm , BLOCK_nonterm }) .set_action<function_AST_ptr>( make_function_AST_WRetT );

    FUNC_PARAMS_nonterm->add_production({ LPAREN_symbol, RPAREN_symbol}).set_action<paramList_AST_ptr>( make_empty_paramenter_list );
    FUNC_PARAMS_nonterm->add_production({ LPAREN_symbol, REQUIRED_PARAMS_nonterm, RPAREN_symbol }).set_action<paramList_AST_ptr>( make_paramenter_list_wReq );
    FUNC_PARAMS_nonterm->add_production({ LPAREN_symbol, DEFAULTED_PARAMS_nonterm, RPAREN_symbol }).set_action<paramList_AST_ptr>( make_paramenter_list_wDef );
    FUNC_PARAMS_nonterm->add_production({ LPAREN_symbol, REQUIRED_PARAMS_nonterm, COMMA_symbol, DEFAULTED_PARAMS_nonterm, RPAREN_symbol }).set_action<paramList_AST_ptr>( make_paramenter_list_wReqDef );

    REQUIRED_PARAMS_nonterm->add_production({ TYPENAME_nonterm, IDENTIFIER_term }).set_action<function_parameter_list::required_ptr>( make_required_paramenter_list );
    REQUIRED_PARAMS_nonterm->add_production({ REQUIRED_PARAMS_nonterm, COMMA_symbol, TYPENAME_nonterm, IDENTIFIER_term }).set_action<function_parameter_list::required_ptr>( addTo_required_paramenter_list );

    DEFAULTED_PARAMS_nonterm->add_production({ TYPENAME_nonterm, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm }).set_action<function_parameter_list::default_ptr>( make_defaulted_paramenter_list );
    DEFAULTED_PARAMS_nonterm->add_production({ DEFAULTED_PARAMS_nonterm, COMMA_symbol, TYPENAME_nonterm, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm }).set_action<function_parameter_list::default_ptr>( addTo_defaulted_paramenter_list );

    // function arguments
    CALL_ARGUMENTS_nonterm->add_production({ LPAREN_symbol, RPAREN_symbol}).set_action<argumentList_AST_ptr>( make_empty_argument_list ).set_precedence();
    CALL_ARGUMENTS_nonterm->add_production({ LPAREN_symbol, UNNAMED_ARGUMENTS_nonterm, RPAREN_symbol}).set_action<argumentList_AST_ptr>( make_arguments_wUn ).set_precedence();
    CALL_ARGUMENTS_nonterm->add_production({ LPAREN_symbol, NAMED_ARGUMENTS_nonterm, RPAREN_symbol}).set_action<argumentList_AST_ptr>( make_arguments_wNam ).set_precedence();
    CALL_ARGUMENTS_nonterm->add_production({ LPAREN_symbol, UNNAMED_ARGUMENTS_nonterm, COMMA_symbol, NAMED_ARGUMENTS_nonterm, RPAREN_symbol}).set_action<argumentList_AST_ptr>( make_arguments_wUnNam ).set_precedence();

    UNNAMED_ARGUMENTS_nonterm->add_production({ EXPRESSION_nonterm }).set_action<call_argument_list::un_arguments_ptr>( make_unnamed_arguments );
    UNNAMED_ARGUMENTS_nonterm->add_production({ UNNAMED_ARGUMENTS_nonterm, COMMA_symbol, EXPRESSION_nonterm }).set_action<call_argument_list::un_arguments_ptr>( extend_unnamed_arguments );

    NAMED_ARGUMENTS_nonterm->add_production({ IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm }).set_action<call_argument_list::named_arguments_ptr>( make_named_arguments );
    NAMED_ARGUMENTS_nonterm->add_production({ NAMED_ARGUMENTS_nonterm, COMMA_symbol, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm }).set_action<call_argument_list::named_arguments_ptr>( extend_named_arguments );




    //General code block
    BLOCK_nonterm->add_production({ BLOCKlist_nonterm, END_BLOCK_term }) .set_action<block_AST_ptr>( pass_block );
    BLOCK_nonterm->add_production({ BLOCKlist_nonterm, STATEMENT_nonterm, END_BLOCK_term }) .set_action<block_AST_ptr>( block_add_ASTnode<statement_AST_ptr>  );

    BLOCKlist_nonterm->add_production({ NEW_BLOCK_term }) .set_action<block_AST_ptr>( make_block_AST );
    BLOCKlist_nonterm->add_production({ BLOCKlist_nonterm, STATEMENT_nonterm, STMT_term }) .set_action<block_AST_ptr>( block_add_ASTnode<statement_AST_ptr> );
//BLOCKlist_nonterm->add_production({ BLOCKlist_nonterm, FUNC_DEF_nonterm }) .set_action<block_AST_ptr>( block_add_ASTnode<function_AST_ptr> ); // need to implement closure!! NOTE: when a closured variable goes out of scope, perhaps copy it to a dynamic variable?
//BLOCKlist_nonterm->add_production({ BLOCKlist_nonterm, CLASS_DEF_nonterm }) .set_action<block_AST_ptr>( block_add_ASTnode<class_AST_ptr> ); // need to make sure this works



    // types of names
    TYPENAME_nonterm->add_production({ IDENTIFIER_term }).set_action<varType_ASTrepr_ptr>( make_typename_simple_AST );

    // this uses the symetry between LHS and RHS expressions to transform a RHS expression into a LHS reference.
    // This is used to get around the limitations of LALR(1), but still have distinct AST nodes for LHS references.
    // This will make a compiler error (not an exception, non-blocking), if the RHS expression cannot be transformed into a LHS reference
    // see notes about this function
    LHS_REFERENCE_nonterm->add_production({ EXPRESSION_nonterm }).set_action<LHSref_AST_ptr>( RHSexp_to_LHSref );



    //Statements
    STATEMENT_nonterm->add_production({ EXPRESSION_nonterm }) .set_action<statement_AST_ptr>( make_statement_expression_AST );
    STATEMENT_nonterm->add_production({ TYPENAME_nonterm, IDENTIFIER_term}) .set_action<statement_AST_ptr>( make_statement_definition_AST );
    //STATEMENT_nonterm->add_production({ IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm}) .set_action<statement_AST_ptr>( make_statement_assignment_AST ); // eventually this should use generic LHS symbols
    STATEMENT_nonterm->add_production({ LHS_REFERENCE_nonterm, EQUALS_symbol, EXPRESSION_nonterm}) .set_action<statement_AST_ptr>( make_statement_assignment_AST );
    STATEMENT_nonterm->add_production({ AUTO_keyword, IDENTIFIER_term, EQUALS_symbol, EXPRESSION_nonterm }) .set_action<statement_AST_ptr>( make_auto_definition_AST );
    STATEMENT_nonterm->add_production({ RETURN_keyword, EXPRESSION_nonterm }) .set_action<statement_AST_ptr>( make_statement_return_AST );

    //Expressions
    EXPRESSION_nonterm->add_production({ EXPRESSION_nonterm, CALL_ARGUMENTS_nonterm }) .set_action<expression_AST_ptr>( make_expression_funcCall_AST ).set_precedence();
    EXPRESSION_nonterm->add_production({ LPAREN_symbol, EXPRESSION_nonterm, RPAREN_symbol }) .set_action<expression_AST_ptr>( make_parenGrouped_expression_AST );
    EXPRESSION_nonterm->add_production({ EXPRESSION_nonterm, DOT_symbol, IDENTIFIER_term }) .set_action<expression_AST_ptr>( make_accessor_expression_AST ).set_precedence();
    EXPRESSION_nonterm->add_production({ INT_term }) .set_action<expression_AST_ptr>( make_intLiteral_AST );
    EXPRESSION_nonterm->add_production({ IDENTIFIER_term }) .set_action<expression_AST_ptr>( make_expression_varRef_AST );
    EXPRESSION_nonterm->add_production({ EXPRESSION_nonterm, PLUS_symbol, EXPRESSION_nonterm }) .set_action<expression_AST_ptr>( make_expression_addition_AST ).set_associativity( "LEFT" ).set_precedence();

    //// set other lexer actions ////
    lex_gen->add_nonreturn_pattern("\" \"");//to eat spaces
    //lex_gen->add_nonreturn_pattern("# (\" \"|[# _ 0-9 a-z A-Z ])* \\n");//comments?

    get_parser(do_file_IO); //force a build of the parser


}

std::shared_ptr<parser> make_cyth_parser::get_parser(bool do_file_IO)
{
    auto new_parser = cyth_parser_generator.get_parser<cython_lexer, parser>(do_file_IO);

    if( not new_parser )
    {
        throw gen_exception("could not generate parser.");
    }

    auto cylex = dynamic_pointer_cast<cython_lexer>( new_parser->lex );
    cylex->set_identifiers( STMT_id, NEW_BLOCK_id, END_BLOCK_id, EOF_id);

    return new_parser;
}

