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

This file defines the base class for abstract syntax tree visitors, which are essentially operators on the AST
*/


// IDEAS: make a visitor that propagates down a certain number of children (I.E. generalize AST_visitor_NoChildren)
// make it so visitor type on children can easily change (for visitorTree) (different children have different visitor types)
//     would then needway to easily cast back...
//

#ifndef AST_VISITOR_190310141302
#define AST_VISITOR_190310141302

#include <vector>
#include <memory>

#include "cyth_AST.hpp"

// The idea is that the visitor will form a tree that parelels the AST, for storing additional information. This is a visitorTree
// However there is a visitorTraveler that doesn't do this, it simply goes down and up the AST without forming more memory.


// Each node of the AST first calls the relavent DOWN callbacks in order of increaseing generality. IE: a binOperator node will call binOperator_down, then expression_down, then ASTnode_down
// then the node informs the node of number of childrnn through initiate_children, then get the visitor for each child throw get_child and sends that to the child
// after all children are done, the node calls the UP callbacks in order of decreasing generality: IE, ASTnode_up, expression_up, then binOperator_up
// one result of this is that ASTnode_down is always called before an AST node's children are called, and ASTnode_up is always called after

// This is the base, which is inherited by both visitorTree and visitorTraveler
class AST_visitor_base
{
public:

    AST_visitor_base(){}
    virtual ~AST_visitor_base(){}

    int number_children;

    virtual bool apply_to_children(){ return true; }  // if this returns true, then the visitor will not be called on children, and the UP callbacks will not be called.

    virtual void initiate_children(int number)=0;

    virtual AST_visitor_base* get_child(int number)=0;

    template <typename return_type>
    return_type*  get_child(int number){ return dynamic_cast<return_type*>( get_child(number) ); }

    //overload these functions to change behavior of visitor

    // DOWN  THESE are called first, in order of increasing generality
    virtual void module_down(module_AST_node* module){}
    virtual void ASTnode_down(AST_node* ASTnode){}

    virtual void allImports_down(import_AST_node* ASTnode){}
    virtual void cImports_down(import_C_AST_node* ASTnode){}
    virtual void CythImports_down(import_cyth_AST_node* ASTnode){}

    virtual void inheritanceList_down( inheritanceList_AST_node* inheritanceList_node){}
    virtual void ClassDef_down( class_AST_node* class_node){}

    virtual void block_down(block_AST_node* block){}
    virtual void callableDef_down(callableDefinition_AST_node* callDef){}
    virtual void funcDef_down(function_AST_node* funcDef){}
    virtual void methodDef_down(method_AST_node* methodDef){}

    virtual void funcParams_down(function_parameter_list* funcParams){}
    virtual void baseParams_down(function_parameter_list::base_parameters_T* baseParams){}
    virtual void reqParams_down(function_parameter_list::required_params* reqParams){}
    virtual void defaultParams_down(function_parameter_list::defaulted_params* defParams){}

    virtual void callArguments_down(call_argument_list* callArgs){}
    virtual void baseArguments_down(call_argument_list::base_arguments_T* argList){}
    virtual void unArguments_down(call_argument_list::unnamed_arguments_T* unArgs){}
    virtual void namedArguments_down(call_argument_list::named_arguments_T* namedArgs){}

    virtual void varTypeRepr_down(varType_ASTrepr_node* varTypeRepr){}

    virtual void statement_down(statement_AST_node* statment){}
    virtual void generic_VarDef_down(General_VarDefinition* vardef){}
    virtual void typed_VarDef_down(Typed_VarDefinition* var_def){}

    virtual void ClassVarDef_down( class_varDefinition_AST_node* class_var_def ){}
    virtual void definitionStmt_down(definition_statement_AST_node* defStmt){}
    virtual void definitionNconstructionStmt_down(definitionNconstruction_statement_AST_node* defStmt){}
    virtual void definitionNassignmentStmt_down(definitionNassignment_statement_AST_node* defStmt){}


    virtual void flowControl_down(flowControl_AST_node* controlNode){}
    virtual void conditional_down(conditional_AST_node* condNode){}

    virtual void activeCond_down(activeCond_AST_node* condNode){}
    virtual void ifCond_down(if_AST_node* ifNode){}
    virtual void elifCond_down(elif_AST_node* elifNode){}

    virtual void else_down(else_AST_node* elifNode){}


    virtual void loop_down(loop_AST_node* loopNode){}
    virtual void while_down(whileLoop_AST_node* whileLoopNode){}
    virtual void for_down(forLoop_AST_node* forLoopNode){}


