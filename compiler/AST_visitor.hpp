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

    virtual void funcDef_down(function_AST_node* funcDef){}

    virtual void varTypeRepr_down(varType_ASTrepr_node* varTypeRepr){}

    virtual void statement_down(statement_AST_node* statment){}
    virtual void expressionStatement_down(expression_statement_AST_node* expStmt){}
    virtual void definitionStmt_down(definition_statement_AST_node* defStmt){}
    virtual void assignmentStmt_down(assignment_statement_AST_node* assignStmt){}

    virtual void functionCall_Stmt_down(functionCall_statement_AST_node* funcCall){}

    virtual void expression_down(expression_AST_node* expression){}
    virtual void intLiteral_down(intLiteral_expression_AST_node* intLitExp){}
    virtual void binOperator_down(binOperator_expression_AST_node* binOprExp){}
    virtual void varReferance_down(varReferance_expression_AST_node* varRefExp){}

    // if apply_to_children returns False, then the visitor stops here, childern are not called, and UP (for this node) is not called. NOte that UP for the higher nodes is still called
    // and the visitor may conintue like normal on sibling nodes.




    // UP  THESE are called last, after up is called on all children, in order of decreasing generality
    virtual void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children){}
    virtual void ASTnode_up(AST_node* ASTnode){}

    virtual void allImports_up(import_AST_node* ASTnode){}
    virtual void cImports_up(import_C_AST_node* ASTnode){}

    virtual void funcDef_up(function_AST_node* funcDef, AST_visitor_base* stmt_child){}

    virtual void varTypeRepr_up(varType_ASTrepr_node* varTypeRepr){}

    virtual void statement_up(statement_AST_node* statment){}
    virtual void expressionStatement_up(expression_statement_AST_node* expStmt, AST_visitor_base* expression_child){}
    virtual void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child){}
    virtual void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child){}

    virtual void functionCall_Stmt_up(functionCall_statement_AST_node* funcCall,  AST_visitor_base* expression_child){}

    virtual void expression_up(expression_AST_node* expression){}
    virtual void intLiteral_up(intLiteral_expression_AST_node* intLitExp){}
    virtual void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor){}
    virtual void varReferance_up(varReferance_expression_AST_node* varRefExp){}

};



// this overloads the methods so that a tree is built
class AST_visitorTree : public AST_visitor_base
{
protected:
    AST_visitor_base* parent;
    std::vector< std::shared_ptr< AST_visitor_base > > children;
    bool children_initiated;

public:
    AST_visitorTree();
    AST_visitorTree(AST_visitor_base* _parent);

    //virtual ~AST_visitorTree(){}

    bool children_are_initiated(){ return children_initiated; }
    void initiate_children(int number);

    AST_visitor_base* get_child(int number){ return children[number].get(); }

    AST_visitor_base* get_parent(){ return parent; }

    template <typename return_type>
    return_type* get_parent(){ return dynamic_cast<return_type*>( parent ); }

    //overload these functions to change behavior of visitor

    // this is called for every child after down, before down is called on child
    virtual std::shared_ptr< AST_visitor_base > make_child(int number)=0;
};


// This overloads the opererators so that the visitor goes down then back up the tree without generating more memory
class AST_visitorTraveler : public AST_visitor_base
{

public:

    void initiate_children(int number){number_children=number;}

    AST_visitor_base* get_child(int number){ return this; }


    //// redefine the UP, becouse previous definition is unnecsary

    virtual void module_up(module_AST_node* module){}

    virtual void expressionStatement_up(expression_statement_AST_node* expStmt){}
    virtual void definitionStmt_up(definition_statement_AST_node* defStmt){}

    virtual void binOperator_up(binOperator_expression_AST_node* binOprExp){}


    // implement them
    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children)
    {
        module_up(module);
    }

    void expressionStatement_up(expression_statement_AST_node* expStmt, AST_visitor_base* expression_child)
    {
        expressionStatement_up(expStmt);
    }
    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child)
    {
        definitionStmt_up(defStmt);
    }

    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
    {
        binOperator_up(binOprExp);
    }
};




#endif // AST_VISITOR_190310141302
