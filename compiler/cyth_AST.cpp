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

This file defines the Cyth abstract sytax tree
*/

#include <fstream>
#include <memory>
#include <algorithm>

#include "cyth_AST.hpp"
#include "AST_visitor.hpp"

using namespace std;
using namespace csu;


/////// basic AST node buisness /////

AST_node::AST_node()
{
    symbol_table = 0;
    verification_state = -1;
}

void AST_node::apply_visitor(AST_visitor_base* visitor)
{
    throw "not implemented";
}

////// module  ///////

//module_AST_node::module_AST_node( csu::utf8_string name ) :
//    top_symbol_table( name cle)
module_AST_node::module_AST_node()
{
    //module_name = name;
    symbol_table = &top_symbol_table;
    max_symbol_loops = 1000; //probably define this elsewhere.
}

void module_AST_node::add_AST_node(AST_node_ptr new_AST_element)
{
    module_contents.push_back( new_AST_element );
    loc = loc + new_AST_element->loc;
}

void module_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->module_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( module_contents.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;
    for( auto& node :  module_contents)
    {
        AST_visitor_base* child = visitor->get_child(i);
        visitor_children.push_back( child );
        node->apply_visitor( child );
        i ++;
    }

    visitor->ASTnode_up( this );
    visitor->module_up( this, visitor_children );


}


///// IMPORTS /////

//import from C
import_C_AST_node::import_C_AST_node(csu::utf8_string _import_name, csu::location_span _loc)
{
    import_name = _import_name;
    usage_name = _import_name;
    loc = _loc;

    type = nullptr;
    variable = nullptr;
    variable_type = nullptr;
}

void import_C_AST_node::set_usage_name(csu::utf8_string _usage_name)
{
    usage_name = _usage_name;
}


void import_C_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->allImports_down( this );
    visitor->cImports_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->ASTnode_up( this );
    visitor->cImports_up( this );
    visitor->allImports_up( this );
}

///// OTHERS ////

// block
block_AST_node::block_AST_node(csu::location_span initial_loc)
{
    loc = initial_loc;
}

void block_AST_node::add_AST_node(AST_node_ptr new_AST_element, csu::location_span _loc)
{
    contents.push_back( new_AST_element );
    loc = loc + _loc;
}

void block_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->block_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    visitor->initiate_children( contents.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;

    for(auto child : contents)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        visitor_children.push_back( child_visitor );
        child->apply_visitor( child_visitor );
        i += 1;
    }

    visitor->ASTnode_up( this );
    visitor->block_up( this, visitor_children );
}

// function
function_AST_node::function_AST_node(utf8_string _name, location_span _loc, block_AST_ptr _block)
{
    name = _name;
    loc = _loc;

    block_AST = _block;

    funcName = nullptr;
    funcType = nullptr;
    specific_overload = nullptr;

    inner_symbol_table = nullptr;
}

void function_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->funcDef_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0) ;
    block_AST->apply_visitor( child );

    visitor->ASTnode_up( this );
    visitor->funcDef_up( this, child );
}


////// names //////

// define type of a variable
varType_ASTrepr_node::varType_ASTrepr_node(csu::utf8_string _name, csu::location_span _loc)
{
    name = _name;
    loc = _loc;
    resolved_type = nullptr;
}

void varType_ASTrepr_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->varTypeRepr_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->ASTnode_up( this );
    visitor->varTypeRepr_up( this );

}




////// statements ////////

// expression statement
expression_statement_AST_node::expression_statement_AST_node(expression_AST_ptr _expression)
{
    //type_of_statement = statement_AST_node::expression_t;
    expression = _expression;
    loc = expression->loc;
}

void expression_statement_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->expressionStatement_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0) ;
    expression->apply_visitor( child );

    visitor->ASTnode_up( this );
    visitor->statement_up( this );
    visitor->expressionStatement_up( this, child );

}


// definition
definition_statement_AST_node::definition_statement_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name, csu::location_span _loc)
{
    //type_of_statement = statement_AST_node::definition_t;

    var_type = _var_type;
    var_name = _var_name;
    loc = _loc;
    variable_symbol = nullptr;
}

void definition_statement_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->definitionStmt_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0);
    var_type->apply_visitor( child );

    visitor->ASTnode_up( this );
    visitor->statement_up( this );
    visitor->definitionStmt_up( this, child );

}


// assignment
assignment_statement_AST_node::assignment_statement_AST_node( csu::utf8_string _var_name, expression_AST_ptr _expression, csu::location_span _loc)
{
    //type_of_statement = statement_AST_node::definition_t;
    var_name = _var_name;
    loc = _loc;
    expression = _expression;
    variable_symbol = nullptr;
}

void assignment_statement_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->assignmentStmt_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0) ;
    expression->apply_visitor( child );

    visitor->ASTnode_up( this );
    visitor->statement_up( this );
    visitor->assignmentStmt_up( this, child );

}

// function call (eventually to be an expression
functionCall_statement_AST_node::functionCall_statement_AST_node( expression_AST_ptr _expression, csu::location_span _loc)
{
    loc = _loc;
    expression = _expression;
    function_to_write = nullptr;
}

void functionCall_statement_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->functionCall_Stmt_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0) ;
    expression->apply_visitor( child );

    visitor->ASTnode_up( this );
    visitor->statement_up( this );
    visitor->functionCall_Stmt_up( this, child );
}


/////  expressions ///////

expression_AST_node::expression_AST_node()
{
    expression_return_type = nullptr;
    //return_type_is_unnamed = true;
}

// int literal
intLiteral_expression_AST_node::intLiteral_expression_AST_node(utf8_string _literal, location_span _loc)
{
    //type_of_expression = expression_AST_node::int_literal_t;
    literal = _literal;
    loc = _loc;
}

void intLiteral_expression_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->intLiteral_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->ASTnode_up( this );
    visitor->expression_up( this );
    visitor->intLiteral_up( this );

}

// binary operator
binOperator_expression_AST_node::binOperator_expression_AST_node(expression_AST_ptr _left_operand, expression_type _type, expression_AST_ptr _right_operand)
{
    //type_of_expression = expression_AST_node::binary_operator_t;
    left_operand = _left_operand;
    type_of_operation = _type;
    right_operand = _right_operand;
    loc = left_operand->loc + right_operand->loc;
}

void binOperator_expression_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->binOperator_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );
    AST_visitor_base* LHS_child = visitor->get_child(0) ;
    left_operand->apply_visitor( LHS_child );
    AST_visitor_base* RHS_child = visitor->get_child(1) ;
    right_operand->apply_visitor( RHS_child );

    visitor->ASTnode_up( this );
    visitor->expression_down( this );
    visitor->binOperator_up( this, LHS_child, RHS_child );

}

//variable name reference
varReferance_expression_AST_node::varReferance_expression_AST_node(csu::utf8_string _var_name, csu::location_span _loc)
{
    var_name = _var_name;
    variable_symbol = nullptr;
    loc = _loc;
}

void varReferance_expression_AST_node::apply_visitor(AST_visitor_base* visitor)
{
    visitor->varReferance_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->ASTnode_up( this );
    visitor->expression_down( this );
    visitor->varReferance_up( this );

}




