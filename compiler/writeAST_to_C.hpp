
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

This file defines the functions to write the AST to C. Assuming the AST has been fully processed.
*/

#ifndef WRITEAST_TO_C_191101195347
#define WRITEAST_TO_C_191101195347

#include <string>
#include <sstream>


#include "UTF8.hpp"
#include "AST_visitor.hpp"
#include "cyth_AST.hpp"

//Next three visitors are the work horses. they write out the LHS_references, expressions and statements

// this is for writing the LHS of an assignment
class source_LHSreference_visitor : public AST_visitor_NoChildren
{
    // this is called solely by assignment at the moment.
    // it has levels, with top level is special. Top level acts like a setter, and thus needs the C-code and type info of the RHS of the assignment
    // the lower levels act like getters, and just build the appropriate LHS C-code.
    // This main class represents only the upper level. it has a child type that represents the lower levels.
    // Note that this upper level MAY call the lower level on itself or on children

private:

    class source_LHS_child : public AST_visitorTree
    {
    public:
        std::ofstream& source_fout;
        //csu::utf8_string LHS_C_code; // visitor assigns to this

        source_LHS_child(std::ofstream& _source_fout) :
            source_fout(_source_fout)
            {
            }

        std::shared_ptr< AST_visitor_base > make_child(int number) override;

        void LHS_varRef_up(LHS_varReference* varref) override;

        void LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor) override;
    };


    std::ofstream& source_fout;
    csu::utf8_string RHS_C_code; // c-code that defines the RHS
    varType_ptr RHS_type;
    expression_AST_node* RHS_AST_node;

public:

    source_LHSreference_visitor(std::ofstream& _source_fout, expression_AST_node* _RHS_AST_node, csu::utf8_string& RHS_exp, varType_ptr _RHS_type);

    // note this will NEVER call 'up' visitors!

    void LHS_varRef_down(LHS_varReference* varref) override;

    void LHS_accessor_down(LHS_accessor_AST_node* LHSaccess) override;
};

class LHSexpression_cleanup_visitor : public AST_visitorTree
{

// this is needed to call destructors on all temporary variables
// this MUST be called on all LHS expression writers!!

public:
    std::ostream& source_fout;

    LHSexpression_cleanup_visitor(std::ostream& _source_fout) :
        source_fout(_source_fout)
    {
    }

    std::shared_ptr< AST_visitor_base > make_child(int number) override
    { return std::make_shared<LHSexpression_cleanup_visitor>( source_fout ); }

    void LHSReference_down(LHS_reference_AST_node* LHS_ref) override;
};



class source_expression_visitor : public AST_visitorTree
{
/// write out straight-forward expressions. does not write functions, classes, or those complex thingies.
/// this should ONLY be called on expressions.

public:
    std::ostream& source_fout;
    //csu::utf8_string C_expression_code; // C expression code that returns the value of the cyth expression.

    //Note that each cyth expresion may require multiple C statements. Thus each cyth expression may write to the file, and to expression_code
    // because 'expression_code' contains code, it MUST be used by its parent


    source_expression_visitor(std::ostream& _source_fout) :
        source_fout(_source_fout)
    {
    }

    std::shared_ptr< AST_visitor_base > make_child(int number) override;

    //void callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs) override;

    //void baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children) override;

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp) override;

    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor) override;

    void varReferance_up(varReferance_expression_AST_node* varRefExp) override;

    void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor) override;

    void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor) override;

    void functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child) override;
};

class expression_cleanup_visitor : public AST_visitorTree
{

// this is needed to call destructors on all temporary variables
// this MUST be called on all expression writers!!

public:
    std::ostream& source_fout;

    expression_cleanup_visitor(std::ostream& _source_fout) :
        source_fout(_source_fout)
    {
    }

    std::shared_ptr< AST_visitor_base > make_child(int number) override
    { return std::make_shared<expression_cleanup_visitor>( source_fout ); }


    void expression_down(expression_AST_node* expression) override;




    //void callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs) override;

    //void baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children) override;

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp) override
    {}

    void varReferance_up(varReferance_expression_AST_node* varRefExp) override
    {}

    void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor) override
    {}

    void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor) override
    {}


// TODO: what to do here?
    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor) override;

    void functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child) override;
};



class source_statement_visitor : public AST_visitorTree
{
/// write out straight-forward statements. does not write functions, classes, or those complex thingies.
/// can write out single statements or block of statements

public:
    std::ofstream& source_fout;
    bool do_children;
    bool write_definitions;

    source_statement_visitor(std::ofstream& _source_fout, bool _write_definitions=true);

    bool apply_to_children() override
        { return do_children; }

    std::shared_ptr< AST_visitor_base > make_child(int number) override;


    //// things that cause this visitor to stop ////
    void funcDef_down(function_AST_node* funcDef) override;
    //// "normal" things ////

    void block_up(block_AST_node* block, std::list<AST_visitor_base*>& visitor_children) override;

    void statement_up(statement_AST_node* statment) override;

    void expressionStatement_down(expression_statement_AST_node* expStmt) override;

    void typed_VarDef_up(Typed_VarDefinition* var_def, AST_visitor_base* var_type) override;

    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child) override;

    void definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child) override;

    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child) override;

    void autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child) override;

    void returnStatement_down(return_statement_AST_node* returnStmt) override;

    void constructElement_up(constructElement_AST_node* constructBlock, AST_visitor_base* exp_child, AST_visitor_base* argList_child) override;
};


void write_module_to_C(module_AST_ptr module);

#endif // WRITEAST_TO_C_191101195347

