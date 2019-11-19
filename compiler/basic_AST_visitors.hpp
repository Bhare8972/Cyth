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
    //void funcDef_down(function_AST_node* funcDef)
    //{  set_new_table(funcDef, funcDef->name); }
};


// BUILD symbol table
// defines names in symbol table. (names of functions, variables, new types, etc...)
// Sometimes requires the type of a name to be found before name can be registered, so is very iterative
// Run multiple times until types are defined, returns true if changes are made
class build_symbol_table : public AST_visitorTree
{
public:
    bool changes_were_made;
    bool is_module;
    bool at_module_level;

    build_symbol_table();
    build_symbol_table(bool _at_module_level);

    std::shared_ptr< AST_visitor_base > make_child(int number);

    void ASTnode_up(AST_node* ASTnode); // accumulates if children have changes.

    // defines new names
    void cImports_up(import_C_AST_node* ASTnode);
    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child);

    void funcDef_down(function_AST_node* funcDef); // need to do this BEFORE block, to define arguments

    // refers to names
    void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr);

    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child);

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp);
    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor);
    void varReferance_up(varReferance_expression_AST_node* varRefExp);

};


// verify symbol table
class verify_symbol_table : public AST_visitorTraveler
{
public:
    bool do_error_reporting;
    bool has_been_verified;

    verify_symbol_table(bool _do_error_reporting){ do_error_reporting=_do_error_reporting; }

    // no need to verify twice.
    void ASTnode_down(AST_node* ASTnode){ has_been_verified = ASTnode->symbol_table_verified;}
    // Note this is called immediatly after ASTnode_down
    bool apply_to_children(){ return not has_been_verified; }


    void module_up(module_AST_node* module);
    void cImports_up(import_C_AST_node* ASTnode);
    void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr);

    void funcDef_up(function_AST_node* funcDef, AST_visitor_base* stmt_child);

    void expressionStatement_up(expression_statement_AST_node* expStmt);
    void definitionStmt_up(definition_statement_AST_node* defStmt);
    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child);

    void functionCall_Stmt_up(functionCall_statement_AST_node* funcCall, AST_visitor_base* expression_child);

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp);
    void binOperator_up(binOperator_expression_AST_node* binOprExp);
    void varReferance_up(varReferance_expression_AST_node* varRefExp);
};

#endif // BASIC_AST_VISITORS_191110152455

