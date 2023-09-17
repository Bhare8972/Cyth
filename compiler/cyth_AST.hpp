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


// NOTE: we will need some more powerful AST modifcation tools to allow for optimizations. Perhap we need 1: paterns that recognize certain AST cases (could be useful in many situations_. and 2) modifications that change how
// a visitor behaves if it see a particular AST node. maybe 3) an easy way to completely swap out AST nodes? (this sounds hard)

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



class class_AST_node;
typedef std::shared_ptr<class_AST_node> class_AST_ptr;

class inheritanceList_AST_node;
typedef std::shared_ptr<inheritanceList_AST_node> inheritanceList_AST_ptr;

class construct_AST_node;
typedef std::shared_ptr<construct_AST_node> construct_AST_ptr;

class constructElement_AST_node;
typedef std::shared_ptr<constructElement_AST_node> constructElement_AST_ptr;

class class_varDefinition_AST_node;
typedef std::shared_ptr<class_varDefinition_AST_node> ClassVarDef_AST_ptr;



class block_AST_node;
typedef std::shared_ptr<block_AST_node> block_AST_ptr;

class function_AST_node;
typedef std::shared_ptr<function_AST_node> function_AST_ptr;

class function_parameter_list;
typedef std::shared_ptr<function_parameter_list> paramList_AST_ptr;

class method_AST_node;
typedef std::shared_ptr<method_AST_node> method_AST_ptr;

class call_argument_list;
typedef std::shared_ptr<call_argument_list> argumentList_AST_ptr;


class conditional_AST_node;
typedef std::shared_ptr<conditional_AST_node> conditional_AST_ptr;

class loop_AST_node;
typedef std::shared_ptr<loop_AST_node> loop_AST_ptr;



class varType_ASTrepr_node;
typedef std::shared_ptr<varType_ASTrepr_node> varType_ASTrepr_ptr;

class statement_AST_node;
typedef std::shared_ptr<statement_AST_node> statement_AST_ptr;

class LHS_reference_AST_node;
typedef std::shared_ptr<LHS_reference_AST_node> LHSref_AST_ptr;

class expression_AST_node;
typedef std::shared_ptr<expression_AST_node> expression_AST_ptr;


//// the AST nodes ////
class AST_node : public std::enable_shared_from_this<AST_node> // TODO! use shared_from_this()
{
public:
    csu::location_span loc;
    sym_table_base* symbol_table;
    int verification_state; // -1 means not verified yet (initial state). 0 (false) means bad. 1 (true) means good.
    // different nodes are verified in different spots. a verification_state of 1 implies children have verification_state of 1.
    // TODO: change this to an enum?

    AST_node* parent_node; // set to null in constructor.
    // this is set by set_symbol_table visitor!

    AST_node();


    void apply_visitor(AST_visitor_base* visitor); // this just has an exception catcher and calls function below
    virtual void apply_visitor_inner(AST_visitor_base* visitor);

    virtual std::string AST_node_name()=0;
};


class module_AST_node : public AST_node
{
    // verified in verify_symbol_table visitor (acclimate from children)
public:
    class compiler_command
    {
    public:
        compiler_command(std::string _command, csu::location_span _loc);

        std::string command;
        csu::location_span loc;
    };


    std::list<AST_node_ptr> module_contents;
    module_sym_table top_symbol_table;
    int max_symbol_loops; //eventully compiler commands could change this

    std::string module_name; // ONLY used for printing??. Identify  module by file_name
    std::string file_name; // canonical absolute location of the module cyth file
    std::string C_header_fname;
    std::string C_source_fname;
    std::string C_object_fname; // only set if object succsessfully compiled

    int main_status; // -2 is default (not checked)
    // after running find_main_func, set to -1 if problem, 0 if no __main__, 1 if good __main__
    csu::utf8_string main_func_name;
    std::string C_mainSource_fname;

