
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

If this doesn't work, than your only hope is to make a sacrifice to a higher demon,
  caus' I've no idea how this works. The simplicity of this header belies an evil complexity.
  Not that I blame myself, writing code that writes code is not easy!!
  (BTW, I find Bezelbub to be an reasonable demon to make deals with, but he only tells you his price AFTER the job is done!)
     (the only problem is that you don't know if giving him good advertisement was one of his previously required costs)
*/

#ifndef WRITEAST_TO_C_191101195347
#define WRITEAST_TO_C_191101195347

#include <string>
#include <sstream>


#include "UTF8.hpp"
#include "AST_visitor.hpp"
#include "cyth_AST.hpp"
#include "c_source_writer.hpp"

//Next three visitors are the work horses. they write out the LHS_references, expressions and statements

// this is for assigning a RHS to a LHS. This mostly works on operating on the LHS, the RHS should be given.
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
        Csource_out_ptr source_fout;
        C_expression_ptr final_expression;

        source_LHS_child(Csource_out_ptr _source_fout) :
            source_fout(_source_fout)
            {
            }

        std::shared_ptr< AST_visitor_base > make_child(int number) override;

        void LHS_varRef_up(LHS_varReference* varref) override;

        void LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor) override;
    };

    C_expression_ptr RHS_exp;

public:

    Csource_out_ptr source_fout;

    source_LHSreference_visitor(Csource_out_ptr _source_fout, C_expression_ptr _RHS_exp);

    // note this will NEVER call 'up' visitors!

    void LHS_varRef_down(LHS_varReference* varref) override;

    void LHS_accessor_down(LHS_accessor_AST_node* LHSaccess) override;
};



class source_expression_visitor : public AST_visitorTree
{
/// write out straight-forward expressions. does not write functions, classes, or those complex thingies.
/// this should ONLY be called on expressions.

private:
    void append_childExp_to_finalExp(); // this loops over all child expressions, and adds their final_expression to this final_expresson

public:

    Csource_out_ptr source_fout;
    C_expression_ptr final_expression;

    //Note that each cyth expresion may require multiple C statements. Thus each cyth expression may write to the file, and to expression_code
    // because 'expression_code' contains code, it MUST be used by its parent


    source_expression_visitor(Csource_out_ptr _source_fout) :
        source_fout(_source_fout)
    {
        //append_childernExp_to_thisExp = true; I don't know what this is.
    }

    std::shared_ptr< AST_visitor_base > make_child(int number) override;


    //void callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs) override;

    //void baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children) override;

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp) override;

    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor) override;

    void binBoolOperator_up(binBoolOp_expression_AST_node* binBoolOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor) override;

    void varReferance_up(varReferance_expression_AST_node* varRefExp) override;

    void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor) override;

    void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor) override;

    void functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child) override;
};





class source_statement_visitor : public AST_visitorTree
{
/// write out straight-forward statements. does not write functions, classes, or those complex thingies.
/// can write out single statements, block of statements, and flow control
/// This will NOT automatically write out expressions. Instead, source_expression_visitor will be instaiated and run inside these methods.

public:
    Csource_out_ptr source_fout;
    bool do_children;
    bool write_definitions;

    C_expression_ptr expression_to_cleanup; // this is cleanedup on ASTnode_up. Also can be used to communicate expressions between _down and _up vistors

    source_statement_visitor(Csource_out_ptr _source_fout, bool _write_definitions=true);

    bool apply_to_children() override
        { return do_children; }

    std::shared_ptr< AST_visitor_base > make_child(int number) override;


    /// things that cause this visitor to stop ////
    void funcDef_down(function_AST_node* funcDef) override;


    /// "normal" things ////
    // cleanup expression_to_cleanup if not empyt;
    void ASTnode_up(AST_node* ASTnode) override;

    // block
    void block_up(block_AST_node* block, std::list<AST_visitor_base*>& visitor_children) override; // cleans up variables in block


    // flow control
    void elifCond_down(elif_AST_node* elifNode) override;
    void activeCond_down(activeCond_AST_node* condNode) override;
    void else_down(else_AST_node* elifNode) override;

    void activeCond_up(activeCond_AST_node* condNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional) override;



    void while_down(whileLoop_AST_node* whileLoopNode) override;
    void while_up(whileLoop_AST_node* whileLoopNode, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child) override;

    void for_down(forLoop_AST_node* forLoopNode)override; // this is very complicated. So sets do_children=false, and does them itself in correct order.


    // single statements
    void statement_up(statement_AST_node* statment) override;

    void expressionStatement_down(expression_statement_AST_node* expStmt) override;


    //void typed_VarDef_up(Typed_VarDefinition* var_def, AST_visitor_base* var_type) override;

    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child) override;

    void definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt,
                            AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child) override;

    void definitionNassignment_up(definitionNassignment_statement_AST_node* defStmt,
                                    AST_visitor_base* varTypeRepr_child, AST_visitor_base* exp_child) override;




    void autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child) override;



    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child) override;

    void returnStatement_down(return_statement_AST_node* returnStmt) override;

    void constructElement_up(constructElement_AST_node* constructBlock, AST_visitor_base* exp_child, AST_visitor_base* argList_child) override;

    void loopCntrlStatement_up(loopCntrl_statement_AST_node* cntrlStmt) override;
};

class module_manager;

void write_module_to_C(module_AST_ptr module);
bool write_mainFunc_to_C(module_AST_ptr module, module_manager* mod_manager);

#endif // WRITEAST_TO_C_191101195347

