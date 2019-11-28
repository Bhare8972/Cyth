/*
Copyright 2019 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file defines basic visitors that act on the AST after it is built by the parser
*/

#ifndef BASIC_AST_VISITORS_191110152455
#define BASIC_AST_VISITORS_191110152455

#include "cyth_AST.hpp"
#include "AST_visitor.hpp"

////// EXCEPTIONS ////
//class NameNotDefined_exc : public std::exception
//{
//    csu::utf8_string name;
//    csu::location_span loc;
//
//    std::string msg;
//
//public:
//    //BeforeDefinition_exc(){}
//    NameNotDefined_exc( csu::utf8_string& _name, csu::location_span& _loc);
//
//    const char* what();
//
//};


// set symbol table
class set_symbol_table : public AST_visitorTree
{
private:
    sym_table_base* symbol_table;

public:

    set_symbol_table(module_AST_node* module, csu::utf8_string module_name);
    set_symbol_table(AST_visitor_base* _parent);

    std::shared_ptr< AST_visitor_base > make_child(int number);

    void set_new_table(AST_node* ASTnode, csu::utf8_string& table_name);

    // if symbol_table is null, sets to parent if has parent
    void ASTnode_down(AST_node* ASTnode);


    void funcDef_down(function_AST_node* funcDef);

};

// defines all the names.
// may throw error is names are defined twice
class define_names : public AST_visitorTree
{
public:
    bool is_module;
    bool at_module_level;

    define_names();
    define_names(bool _at_module_level);

    std::shared_ptr< AST_visitor_base > make_child(int number);


    void cImports_up(import_C_AST_node* ASTnode); // completely verified here
    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child);

    void funcDef_down(function_AST_node* funcDef); // need to do this BEFORE block, to define arguments
};

// find where names are explicitly referenced
// throw if name is used before defined, or not defined at all
class reference_names : public AST_visitorTree
{
public:

    std::shared_ptr< AST_visitor_base > make_child(int number);


    void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr); // completely verified here

    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child);

    void varReferance_up(varReferance_expression_AST_node* varRefExp);
};


// we need to register all the overloads of different functions and methods
class register_overloads : public AST_visitorTree
{
public:
    bool is_module;
    bool at_module_level;

    register_overloads();
    register_overloads(bool _at_module_level);

    std::shared_ptr< AST_visitor_base > make_child(int number);

    void funcDef_down(function_AST_node* funcDef);
};

// now we figure out what the type of everything is
// throw error if types are incompatible in some way
class build_types : public AST_visitorTree
{
public:
    bool changes_were_made;

    build_types();

    std::shared_ptr< AST_visitor_base > make_child(int number);

    void ASTnode_up(AST_node* ASTnode); // accumulates if children have changes.

    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child); // completely verify
    void functionCall_Stmt_up(functionCall_statement_AST_node* funcCall, AST_visitor_base* expression_child);
    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child);

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp); // fully verfied
    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor);
    void varReferance_up(varReferance_expression_AST_node* varRefExp); // fully verified
};

//// BUILD symbol table
//// defines names in symbol table. (names of functions, variables, new types, etc...)
//// Sometimes requires the type of a name to be found before name can be registered, so is very iterative
//// Run multiple times until types are defined, returns true if changes are made
//class build_symbol_table : public AST_visitorTree
//{
//public:
//    bool changes_were_made;
//    bool is_module;
//    bool at_module_level;
//
//    build_symbol_table();
//    build_symbol_table(bool _at_module_level);
//
//    std::shared_ptr< AST_visitor_base > make_child(int number);
//
//    void ASTnode_up(AST_node* ASTnode); // accumulates if children have changes.
//
//    // defines new names
//    void cImports_up(import_C_AST_node* ASTnode);
//    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child);
//
//    void funcDef_down(function_AST_node* funcDef);
//
//    // refers to names
//    void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr);
//
//    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child);
//    void functionCall_Stmt_up(functionCall_statement_AST_node* funcCall, AST_visitor_base* expression_child);
//
//    void intLiteral_up(intLiteral_expression_AST_node* intLitExp);
//    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor);
//    void varReferance_up(varReferance_expression_AST_node* varRefExp);
//
//};




// verify symbol table
// by the time we get here the AST nodes are in three classes
// 1) nodes that have already been verified
// 2) nodes that need to be verified
// 3) nodes that are verified via acclimation of children
class verify_symbol_table : public AST_visitorTree
{
public:
    bool is_verified; // true if this node doesn't need to check children for verification
    bool do_acclimation; // default is true. Set to False on way down if not.
    int verification_state; // set in ASTnode_down or ASTnode_up accordingly

    verify_symbol_table();
    std::shared_ptr< AST_visitor_base > make_child(int number);

    void ASTnode_down(AST_node* ASTnode)
    {
        if( ASTnode->verification_state!=-1 )
        {
            is_verified = true;
            verification_state = ASTnode->verification_state;
        }
    }
    // Note this is called immediatly after ASTnode_down
    bool apply_to_children(){ return not is_verified; } // only do children if not verified

    // acclimate children unless (do_acclimation has been set to false) or is_verified.
    void ASTnode_up(AST_node* ASTnode);


    void fail_if_not_verified( AST_node* ASTnode )
    {
        if( ASTnode->verification_state == -1 )
        {
            ASTnode->verification_state = 0;
        }
    }

    void assignmentStmt_down(assignment_statement_AST_node* assignStmt){ fail_if_not_verified(assignStmt); }
    void functionCall_Stmt_down(functionCall_statement_AST_node* funcCall){ fail_if_not_verified(funcCall); }
    void binOperator_down(binOperator_expression_AST_node* binOprExp){ fail_if_not_verified(binOprExp); }
};

#endif // BASIC_AST_VISITORS_191110152455