    // these are found and set by the parser/module manager
    std::list< std::string > imported_C_files;
    std::list< std::string > imported_cyth_modules;
    std::list< std::string > object_fnames_to_link;// set by module manager when this is compiled to object file.
        // Includes cyth modules and their dependencies

    module_AST_node();
    std::string AST_node_name() override { return "module_ASTnode"; }

    void add_AST_node(AST_node_ptr new_AST_element);

    void apply_visitor_inner(AST_visitor_base* visitor) override;

    void find_main_func();

    void add_CompilerCommand( csu::utf8_string _command, csu::location_span _loc );
    std::list< compiler_command > compiler_comands;
};





/// IMPORTS
class import_AST_node : public AST_node
{
    private:

    public:
    csu::utf8_string import_name; // the name imported
    csu::utf8_string usage_name; // the name used in this module


    virtual void set_usage_name(csu::utf8_string _usage_name)=0;
};

// this is a C import, we do not know if this is variable, type, or type if it is a variable.
// Therefore, if this is a C import, it defines three things 1) a variable of this name, 2) a C type of this name, and 3) a C type with unknown name, that is type of the variable that DOES have this name..
// try to never be in the situation that you need to actually know the name of the type of the variable you are importing...
class import_C_AST_node : public import_AST_node
{
    // verified in set_symbol_table visitor
public:
    csu::utf8_string import_file_name; // the name imported in the cyth file
    std::string found_file_location; // set by module manager before compiling

    // these are fully defined during the define_names visitor
    varType_ptr type;
    varName_ptr variable;
    varType_ptr variable_type; // the unnamed type of the variable. This is not placed in the namespace
    // note variable_type is actually a referance, not a definition. Though still set in define_names visitor

    import_C_AST_node(csu::utf8_string _file_name, csu::utf8_string _import_name, csu::location_span _loc);
    import_C_AST_node(csu::utf8_string _import_name, csu::location_span _loc);
    void set_usage_name(csu::utf8_string _usage_name);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "Cimport_ASTnode"; }
};


class import_cyth_AST_node : public import_AST_node
{
    // verified in set_symbol_table visitor
public:
    csu::utf8_string import_module_string;
    csu::utf8_string imported_module_fname; // set by module manager

    csu::utf8_string import_module_Cheader_fname; // set in define_names visitor

    // these are fully defined during the define_names visitor
    //varType_ptr type;
    //varName_ptr variable;
    //varType_ptr variable_type; // the unnamed type of the variable. This is not placed in the namespace
    // note variable_type is actually a referance, not a definition. Though still set in define_names visitor

    import_cyth_AST_node(csu::utf8_string _import_module_string, csu::utf8_string _import_name, csu::location_span _loc);
    //import_cyth_AST_node(csu::utf8_string _import_name, csu::location_span _loc);
    void set_usage_name(csu::utf8_string _usage_name);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "CythImport_ASTnode"; }
};







/// some needed statment things ///
class statement_AST_node : public AST_node // NOTE: these are single-line statments. Does not include control structures like loops, conditionals, functions, etc...
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

class General_VarDefinition : public statement_AST_node
{
public:
    csu::utf8_string var_name;
    varName_ptr variable_symbol;

    General_VarDefinition();
};

class Typed_VarDefinition : public General_VarDefinition
{
public:
    varType_ASTrepr_ptr var_type;
};








/// CLASSES

class class_varDefinition_AST_node : public Typed_VarDefinition
{
public:
    expression_AST_ptr default_value; // note is AST node.
    // maybe need to move this to general definition??

    class_varDefinition_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name, csu::location_span _loc);
    class_varDefinition_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name, expression_AST_ptr _default_value, csu::location_span _loc);


    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "ClassVarDef_ASTnode"; }
};

class inheritanceList_AST_node : public AST_node
{
public:
    std::list< csu::utf8_string > class_IDs;
    std::list< ClassType_ptr > types; // set by visitors

