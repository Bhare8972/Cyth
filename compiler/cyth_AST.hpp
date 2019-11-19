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

This file defines the Cyth abstract syntax tree
*/

#include <list>
#include <memory>
#include <exception>

#ifndef CYTH_AST_170212090415
#define CYTH_AST_170212090415

#include "sym_table.hpp"

#include "parser.hpp"

//// type defnitions ////
typedef int funcCall_AST_ptr;
typedef int funcArgument_AST_ptr;


class AST_visitor_base;

class AST_node;
typedef std::shared_ptr<AST_node> AST_node_ptr;

class module_AST_node;
typedef std::shared_ptr<module_AST_node> module_AST_ptr;

class import_AST_node;
typedef std::shared_ptr<import_AST_node> import_AST_ptr;

class block_AST_node;
typedef std::shared_ptr<block_AST_node> block_AST_ptr;

class function_AST_node;
typedef std::shared_ptr<function_AST_node> function_AST_ptr;

class varType_ASTrepr_node;
typedef std::shared_ptr<varType_ASTrepr_node> varType_ASTrepr_ptr;

class statement_AST_node;
typedef std::shared_ptr<statement_AST_node> statement_AST_ptr;

class expression_AST_node;
typedef std::shared_ptr<expression_AST_node> expression_AST_ptr;


//// exceptions ////
//class operation_DNE : public std::exception
//{
//    csu::utf8_string operation_name;
//    csu::utf8_string LHS_type;
//    csu::utf8_string RHS_type;
//    csu::location_span invoke_loc;
//
//    std::string msg;
//
//public:
//    operation_DNE(){}
//    operation_DNE( csu::utf8_string _operation_name, csu::utf8_string& _LHS_type,
//                         csu::utf8_string& _RHS_type, csu::location_span& _invoke_loc);
//
//    const char* what();
//
//};


//// the AST nodes ////
class AST_node
{
public:
    csu::location_span loc;
    sym_table_base* symbol_table;
    bool symbol_table_verified;

    AST_node();

    // NOTE: these need to be depreciated and replaced with visitors
//    virtual void set_symbol_table(sym_table_base* upper_sym_table); // this sets the namespace of each AST node

    //virtual bool build_symbol_table();
    // defines names in symbol table. (names of functions, variables, new types, etc...)
    // Sometimes requires the type of a name to be found before name can be registered, so is very iterative
    // Run multiple times until types are defined, returns true if changes are made



//    virtual bool verify_symbol_table(); //returns false if not all symbols could be defined. MAKES NO CHANGES. Could be ran from above operations
    // needs to have ability to print WHY something can't be defined


    /*
    virtual void set_symbol_table(sym_table_base* upper_sym_table);
    virtual bool build_symbol_table(); //returns if changes have been made, new terms could be defined. Includes finding type of expression
    virtual bool verify_symbol_table(); //returns false if not all symbols could be defined. MAKES NO CHANGES
    */

    virtual void apply_visitor(AST_visitor_base* visitor);
};


class module_AST_node : public AST_node
{
public:
    std::list<AST_node_ptr> module_contents;
    module_sym_table top_symbol_table;
    int max_symbol_loops; //eventully compiler commands could change this
    csu::utf8_string module_name;

    module_AST_node();

    void add_AST_node(AST_node_ptr new_AST_element);


    //void set_symbol_table(csu::utf8_string name);
    //void set_symbol_table();
    //bool build_symbol_table();
//    bool verify_symbol_table();

    void apply_visitor(AST_visitor_base* visitor);
};

// this is the parent type for all imports
// right now there are two types, from c and cyth (ostensibly)
class import_AST_node : public AST_node
{
    private:

    public:
    csu::utf8_string import_name; // the name imported
    csu::utf8_string usage_name; // the name used in this module

    import_AST_node(){}
    virtual ~import_AST_node(){}


    virtual void set_usage_name(csu::utf8_string _usage_name)=0;
};

// this is a C import, we do not know if this is variable, type, or type if it is a variable.
// Therefore, if this is a C import, it defines three things 1) a variable of this name, 2) a C type of this name, and 3) a C type with unknown name, that is type of the variable that DOES have this name..
// try to never be in the situation that you need to actually know the name of the type of the variable you are importing...
class import_C_AST_node : public import_AST_node
{
public:
    varType_ptr type;
    varName_ptr variable;
    varType_ptr variable_type; // the unnamed type of the variable. This is not placed in the namespace

    import_C_AST_node(csu::utf8_string _import_name, csu::location_span _loc);
    void set_usage_name(csu::utf8_string _usage_name);

    //void set_symbol_table(sym_table_base* upper_sym_table);
    //bool build_symbol_table();
//    bool verify_symbol_table();

    void apply_visitor(AST_visitor_base* visitor);
};