    virtual void expressionStatement_down(expression_statement_AST_node* expStmt){}
    virtual void assignmentStmt_down(assignment_statement_AST_node* assignStmt){}
    virtual void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt){}
    virtual void returnStatement_down(return_statement_AST_node* returnStmt){}
    virtual void loopCntrlStatement_down(loopCntrl_statement_AST_node* cntrlStmt){}

    virtual void LHSReference_down(LHS_reference_AST_node* LHS_ref){}
    virtual void LHS_varRef_down(LHS_varReference* varref){}
    virtual void LHS_accessor_down(LHS_accessor_AST_node* LHSaccess){}

    virtual void expression_down(expression_AST_node* expression){}
    virtual void intLiteral_down(intLiteral_expression_AST_node* intLitExp){}
    virtual void binOperator_down(binOperator_expression_AST_node* binOprExp){}
    virtual void varReferance_down(varReferance_expression_AST_node* varRefExp){}
    virtual void ParenExpGrouping_down(ParenGrouped_expression_AST_node* parenGroupExp){}
    virtual void accessorExp_down(accessor_expression_AST_node* accessorExp){}
    virtual void functionCall_Exp_down(functionCall_expression_AST_node* funcCall){}

    virtual void constructBlock_down(construct_AST_node* constructBlock){}
    virtual void constructElement_down(constructElement_AST_node* constructBlock){}




    // if apply_to_children returns False, then the visitor stops here, childern are not called, and UP (for this node) is not called. NOte that UP for the higher nodes is still called
    // and the visitor may continue like normal on sibling nodes.


    // UP  THESE are called last, after up is called on all children, in order of decreasing generality
    virtual void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children){}
    virtual void ASTnode_up(AST_node* ASTnode){}

    virtual void allImports_up(import_AST_node* ASTnode){}
    virtual void cImports_up(import_C_AST_node* ASTnode){}
    virtual void CythImports_up(import_cyth_AST_node* ASTnode){}

    virtual void inheritanceList_up( inheritanceList_AST_node* inheritanceList_node){}
    virtual void ClassDef_up( class_AST_node* clss, std::list<AST_visitor_base*>& var_def_children,
                             std::list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child ) {}

    virtual void block_up(block_AST_node* block, std::list<AST_visitor_base*>& visitor_children){}
    virtual void callableDef_up(callableDefinition_AST_node* callDef, AST_visitor_base* paramList_child){}
    virtual void funcDef_up(function_AST_node* funcDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                            AST_visitor_base* funcBody_child){}
    virtual void methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child){}


    virtual void funcParams_up(function_parameter_list* funcParams, AST_visitor_base* req_child, AST_visitor_base* default_child){}
    virtual void baseParams_up(function_parameter_list::base_parameters_T* baseParams, std::list<AST_visitor_base*>& visitor_children){}
    virtual void reqParams_up(function_parameter_list::required_params* reqParams, std::list<AST_visitor_base*>& visitor_children){}
    virtual void defaultParams_up(function_parameter_list::defaulted_params* defParams,
                                  std::list<AST_visitor_base*>& param_name_visitors, std::list<AST_visitor_base*>& default_exp_visitors){}

    virtual void callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs){}
    virtual void baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children){}
    virtual void unArguments_up(call_argument_list::unnamed_arguments_T* unArgs, std::list<AST_visitor_base*>& visitor_children){}
    virtual void namedArguments_up(call_argument_list::named_arguments_T* namedArgs, std::list<AST_visitor_base*>& visitor_children){}

    virtual void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr){}

    virtual void statement_up(statement_AST_node* statment){}
    virtual void generic_VarDef_up(General_VarDefinition* var_def){}
    virtual void typed_VarDef_up(Typed_VarDefinition* var_def, AST_visitor_base* varTypeRepr_child){}

    virtual void ClassVarDef_up( class_varDefinition_AST_node* class_var_def, AST_visitor_base* varType, AST_visitor_base* default_exp){}
    virtual void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child){}
    virtual void definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child){}
    virtual void definitionNassignment_up(definitionNassignment_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* exp_child){}


    virtual void flowControl_up(flowControl_AST_node* controlNode, AST_visitor_base* Block_child){}
    virtual void conditional_up(conditional_AST_node* condNode, AST_visitor_base* Block_child){}

    virtual void activeCond_up(activeCond_AST_node* condNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional){}
    virtual void ifCond_up(if_AST_node* ifNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional){}
    virtual void elifCond_up(elif_AST_node* elifNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional){}

    virtual void else_up(else_AST_node* elseNode, AST_visitor_base* block_child){}


    virtual void loop_up(loop_AST_node* loopNode, AST_visitor_base* Block_child){}
    virtual void while_up(whileLoop_AST_node* whileLoopNode, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child){}
    virtual void for_up(forLoop_AST_node* forLoopNode, AST_visitor_base* initialStmt_child, AST_visitor_base* updateStmt_child, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child){}



    virtual void expressionStatement_up(expression_statement_AST_node* expStmt, AST_visitor_base* expression_child){}
    virtual void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child){}
    virtual void autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child){}
    virtual void returnStatement_up(return_statement_AST_node* returnStmt, AST_visitor_base* expression_child){}
    virtual void loopCntrlStatement_up(loopCntrl_statement_AST_node* cntrlStmt){}

    virtual void LHSReference_up(LHS_reference_AST_node* LHS_ref){}
    virtual void LHS_varRef_up(LHS_varReference* varref){}
    virtual void LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor){}

    virtual void expression_up(expression_AST_node* expression){}
    virtual void intLiteral_up(intLiteral_expression_AST_node* intLitExp){}
    virtual void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor){}
    virtual void varReferance_up(varReferance_expression_AST_node* varRefExp){}
    virtual void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor){}
    virtual void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor){}
    virtual void functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child){}

    virtual void constructBlock_up(construct_AST_node* constructBlock, std::list<AST_visitor_base*>& visitor_children){}
    virtual void constructElement_up(constructElement_AST_node* constructBlock, AST_visitor_base* exp_child, AST_visitor_base* argList_child){}

};



