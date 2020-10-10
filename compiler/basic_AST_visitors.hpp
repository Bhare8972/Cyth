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

    set_symbol_table(module_AST_node* module);
    set_symbol_table(AST_visitor_base* _parent);

    std::shared_ptr< AST_visitor_base > make_child(int number) override;

    void set_new_table(AST_node* ASTnode, csu::utf8_string& table_name);

    // if symbol_table is null, sets to parent if has parent
    void ASTnode_down(AST_node* ASTnode) override;

    void funcDef_down(function_AST_node* funcDef) override;
    void ClassDef_down( class_AST_node* class_node) override;
    void methodDef_down(method_AST_node* methodDef) override;
};

// defines all the names.
// may throw error if names are defined twice
class module_manager;
class define_names : public AST_visitorTree
{
public:
    module_manager *cyth_module_manager;

    bool is_module;
    bool at_module_level;

    define_names(module_manager *_cyth_module_manager);
    define_names(module_manager *_cyth_module_manager, bool _at_module_level);

    std::shared_ptr< AST_visitor_base > make_child(int number) override;


    void cImports_up(import_C_AST_node* ASTnode) override; // completely verified here
    void CythImports_up(import_cyth_AST_node* ASTnode) override;

    // difficult to combine these three, since ClassVarDef must be on down
    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child) override;
    void definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child) override;
    void autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child) override;

    void funcDef_down(function_AST_node* funcDef) override; // need to do this BEFORE block, to define arguments
    void baseParams_up(function_parameter_list::base_parameters_T* baseParams, std::list<AST_visitor_base*>& visitor_children) override;

    void ClassDef_down( class_AST_node* class_node) override;
    void ClassVarDef_down( class_varDefinition_AST_node* class_var_def ) override; // different from definitionStmt_up, since is not ordered

    void methodDef_down(method_AST_node* methodDef) override; // need to do this BEFORE block, to define arguments
};

// find where names are explicitly referenced
// throw if name is used before defined, or not defined at all
// is this really needed? or do this in build_types? what about member accssesing?
class reference_names : public AST_visitorTree
{
public:

    std::shared_ptr< AST_visitor_base > make_child(int number) override;

    void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr) override; // completely verified here

//void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child) override;
    void LHS_varRef_up(LHS_varReference* varref) override;

    void varReferance_up(varReferance_expression_AST_node* varRefExp) override;

    void inheritanceList_up( inheritanceList_AST_node* inheritanceList_node) override;

};


// we need to register all the overloads of different functions and methods
class register_overloads : public AST_visitorTree
{
public:
    bool is_module;
    bool at_module_level;
    callableDefinition_AST_node* current_callable;

    register_overloads(); // called by module
    register_overloads(bool _at_module_level, callableDefinition_AST_node* _current_callable); // else

    std::shared_ptr< AST_visitor_base > make_child(int number) override;

    void callableDef_down(callableDefinition_AST_node* callDef) override; // sets current_callable
    void returnStatement_down(return_statement_AST_node* returnStmt) override; // uses above info

    // need to register the type of the parameters BEFORE we register the function overload...
    void baseParams_down(function_parameter_list::base_parameters_T* baseParams) override;

    void funcDef_up(function_AST_node* funcDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                    AST_visitor_base* funcBody_child) override;

    void methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child) override;

// wonderfully enough, this does a whole lot more than just register overloads.
//  checks validity of some special methods, and makes defaulted methods (__init__, __exInit__, __del__,
//          __convert__, __exConvert__, and __assign__ (todo) )
// also sets top-level inheritances list
    void ClassDef_up( class_AST_node* class_node, std::list<AST_visitor_base*>& var_def_children,
                     std::list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child ) override;
};


// need to do some stuff with classes, such as sorting  inheritance
class build_classes : public AST_visitorTree
{
    public:
    std::shared_ptr< AST_visitor_base > make_child(int number) override;

    void ClassDef_up( class_AST_node* class_node, std::list<AST_visitor_base*>& var_def_children,
         std::list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child ) override;
};




// now we figure out what the type of everything is
// throw error if types are incompatible in some way
class build_types : public AST_visitorTree
{
public:
    bool changes_were_made;
    bool debug; // default false

    build_types();
    build_types(bool _debug);

    std::shared_ptr< AST_visitor_base > make_child(int number) override;

    void ASTnode_up(AST_node* ASTnode) override; // accumulates if children have changes.

    void callableDef_up(callableDefinition_AST_node* callDef, AST_visitor_base* paramList_child) override;