TODO: define class below
class block_AST_node : public AST_node
{
public:
    std::list<AST_node_ptr> contents;

    block_AST_node(csu::location_span initial_loc);

    void add_AST_node(AST_node_ptr new_AST_element, csu::location_span loc);

    void apply_visitor(AST_visitor_base* visitor);
};


TODO: fix function!!
//// functions ////
class function_AST_node : public AST_node
{
public:

    csu::utf8_string name;
    //parameter_list_ptr parameter_list;
    block_AST_ptr block_AST;
    // return type

    varName_ptr funcName;
    DefFuncType_ptr funcType; // note: this is NOT stored in the symbol table, as it is NOT a symbol!
    ResolvedFunction_ptr specific_overload; // note that THIS is the approprate C_name to refer to this function!

    function_AST_node(csu::utf8_string _name, csu::location_span _loc, block_AST_ptr _block);
//    function_AST_node(csu::utf8_string _name, csu::location_span _loc, parameter_list_ptr _parameter_list, block_AST_ptr _block_AST);

    void apply_visitor(AST_visitor_base* visitor);
};




//// types of names /////

// variable types //
class varType_ASTrepr_node : public AST_node
{
public:
    csu::utf8_string name;//// NOTE: the 'name' of a type will need to be generalized away from a string
    varType_ptr resolved_type;

    varType_ASTrepr_node(csu::utf8_string _name, csu::location_span _loc);

    //void set_symbol_table(sym_table_base* upper_sym_table);
    //bool build_symbol_table();
//    bool verify_symbol_table();

    void apply_visitor(AST_visitor_base* visitor);
};




//// statements ////
class statement_AST_node : public AST_node
{
public:
    /*
    enum statement_type
    {
        empty,
        expression_t,
        definition_t,
    };

    statement_type type_of_statement;*/
};

class expression_statement_AST_node : public statement_AST_node
{
public:
    expression_AST_ptr expression;
    expression_statement_AST_node(expression_AST_ptr _expression);
    void apply_visitor(AST_visitor_base* visitor);
};

class definition_statement_AST_node : public statement_AST_node
{
public:
    varType_ASTrepr_ptr var_type;
    csu::utf8_string var_name;
    varName_ptr variable_symbol;

    definition_statement_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name, csu::location_span _loc);

    void apply_visitor(AST_visitor_base* visitor);
};

class assignment_statement_AST_node : public statement_AST_node
{
public:
    csu::utf8_string var_name;
    varName_ptr variable_symbol;
    expression_AST_ptr expression;

    csu::utf8_string operation_function_name;

    assignment_statement_AST_node( csu::utf8_string _var_name, expression_AST_ptr _expression, csu::location_span _loc);

    void apply_visitor(AST_visitor_base* visitor);
};

class functionCall_statement_AST_node : public statement_AST_node
{
public:
    expression_AST_ptr expression;
    ResolvedFunction_ptr specific_overload;

    functionCall_statement_AST_node( expression_AST_ptr _expression, csu::location_span _loc);

    void apply_visitor(AST_visitor_base* visitor);
};


//// expressions ////

class expression_AST_node : public AST_node
{
public:
    /*
    enum expression_type
    {
        empty,
        func_call_t,
        int_literal_t,
        reference_t,
        binary_operator_t
    };
    expression_type type_of_expression;*/
    varType_ptr expression_return_type;
    bool return_type_is_unnamed; // the type that hath no name!!

    expression_AST_node();
};

class intLiteral_expression_AST_node : public expression_AST_node
{
public:
    csu::utf8_string literal;

    intLiteral_expression_AST_node(csu::utf8_string _literal, csu::location_span _loc);

    void apply_visitor(AST_visitor_base* visitor);
};

// NOTE: need to sub-class this in future for other operations, or something
class binOperator_expression_AST_node : public expression_AST_node
{
public:
    enum expression_type
    {
        empty,
        addition_t,
    };
    expression_type type_of_operation;

    expression_AST_ptr left_operand;
    expression_AST_ptr right_operand;

    csu::utf8_string operation_function_name;

    binOperator_expression_AST_node(expression_AST_ptr _left_operand, expression_type _type, expression_AST_ptr _right_operand);


    void apply_visitor(AST_visitor_base* visitor);

};

class varReferance_expression_AST_node : public expression_AST_node
{
public:
    csu::utf8_string var_name;
    varName_ptr variable_symbol;

    varReferance_expression_AST_node(csu::utf8_string _var_name, csu::location_span _loc);

    void apply_visitor(AST_visitor_base* visitor);
};

//class funcCall_expression_AST_node : public expression_AST_node
//{
//public:
//    class funcCall_argument_list
//    {
//
//    };
//
//};
//
//class reference_expression_AST_node : public expression_AST_node
//{
//
//};







#endif