    inheritanceList_AST_node(){}
    void add_item(csu::utf8_string &item, csu::location_span &_loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "inheritanceList_ASTnode"; }
};

class class_AST_node : public AST_node
{
    // verified?
public:
    csu::utf8_string name;
    sym_table_ptr inner_symbol_table;
    ClassType_ptr type_ptr; // set in define_names visitor
    varName_ptr self_name; // note, this is NOT same type as  type_ptr, is some kind of reference type

    inheritanceList_AST_ptr inheritanceList;

    bool write_default_constructor;
    MethodType::ResolvedMethod_ptr default_constructor_overload; // set if and only if write_default_constructor is true

    bool write_default_deconstructor;
    MethodType::ResolvedMethod_ptr default_destructor_overload;

    bool write_selfCopy_constructor;
    MethodType::ResolvedMethod_ptr default_CopyConstructor_overload;

    std::list< std::pair<MethodType::ResolvedMethod_ptr, varName_ptr> > assignments_to_default;

    std::list<ClassVarDef_AST_ptr> var_definitions;
    std::list<method_AST_ptr> method_definitions;

    class_AST_node(csu::utf8_string _name, inheritanceList_AST_ptr _inheritanceList, csu::location_span _loc);
    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "class_ASTnode"; }


    void add_var_def(ClassVarDef_AST_ptr new_variable_definition);
    void add_method_def(method_AST_ptr new_method_definition);
};

// construct block
class construct_AST_node : public AST_node
{
    // verified in verify_symbol_table visitor (acclimate from children)
public:
    std::list<constructElement_AST_ptr> contents;

    construct_AST_node(csu::location_span initial_loc);

    void add(constructElement_AST_ptr _newElement);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "construct_ASTnode"; }
};

class constructElement_AST_node : public AST_node
{
    // verified in verify_symbol_table visitor (acclimate from children)
public:

    expression_AST_ptr expression;
    argumentList_AST_ptr argument_list;

    constructElement_AST_node(expression_AST_ptr _expression,  argumentList_AST_ptr _argument_list, csu::location_span initial_loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "constructElement_ASTnode"; }
};




/// FUNCTIONS
class block_AST_node : public AST_node
{
    // verified in verify_symbol_table visitor (acclimate from children)
public:

    std::list<AST_node_ptr> contents;

    block_AST_node(csu::location_span initial_loc);

    void add_AST_node(AST_node_ptr new_AST_element, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "block_ASTnode"; }
};

//// functions ////
class callableDefinition_AST_node : public AST_node
/// callable things define parameters, take arguments, and potentially return a value, I think
// partially verify in build_types, amoung other complex acrobatics
{
public:
    enum returnMode_T
    // this is the mode by which the return type is determined
    {
        by_returnStatement_M, //default
        // determine by return statement
        //   if no return statement, return void
        //   if all static, require all to have same T
        //   OTHER NOT IMPLEMENTED:
        //   if mix of static and dynamic. Require static to all have same type. Return var{ static_type }. Require static to be
        //       convertabel soley by going up inhertance.
        //   if all var{static_type}, and all static_type is equivalent, return var{static_type}.
        //   if mix of var, and var{T} of different T, then just return var
        by_explicitDefinition_M,
        //   return type is explcitly defined. All return statements will attempt to convert to this via implicit conversions only
    };
    sym_table_ptr inner_symbol_table;

    paramList_AST_ptr paramList;
    varType_ptr return_type;
    returnMode_T returnType_mode;

    int num_returns; // set in register_overloads
    int num_verified_returns; // set in build_types
    //-1 means at least one is bad, and return type will never be found.
    // once these two numbers are equal, return_type can be found, and notify_return_type is called
    //  when notify_return_type, this is also set to -2 so notify_return_type isn't called again

    callableDefinition_AST_node();

    virtual void notify_return_type()=0; // this is called in build_types visitor exactly once, after we know the return type
};