// this overloads the methods so that a tree is built
// probably the most 'normal' visitor!
//
class AST_visitorTree : public AST_visitor_base
{
protected:
    AST_visitor_base* parent; // this should be of type AST_visitorTree at least I think?
    std::vector< std::shared_ptr< AST_visitor_base > > children;
    bool children_initiated;

public:
    AST_visitorTree();
    AST_visitorTree(AST_visitor_base* _parent);

    //virtual ~AST_visitorTree(){}

    bool children_are_initiated() { return children_initiated; }
    void initiate_children(int number) override;

    AST_visitor_base* get_child(int number) override
        { return children[number].get(); }

    AST_visitor_base* get_parent(){ return parent; }

    template <typename return_type>
    return_type* get_parent(){ return dynamic_cast<return_type*>( parent ); }

    //overload these functions to change behavior of visitor

    // this is called for every child after down, before down is called on child
    virtual std::shared_ptr< AST_visitor_base > make_child(int number)=0;
};


/// Revisitors, crawl down or up a previous visitor. Total basterdization of visitor
// crawls down and up previous visitor. Does not accses parents.
template<class visitor_type>
class revisitor_tree : public AST_visitorTree
{
public :
    visitor_type* twin;

    virtual std::shared_ptr< revisitor_tree<visitor_type> > make_revistor_child(int number)=0;

    std::shared_ptr< AST_visitor_base > make_child(int number) final
    {
        auto new_child = make_revistor_child( number );
        new_child->twin = dynamic_cast<visitor_type*>( twin->get_child( number ) );
        return new_child;
    }
};


template< class visitor_type >
void apply_revisitor(AST_node* applied_node, visitor_type* visitor_to_visit, revisitor_tree<visitor_type>* revisitor  )
{
    revisitor->twin = visitor_to_visit;
    applied_node->apply_visitor( revisitor );
}







// This overloads the opererators so that the visitor goes down then back up the tree without generating more memory
class AST_visitorTraveler : public AST_visitor_base
{

public:

    void initiate_children(int number) override
            {number_children=number;}

    AST_visitor_base* get_child(int number) override
            { return this; }


    //// redefine the UP, becouse previous definition is unnecsary
    // technically I'd like to do this, but it is not a pain and not necisary.

//    virtual void module_up(module_AST_node* module){}
//
//    virtual void expressionStatement_up(expression_statement_AST_node* expStmt){}
//    virtual void definitionStmt_up(definition_statement_AST_node* defStmt){}
//
//    virtual void binOperator_up(binOperator_expression_AST_node* binOprExp){}
//
//
//    // implement them
//    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children)
//    {
//        module_up(module);
//    }
//
//    void expressionStatement_up(expression_statement_AST_node* expStmt, AST_visitor_base* expression_child)
//    {
//        expressionStatement_up(expStmt);
//    }
//    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child)
//    {
//        definitionStmt_up(defStmt);
//    }
//
//    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
//    {
//        binOperator_up(binOprExp);
//    }
};

// if a visitor only visits the thing it is called on, which no chillins
class AST_visitor_NoChildren : public AST_visitor_base
{
public:

    bool apply_to_children() final { return false; }

    void initiate_children(int number) final { throw gen_exception("AST_visitor_NoChildren should not have children. This should never be reached."); }

    AST_visitor_base* get_child(int number) final { throw gen_exception("AST_visitor_NoChildren should not have children. This should never be reached."); }