    void baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children) override;
    void callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs) override;

    void reqParams_up(function_parameter_list::required_params* reqParams, std::list<AST_visitor_base*>& visitor_children) override;
    void defaultParams_up(function_parameter_list::defaulted_params* defParams,
                                  std::list<AST_visitor_base*>& param_name_visitors, std::list<AST_visitor_base*>& default_exp_visitors) override;

    void typed_VarDef_up(Typed_VarDefinition* var_def, AST_visitor_base* var_type) override;
    void definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child) override;
    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child) override;
    void autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child) override;
    void returnStatement_up(return_statement_AST_node* returnStmt, AST_visitor_base* expression_child) override;

    void LHS_varRef_up(LHS_varReference* varref) override;
    void LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor) override;

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp) override; // fully verfied
    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor) override;
    void varReferance_up(varReferance_expression_AST_node* varRefExp) override; // potentially verified
    void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor) override;
    void functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child) override;
    void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor) override;

    void constructElement_up(constructElement_AST_node* constructBlock,AST_visitor_base* exp_child,  AST_visitor_base* argList_child) override;
};



// verify symbol table
// by the time we get here the AST nodes are in four classes
// 1) nodes that have already been verified
// 2) nodes that need to be verified
        // 2a need to override before if something special is needed
        // 2b need to call fail_if_not_verified if fail on -1 state.
            // if called in DOWN, then will not acclimate from children.
// 3) nodes that are verified via acclimation of children
        // this is default behavior. Can turn off by overrdign DOWN callback and set do_acclimation to false.
        // will throw exception if child is not verified.
class verify_symbol_table : public AST_visitorTree
{
public:
    bool is_verified; // true if this node doesn't need to check children for verification.
    bool do_acclimation; // default is true. Set to False on way down if not.
    int verification_state; // set in ASTnode_down or ASTnode_up accordingly

    verify_symbol_table();
    std::shared_ptr< AST_visitor_base > make_child(int number) override;

    void ASTnode_down(AST_node* ASTnode) override
    {
        if( ASTnode->verification_state != -1 )
        {
            is_verified = true;
            verification_state = ASTnode->verification_state;
        }
    }
    // Note this is called immediatly after ASTnode_down
    bool apply_to_children() override
        { return not is_verified; } // only do children if not verified

    // acclimate children unless (do_acclimation has been set to false) or is_verified.
    void ASTnode_up(AST_node* ASTnode) override;


    void fail_if_not_verified( AST_node* ASTnode, std::string name )
    {
        if( ASTnode->verification_state == -1 )
        {
            std::cout << "could not verify type info in " << name <<" " << ASTnode->loc << std::endl;
            std::cout << "   perhaps more type-iterations are needed" << std::endl;
            ASTnode->verification_state = 0;
        }
    }

    void baseParams_down(function_parameter_list::base_parameters_T* baseParams) override
        { fail_if_not_verified(baseParams, "function parameters"); }

    void baseArguments_down(call_argument_list::base_arguments_T* argList) override
        { fail_if_not_verified(argList, "call argument list"); }

    void callArguments_down(call_argument_list* callArgs) override
        { fail_if_not_verified(callArgs, "call arguments"); }



    void returnStatement_down(return_statement_AST_node* returnStmt) override
        { fail_if_not_verified(returnStmt, "return"); }

    void assignmentStmt_down(assignment_statement_AST_node* assignStmt) override
        { fail_if_not_verified(assignStmt, "assignment"); }

    void binOperator_down(binOperator_expression_AST_node* binOprExp) override
        { fail_if_not_verified(binOprExp, "binary expression"); }


    void LHS_varRef_up(LHS_varReference* varref) override
        { fail_if_not_verified(varref, "LHS variable reference"); }

    void LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor) override
        { fail_if_not_verified(LHSaccess, "LHS member access"); }


    void varReferance_up(varReferance_expression_AST_node* varRefExp) override
        { fail_if_not_verified(varRefExp, "variable reference expression"); }

    void generic_VarDef_down(General_VarDefinition* vardef) override
        { fail_if_not_verified(vardef, "variable definition"); }

    void ParenExpGrouping_down(ParenGrouped_expression_AST_node* parenGroupExp) override
        { fail_if_not_verified(parenGroupExp, "parenthetical group"); }

    void functionCall_Exp_down(functionCall_expression_AST_node* funcCall) override
        { fail_if_not_verified(funcCall, "function call"); }

    void accessorExp_down(accessor_expression_AST_node* accessorExp) override
        { fail_if_not_verified(accessorExp, "member access"); }

    void methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child) override; // need to check return types same as overrided parent method

    void ClassDef_up( class_AST_node* clss_node, std::list<AST_visitor_base*>& var_def_children,
                     std::list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child ) override; // check that return types in the method_connection_table are consistent

};

#endif // BASIC_AST_VISITORS_191110152455