class function_AST_node : public callableDefinition_AST_node
{
    // partially verified in register_overloads visitor
    // partially verify in build_types, via callableDefinition_AST_node
    // verified in verify_symbol_table visitor (acclimate from children)
public:

    csu::utf8_string name;
    block_AST_ptr block_AST;
    varType_ASTrepr_ptr return_type_ASTnode; // may be null if not specified

    varName_ptr funcName; // partially set in define_names, type set in register_overloads
    // these are set in register_overloads visitor
    DefFuncType_ptr funcType; // note: this is NOT stored in the symbol table, as it is NOT a symbol!
    DefFuncType::ResolvedFunction_ptr specific_overload; // note that THIS is the approprate C_name to refer to this function!

    function_AST_node(csu::utf8_string _name, csu::location_span _loc, paramList_AST_ptr _paramList, block_AST_ptr _block);
    function_AST_node(csu::utf8_string _name, varType_ASTrepr_ptr _return_type_ASTnode, csu::location_span _loc,
                      paramList_AST_ptr _paramList, block_AST_ptr _block);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "function_ASTnode"; }

    void notify_return_type() override; // called once return type is set
};

class function_parameter_list : public AST_node
{
// partially verified in register_overloads
// defaulted_params and required_params potentially verified in build_types
// verification completed in verify_symbol_table
public:

    class base_parameters_T : public AST_node
    {
    public:
        class param_holder
        {
        public:
            varType_ASTrepr_ptr var_type_ASTnode; // AST child
            csu::utf8_string var_name;
            varName_ptr variable_symbol; // use this for the type!! once its defined???
            csu::location_span loc;
            //bool is_auto;
        };


        std::list<param_holder> param_list;
    };

    class required_params : public base_parameters_T
    {
    public:

        required_params( csu::location_span _loc );
        void add_typed_param( varType_ASTrepr_ptr _var_type, csu::utf8_string& _var_name, csu::location_span _loc );

        void apply_visitor_inner(AST_visitor_base* visitor) override;
        std::string AST_node_name() override { return "reqParams_ASTnode"; }
    };
    typedef std::shared_ptr<required_params> required_ptr;


    class defaulted_params : public base_parameters_T
    {
    public:
        std::list<expression_AST_ptr> parameter_defaults; // these are all AST children

        defaulted_params( csu::location_span _loc );

        void add_typed_param( varType_ASTrepr_ptr _var_type, csu::utf8_string& _var_name, expression_AST_ptr default_exp, csu::location_span _loc );
        void apply_visitor_inner(AST_visitor_base* visitor) override;
        std::string AST_node_name() override { return "defParams_ASTnode"; }
    };
    typedef std::shared_ptr<defaulted_params> default_ptr;


    //NOTE: these are AST children, and they have AST children  (can be null!)
    required_ptr required_list;
    default_ptr defaulted_list;

    function_parameter_list(csu::location_span _loc, required_ptr _required_list, default_ptr _defaulted_list); // these can be null as necisary.
    func_param_ptr get_parameter_info();


    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "funcParameters_ASTnode"; }
};

// methods
class method_AST_node : public callableDefinition_AST_node
{
public:

    csu::utf8_string name;
    block_AST_ptr block_AST;
    varType_ASTrepr_ptr return_type_ASTnode; // may be null if not specified

    varName_ptr methodName; // partially set in define_names, type set in register_overloads
    // these are set in register_overloads visitor
    MethodType_ptr funcType; // note: this is NOT stored in the symbol table, as it is NOT a symbol!
    MethodType::ResolvedMethod_ptr specific_overload; // note that THIS is the approprate C_name to refer to this!

    //varName_ptr self_name; // defined name "self" refering to the class. Oddly, set in set_symbol_table visitor
        // type is C-pointer to class_type, and thus type is set when class_type is.
    varType_ptr class_type;// type of class that this is member of. Set in define_names by ClassDef_down visitor