    // ups will never be called
    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children) final {}
    void ASTnode_up(AST_node* ASTnode) final {}

    void allImports_up(import_AST_node* ASTnode) final {}
    void cImports_up(import_C_AST_node* ASTnode) final {}
    void CythImports_up(import_cyth_AST_node* ASTnode) final {}

    void inheritanceList_up( inheritanceList_AST_node* inheritanceList_node) final {}
    void ClassDef_up( class_AST_node* block, std::list<AST_visitor_base*>& var_def_children, std::list<AST_visitor_base*>& method_def_children
                     , AST_visitor_base* inheritanceList_child  ) final {}

    void block_up(block_AST_node* block, std::list<AST_visitor_base*>& visitor_children) final {}
    void callableDef_up(callableDefinition_AST_node* callDef, AST_visitor_base* paramList_child) final {}
    void funcDef_up(function_AST_node* funcDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child, AST_visitor_base* funcBody_child) final {}
    void methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child, AST_visitor_base* methodBody_child) final {}


    void funcParams_up(function_parameter_list* funcParams, AST_visitor_base* req_child, AST_visitor_base* default_child) final {}
    void baseParams_up(function_parameter_list::base_parameters_T* baseParams, std::list<AST_visitor_base*>& visitor_children) final {}
    void reqParams_up(function_parameter_list::required_params* reqParams, std::list<AST_visitor_base*>& visitor_children) final {}
    void defaultParams_up(function_parameter_list::defaulted_params* defParams,
                                  std::list<AST_visitor_base*>& param_name_visitors, std::list<AST_visitor_base*>& default_exp_visitors) final {}

    void callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs) final {}
    void baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children) final {}
    void unArguments_up(call_argument_list::unnamed_arguments_T* unArgs, std::list<AST_visitor_base*>& visitor_children) final {}
    void namedArguments_up(call_argument_list::named_arguments_T* namedArgs, std::list<AST_visitor_base*>& visitor_children) final {}

    void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr) final {}

    void statement_up(statement_AST_node* statment) final {}
    void generic_VarDef_up(General_VarDefinition* var_def) final {}
    void typed_VarDef_up(Typed_VarDefinition* var_def, AST_visitor_base* varTypeRepr_child) final {}

    void ClassVarDef_up( class_varDefinition_AST_node* class_var_def, AST_visitor_base* varType, AST_visitor_base* default_exp) final {}
    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child) final {}

    void definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child) final {}
    void definitionNassignment_up(definitionNassignment_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* exp_child) final {}


    void flowControl_up(flowControl_AST_node* controlNode, AST_visitor_base* elseBlock_child) final {}
    void conditional_up(conditional_AST_node* condNode, AST_visitor_base* elseBlock_child) final {}

    void activeCond_up(activeCond_AST_node* condNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional) final {}
    void ifCond_up(if_AST_node* ifNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional) final {}
    void elifCond_up(elif_AST_node* elifNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional) final {}

    void else_up(else_AST_node* elseNode, AST_visitor_base* block_child) final {}

    void loop_up(loop_AST_node* loopNode, AST_visitor_base* Block_child) final {}
    void while_up(whileLoop_AST_node* whileLoopNode, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child) final {}
    void for_up(forLoop_AST_node* forLoopNode, AST_visitor_base* initialStmt_child, AST_visitor_base* updateStmt_child, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child) final {}


    void expressionStatement_up(expression_statement_AST_node* expStmt, AST_visitor_base* expression_child) final {}
    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child) final {}
    void autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child) final {}
    void returnStatement_up(return_statement_AST_node* returnStmt, AST_visitor_base* expression_child) final {}
    void loopCntrlStatement_up(loopCntrl_statement_AST_node* cntrlStmt)final {}

    void LHSReference_up(LHS_reference_AST_node* LHS_ref) final {}
    void LHS_varRef_up(LHS_varReference* varref) final {}
    void LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor) final {}

    void expression_up(expression_AST_node* expression) final {}
    void intLiteral_up(intLiteral_expression_AST_node* intLitExp) final {}
    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor) final {}
    void varReferance_up(varReferance_expression_AST_node* varRefExp) final {}
    void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor) final {}
    void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor) final {}
    void functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child) final {}

    void constructBlock_up(construct_AST_node* constructBlock, std::list<AST_visitor_base*>& visitor_children) final {}
    void constructElement_up(constructElement_AST_node* constructBlock, AST_visitor_base* exp_child, AST_visitor_base* argList_child) final {}

};


void apply_visitor_upwards(AST_node* node, AST_visitor_base* visitor);
// apply visitor to this node, and then each parent upwards.
// best when used with AST_visitor_NoChildren
// must be applied AFTER set_symbol_table visitor. (otherwise parents is not set).


#endif // AST_VISITOR_190310141302