    method_AST_node(csu::utf8_string _name, csu::location_span _loc, paramList_AST_ptr _paramList, block_AST_ptr _block);
    method_AST_node(csu::utf8_string _name, varType_ASTrepr_ptr _return_type_ASTnode, csu::location_span _loc,
                      paramList_AST_ptr _paramList, block_AST_ptr _block);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "method_ASTnode"; }

    void notify_return_type() override; // called once return type is set
};





/// types of names ///

// variable types //
class varType_ASTrepr_node : public AST_node
{
    // fully verified in reference_names
public:
    csu::utf8_string name;//// NOTE: the 'name' of a type will need to be generalized away from a string
    // fully set in reference_names visitor
    varType_ptr resolved_type;

    varType_ASTrepr_node(csu::utf8_string _name, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "varType_ASTnode"; }
};





/// conditionals ///
class flowControl_AST_node : public AST_node // includes loops and conditionals
{
public:
    block_AST_ptr block_AST;
    sym_table_ptr inner_symbol_table;
};

class conditional_AST_node : public flowControl_AST_node
{
    // if, elif, else
};

class activeCond_AST_node : public conditional_AST_node //if or elif
{
public:
    expression_AST_ptr if_expression;
    conditional_AST_ptr child_conditional;

    activeCond_AST_node(expression_AST_ptr _if_expression, block_AST_ptr _block_AST, conditional_AST_ptr _child_conditional);
};

class if_AST_node : public activeCond_AST_node
{
public:
    if_AST_node(expression_AST_ptr _if_expression, block_AST_ptr block_AST, conditional_AST_ptr _child_conditional, csu::location_span _loc );

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "if_AST_node"; }
};

class elif_AST_node : public activeCond_AST_node
{
public:
    elif_AST_node(expression_AST_ptr _if_expression, block_AST_ptr block_AST, conditional_AST_ptr _child_conditional, csu::location_span _loc );

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "elif_AST_node"; }
};

class else_AST_node : public conditional_AST_node
{
public:

    else_AST_node(block_AST_ptr _block_AST, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "else_AST_node"; }
};




/// loops ///

class loopCntrl_statement_AST_node;

class loop_AST_node : public flowControl_AST_node
{
public:
    struct loop_list_node // sort of a linked list connecting loops to be controled by this statement
    {
        // see loopCntrl_statement_AST_node for termonolgy description
        loopCntrl_statement_AST_node* cntrl_node; // this object;
        loop_AST_node* current_loop; // this loop owns this object
        loop_list_node* next; // next lower loop. If this is null than this loop is the terminal loop
    };

    std::list<loop_list_node> control_statments; // break and continue statemtns that affect this node
};

class whileLoop_AST_node : public loop_AST_node
{
public:
    expression_AST_ptr while_expression;

    whileLoop_AST_node(block_AST_ptr _block_AST, expression_AST_ptr _whileExp, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "whileLoop_AST_node"; }
};

class forLoop_AST_node : public loop_AST_node
{
public:
    expression_AST_ptr while_expression;
    statement_AST_ptr initial_statement;
    statement_AST_ptr update_statement;

    forLoop_AST_node(block_AST_ptr _block_AST, expression_AST_ptr _whileExp, statement_AST_ptr _initial_statement, statement_AST_ptr _update_statement, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "forLoop_AST_node"; }
};





/// statements ///

class expression_statement_AST_node : public statement_AST_node
{
    // verified in verify_symbol_table visitor (acclimate from children)
public:
    expression_AST_ptr expression;
    expression_statement_AST_node(expression_AST_ptr _expression);
    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "expStatement_ASTnode"; }
};

class definition_statement_AST_node : public Typed_VarDefinition
{
    // partially verified in define_names visitor
    // fully verified in build_types
public:

    definition_statement_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "defStatement_ASTnode"; }
};

class definitionNconstruction_statement_AST_node : public Typed_VarDefinition
{

public:
    argumentList_AST_ptr argument_list;

    definitionNconstruction_statement_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name,
                                               argumentList_AST_ptr _argument_list, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "defNConstruct_ASTnode"; }
};

class definitionNassignment_statement_AST_node : public Typed_VarDefinition
{

public:
    expression_AST_ptr expression;

    definitionNassignment_statement_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name,
                                             expression_AST_ptr _expression, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "defNAssign_ASTnode"; }
};

class assignment_statement_AST_node : public statement_AST_node
{
    // partially verified in reference_names
    // potentially verified in build_types

    // verified in verify_symbol_table visitor
public:
    LHSref_AST_ptr LHS;
    expression_AST_ptr expression; // once type of variable_symbol, type compatibility will be check in build_types

    assignment_statement_AST_node( LHSref_AST_ptr _LHS, expression_AST_ptr _expression, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "assignment_ASTnode"; }
};

class auto_definition_statement_AST_node : public General_VarDefinition
{
// partially verified in define_names
// partially verified in build_types
// fully verified in verify_symbol_table

// General_VarDefinition
//    csu::utf8_string var_name;
//    varName_ptr variable_symbol;

public:
    //csu::utf8_string var_name;
    //varName_ptr variable_symbol; // set in define_names visitor. Type set in build_types visitor
    expression_AST_ptr expression;

    auto_definition_statement_AST_node(csu::utf8_string& _var_name, expression_AST_ptr _expression, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "autoDef_ASTnode"; }
};

class return_statement_AST_node : public statement_AST_node
{
    // potentially verify in build_types.
    // fully verify in verify_symbol_table
public:
    expression_AST_ptr expression;
    callableDefinition_AST_node* callable_to_escape;

    return_statement_AST_node(expression_AST_ptr _expression, csu::location_span _loc);
    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "return_ASTnode"; }
};


class call_argument_list : public AST_node
{
// all potentially verified in build_types, completely verified in verify_symbol_table
public:

    class base_arguments_T : public AST_node
    {
        public:

        std::vector<expression_AST_ptr> arguments;
    };

    class unnamed_arguments_T : public base_arguments_T
    {
    public:

        unnamed_arguments_T( csu::location_span _loc );
        void add_argument( expression_AST_ptr new_argument, csu::location_span _loc );

        void apply_visitor_inner(AST_visitor_base* visitor) override;
        std::string AST_node_name() override { return "unnamedArgs_ASTnode"; }
    };
    typedef std::shared_ptr<unnamed_arguments_T> un_arguments_ptr;



    class named_arguments_T : public base_arguments_T
    {
    public:
        std::vector<csu::utf8_string> names;

        named_arguments_T( csu::location_span _loc );
        void add_argument( csu::utf8_string& name, expression_AST_ptr new_argument, csu::location_span _loc );

        void apply_visitor_inner(AST_visitor_base* visitor) override;
        std::string AST_node_name() override { return "namedArgs_ASTnode"; }
    };
    typedef std::shared_ptr<named_arguments_T> named_arguments_ptr;



    //NOTE: these are AST children, and they have AST children (can be null!)
    un_arguments_ptr unnamed_argument_list;
    named_arguments_ptr named_argument_list;

    call_argument_list(csu::location_span _loc, un_arguments_ptr _unnamed_list, named_arguments_ptr _named_list); // these can be null as necisary.

    int num_unnamed();
    int num_named();

    expression_AST_ptr unNamedExp_by_index(int i);
    expression_AST_ptr namedExp_by_index(int i);
    csu::utf8_string namedExpName_by_index(int i);

    int total_size();
    expression_AST_ptr expression_from_index(int i);

    function_argument_types_ptr get_argument_types();

    void print(std::ostream& output);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "callArgs_ASTnode"; }
};

class loopCntrl_statement_AST_node : public statement_AST_node
{
public:

    // some termonology:
        // top loop : loop that contains this cntrl statement
        // terminal loop : lowest level loop that is controlled by this statment. Is the one that could ostensibly be continued.
        // mid-loop : a loop controled by this statment that isn't the terminal loop
           // top loop is either a terminal loop or a mid-loop
        // penultimate loop : the lowest mid-loop. This may not exist if the top loop is the terminal loop
    enum cntrl_type
    {
        empty,
        break_t,
        cont_t,
    };

    int depth;
    cntrl_type type;

    // Set by a visitors
    loop_AST_node::loop_list_node* top_loop; // originally null.  owned by the loop
    csu::utf8_string var_control_name; // if depth !=0, this is c-variable (int) for control;

    loopCntrl_statement_AST_node(cntrl_type _type, int _depth, csu::location_span _loc);
    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "loopCntrl_ASTnode"; }
};



/// LHS references ///
// note, due to LALR(1) issues, the parsing of these is weird. See cyth_parser.cpp for notes
// because of this strange parsing, sometimes a dummy node, of type LHS_reference_AST_node, will be made (on an error)
// thus, LHS_reference_AST_node, cannot be pure virtual
class LHS_reference_AST_node : public AST_node
{
public:
    int level; //0 for top level, 1 otherwise. Set externally when node is made.
    // default is -1, so that (ostensibly) an error is thrown if this is not done
    // technically I guess these should be different nodes in some way. But I will try to avoid that (would actually be MORE complex).

    varType_ptr reference_type; // set in build_types. all LHS_references must be checked there
    //exp_writer_ptr writer; // technically is an expression, so we need this. Used by C-writer visiters

    LHS_reference_AST_node();
    std::string AST_node_name() override { return "LHS_reference_ASTnode"; }
};


class LHS_varReference : public LHS_reference_AST_node
{
public:
    csu::utf8_string var_name;
    varName_ptr variable_symbol; // set in reference_names visitor

    LHS_varReference(csu::utf8_string& _name, csu::location_span _loc );
    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "LHSvar_ASTnode"; }

};

class LHS_accessor_AST_node : public LHS_reference_AST_node
{
public:
    LHSref_AST_ptr LHS_exp;
    csu::utf8_string name;

    LHS_accessor_AST_node(LHSref_AST_ptr _LHS_exp, csu::utf8_string _name, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "LHSaccessor_ASTnode"; }
};











/// expressions ///

class expression_AST_node : public AST_node
{
public:

    varType_ptr expression_return_type; // set in build_types. all expresions must be checked there
//
//    bool has_output_ownership;
//    // set to true if cleanup has job to run destructor on this output.  default is true.
//    // Sometimes set to false on rare occasion by extrnal code, when an external code takes charge of the object
//
//    // these are set by the writer
//    //csu::utf8_string C_exp;
//    exp_writer_ptr writer;
//    bool c_exp_can_be_referenced;

    expression_AST_node();
};

class intLiteral_expression_AST_node : public expression_AST_node
{
    // fully verified in build_types::intLiteral_up
public:
    csu::utf8_string literal;

    intLiteral_expression_AST_node(csu::utf8_string _literal, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "int_ASTnode"; }
};

class binOperator_expression_AST_node : public expression_AST_node
{
    //partial verification in build_types
    // verified in verify_symbol_table visitor
public:
    enum expression_type
    {
        empty,
// math
        power_t,
        multiplication_t,
        division_t,
        modulus_t,
        addition_t,
        subtraction_t,

// comparison
        lessThan_t,
        greatThan_t,
        equalTo_t,
        notEqual_t,
        lessEqual_t,
        greatEqual_t,
    };

    enum expression_mode // set by vistor
    {
        LHS_m,  // using LHS.loperator( RHS )
        RHS_m   // using RHS.roperator( LHS )
    };

    expression_type type_of_operation;
    expression_mode  mode_of_operation; //set by build_types visitor

    expression_AST_ptr left_operand;
    expression_AST_ptr right_operand;

    binOperator_expression_AST_node(expression_AST_ptr _left_operand, expression_type _type, expression_AST_ptr _right_operand);

    const char* operator_name()
    {
        switch (type_of_operation)
        {
            case binOperator_expression_AST_node::empty:   return "empty";

            case binOperator_expression_AST_node::power_t:   return "power(^)";
            case binOperator_expression_AST_node::multiplication_t: return "multiplication(*)";
            case binOperator_expression_AST_node::division_t: return "division(/)";
            case binOperator_expression_AST_node::modulus_t: return "modulus(%)";
            case binOperator_expression_AST_node::addition_t: return "addition(+)";
            case binOperator_expression_AST_node::subtraction_t: return "subtraction(-)";

            case binOperator_expression_AST_node::lessThan_t: return "lessThan(<)";
            case binOperator_expression_AST_node::greatThan_t: return "greatThan(>)";
            case binOperator_expression_AST_node::equalTo_t: return "equalTo(==)";
            case binOperator_expression_AST_node::notEqual_t: return "notEqual(!=)";
            case binOperator_expression_AST_node::lessEqual_t: return "lessEqual(<=)";
            case binOperator_expression_AST_node::greatEqual_t: return "greatEqual(>=)";
        }
        return "Dr. Evil";
    }

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "binOp_ASTnode"; }

};

class binBoolOp_expression_AST_node : public expression_AST_node
{
    // boolean are seperate becouse I don't want a ton of if/else in the vistors

    //partial verification in build_types
    // verified in verify_symbol_table visitor
public:
    enum expression_type
    {
        empty,

        and_t,
        or_t,
        xor_t,
    };

    expression_type type_of_operation;

    expression_AST_ptr left_operand;
    expression_AST_ptr right_operand;

    binBoolOp_expression_AST_node(expression_AST_ptr _left_operand, expression_type _type, expression_AST_ptr _right_operand);

    const char* operator_name()
    {
        switch (type_of_operation)
        {
            case binBoolOp_expression_AST_node::empty:   return "empty";

            case binBoolOp_expression_AST_node::and_t:   return "AND";
            case binBoolOp_expression_AST_node::or_t: return "OR";
            case binBoolOp_expression_AST_node::xor_t: return "XOR";
        }
        return "Mr. Good";
    }

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "binBoolOp_ASTnode"; }

};




class varReferance_expression_AST_node : public expression_AST_node
{
    // partially verified in reference_names
    // completely verified varReferance_up
public:
    csu::utf8_string var_name;
    varName_ptr variable_symbol; // set in reference_names

    varReferance_expression_AST_node(csu::utf8_string _var_name, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "varRef_ASTnode"; }
};

class accessor_expression_AST_node : public expression_AST_node
{
    // potentially verify in build_types.
    // fully verify in verify_symbol_table
public:
    expression_AST_ptr expression;
    csu::utf8_string name;

    accessor_expression_AST_node(expression_AST_ptr _expression, csu::utf8_string _name, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "accessor_ASTnode"; }
};

class ParenGrouped_expression_AST_node : public expression_AST_node
{
    // potentially verify in build_types.
    // fully verify in verify_symbol_table
public:
    expression_AST_ptr expression;

    ParenGrouped_expression_AST_node(expression_AST_ptr _expression, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "parens_ASTnode"; }
};

class functionCall_expression_AST_node : public expression_AST_node
{
    // potentially verified in build_types
    // verified in verify_symbol_table visitor
public:
    expression_AST_ptr expression;
    argumentList_AST_ptr argument_list;


    functionCall_expression_AST_node( expression_AST_ptr _expression, argumentList_AST_ptr _argument_list, csu::location_span _loc);

    void apply_visitor_inner(AST_visitor_base* visitor) override;
    std::string AST_node_name() override { return "funcCall_ASTnode"; }
};


#endif
