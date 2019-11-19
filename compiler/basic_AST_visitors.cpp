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

#include "basic_AST_visitors.hpp"
#include "gen_ex.h"

using namespace csu;
using namespace std;


//// set symbol table ////

set_symbol_table::set_symbol_table(module_AST_node* module, utf8_string module_name)
{
    module->module_name = module_name;
    module->top_symbol_table.set_name( module_name );
    symbol_table = module->symbol_table;
}

set_symbol_table::set_symbol_table(AST_visitor_base* _parent) :
    AST_visitorTree( _parent )
{
    symbol_table = nullptr;
}

shared_ptr< AST_visitor_base > set_symbol_table::make_child(int number)
{
    return make_shared<set_symbol_table>( this );
}


void set_symbol_table::ASTnode_down(AST_node* ASTnode)
{
    if( parent and not symbol_table)
    {
        set_symbol_table* par = dynamic_cast<set_symbol_table*>( parent );
        ASTnode->symbol_table = par->symbol_table;
        symbol_table = ASTnode->symbol_table;
    }
}

//void set_symbol_table::set_new_table(AST_node* ASTnode, utf8_string& table_name)
//{
//    set_symbol_table* par = dynamic_cast<set_symbol_table*>( parent );
//    sym_table_ptr new_table = make_shared<local_sym_table>( table_name, par->symbol_table.get() );
//    ASTnode->symbol_table = new_table;
//    symbol_table = ASTnode->symbol_table;
//}



//// build_symbol_table ////

build_symbol_table::build_symbol_table()
{
    changes_were_made = false;
    at_module_level = false;
    is_module = true;
}

build_symbol_table::build_symbol_table(bool _at_module_level)
{
    is_module = false;
    changes_were_made = false;
    at_module_level = _at_module_level;
}

shared_ptr< AST_visitor_base > build_symbol_table::make_child(int number)
{
    return make_shared<build_symbol_table>( is_module );
}

void build_symbol_table::ASTnode_up(AST_node* ASTnode)
{
    for(auto it : children)
    {
        shared_ptr< build_symbol_table > chil = dynamic_pointer_cast<build_symbol_table>( it );
        if( chil->changes_were_made )
        {
            changes_were_made = true;
            break;
        }
    }
}

void build_symbol_table::cImports_up(import_C_AST_node* ASTnode)
{

    if( not ASTnode->variable_type)
    {
        utf8_string TMP = "UNNAMED_C_TYPE";
        ASTnode->variable_type = ASTnode->symbol_table->get_type_global(TMP, ASTnode->loc);

        changes_were_made = true;
    }

    if( not ASTnode->variable)
    {
        ASTnode->variable = ASTnode->symbol_table->new_variable( ASTnode->variable_type, ASTnode->usage_name, ASTnode->loc, false);
        ASTnode->variable->C_name = ASTnode->import_name;

        changes_were_made = true;
    }

    if( not ASTnode->type)
    {
        shared_ptr<varType_fromC> T = make_shared<varType_fromC>();
        T->unnamed_C_type = ASTnode->variable_type;
        ASTnode->type = T;
        ASTnode->type->definition_name = ASTnode->usage_name;
        ASTnode->type->C_name = ASTnode->import_name;
        ASTnode->type->loc = ASTnode->loc;
        ASTnode->type->is_ordered = false;

        ASTnode->symbol_table->add_type( ASTnode->type );

        changes_were_made = true;
    }

}

void build_symbol_table::funcDef_down(function_AST_node* funcDef)
{
    bool do_ordered = at_module_level;

    if( not funcDef->funcName )
    {
        //// FIRST WE GET THE resolved_name and resolved_type ////

        // first we check if function has been defined elsewhere
        varName_ptr resolved_name = funcDef->symbol_table->get_variable_global( funcDef->name, funcDef->loc );
        DefFuncType_ptr resolved_type = nullptr;

        if( resolved_name )
        // check name is a function
        {
            if( not ( resolved_name->var_type->definition_name == "DEFINED_FUNCTION" ) )
            {
                throw gen_exception("name ", funcDef->name, " already defined ", resolved_name->loc, " with type ", resolved_name->var_type->definition_name,
                    ". Define function error ", funcDef->loc);
            }
            else
            {
                resolved_type = dynamic_pointer_cast<DefFuncType>( resolved_name->var_type );
            }

            if( not do_ordered and resolved_name->is_ordered )
            {
                resolved_name->is_ordered  = false; // ... hard to explain why...
            }
        }
        else
        // entirely new stuff!
        {
            resolved_type = make_shared<DefFuncType>( funcDef->loc );
            resolved_name = funcDef->symbol_table->new_variable( resolved_type, funcDef->name, funcDef->loc, do_ordered );
        }

        funcDef->funcName = resolved_name;
        funcDef->funcType = resolved_type;

        changes_were_made = true;
    }

    if( funcDef->funcName and not funcDef->specific_overload )
    {
        //// NOW WE ATTEMPT TO DEFINE OVERLOAD ////
        ResolvedFunction_ptr specific_overload = funcDef->funcType->define_overload( funcDef->loc, funcDef->symbol_table );

        specific_overload->C_name = funcDef->symbol_table->namespace_unique_name + "__";
        specific_overload->C_name += funcDef->name;
        specific_overload->C_name += utf8_string("__");
        specific_overload->C_name += to_string( funcDef->funcType->num_overloads() );

        if( not specific_overload )
        {
            throw gen_exception("new function overload: ", funcDef->name, ", " , funcDef->funcName->loc, ". Cannot be defined. Good luck!");
        }

        if( not do_ordered)
        {
            specific_overload->set_unordered();
        }

        funcDef->specific_overload = specific_overload;
        changes_were_made = true;
    }
}

void build_symbol_table::varTypeRepr_up(varType_ASTrepr_node* varTypeRepr)
{
    if( not varTypeRepr->resolved_type)
    {
        varTypeRepr->resolved_type = varTypeRepr->symbol_table->get_type_global(varTypeRepr->name, varTypeRepr->loc);

        if(varTypeRepr->resolved_type)
        {
            changes_were_made = true;
        }
    }
}

void build_symbol_table::definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child)
{

    if( (not defStmt->variable_symbol) and defStmt->var_type->resolved_type) //type is resolved, but variable not defined
    {
        defStmt->variable_symbol = defStmt->symbol_table->new_variable(defStmt->var_type->resolved_type, defStmt->var_name, defStmt->loc);
        changes_were_made = true;
    }
}

void build_symbol_table::assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child)
{
    if( not assignStmt->variable_symbol )
    {
        assignStmt->variable_symbol = assignStmt->symbol_table->get_variable_global( assignStmt->var_name, assignStmt->loc );
        changes_were_made = true;
    }
}

void build_symbol_table::intLiteral_up(intLiteral_expression_AST_node* intLitExp)
{
    if( not intLitExp->expression_return_type)
    {
        utf8_string TMP = "UNNAMED_C_TYPE";
        intLitExp->expression_return_type = intLitExp->symbol_table->get_type_global(TMP, intLitExp->loc);
        changes_were_made = true;
    }
}

void build_symbol_table::binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
{
    // NOTE: we are assuming LHS + RHS at this point
    verify_symbol_table verify_vstr( false );
    binOprExp->left_operand->apply_visitor(&verify_vstr);
    binOprExp->right_operand->apply_visitor(&verify_vstr);

    if( binOprExp->left_operand->symbol_table_verified and binOprExp->right_operand->symbol_table_verified and not binOprExp->expression_return_type)
    {

        varType_ptr LHS_type = binOprExp->left_operand->expression_return_type;
        varType_ptr RHS_type = binOprExp->right_operand->expression_return_type;

        if( LHS_type->get_has_LHSaddition( RHS_type.get() ) )
        {
            binOprExp->expression_return_type = LHS_type->get_LHSaddition_type( RHS_type.get() );
            changes_were_made = true;
            utf8_string T = "UNNAMED_C_TYPE";
            if( binOprExp->expression_return_type == binOprExp->symbol_table->get_type_global(T, binOprExp->loc) )
            {
                binOprExp->return_type_is_unnamed = true;
            }
            binOprExp->operation_function_name = LHS_type->get_LHSaddition_name( RHS_type.get() );
        }
    }
}

void build_symbol_table::varReferance_up(varReferance_expression_AST_node* varRefExp)
{
    if( not varRefExp->variable_symbol )
    {
        varRefExp->variable_symbol = varRefExp->symbol_table->get_variable_global( varRefExp->var_name, varRefExp->loc );
        varRefExp->expression_return_type = varRefExp->variable_symbol->var_type;

        utf8_string T = "UNNAMED_C_TYPE";
        if( varRefExp->expression_return_type == varRefExp->symbol_table->get_type_global(T, varRefExp->loc) )
        {
            varRefExp->return_type_is_unnamed = true;
        }

        changes_were_made = true;
    }
}




//// verify symbol table ////

void verify_symbol_table::module_up(module_AST_node* module)
{
    module->symbol_table_verified = true;
    for(auto node : module->module_contents)
    {
        if( not node->symbol_table_verified )
        {
            module->symbol_table_verified = false;
            break;
        }
    }
}

void verify_symbol_table::cImports_up(import_C_AST_node* ASTnode)
{
    if( (not ASTnode->type) or (not ASTnode->variable) or (not ASTnode->variable_type) )
    {
        if( do_error_reporting )
        {
            cout<< "ERROR: could not define import at " << ASTnode->loc << endl;
        }
        ASTnode->symbol_table_verified = false;
    }
    else
    {
        ASTnode->symbol_table_verified = true;
    }
}

void verify_symbol_table::varTypeRepr_up(varType_ASTrepr_node* varTypeRepr)
{
    if( varTypeRepr->resolved_type )
    {
        varTypeRepr->symbol_table_verified = true;
    }
    else
    {
        if( do_error_reporting )
        {
            cout<< "ERROR: could not resolve type '" << varTypeRepr->name <<"' " << varTypeRepr->loc << endl;
        }
        varTypeRepr->symbol_table_verified = false;
    }
}

void verify_symbol_table::funcDef_up(function_AST_node* funcDef, AST_visitor_base* stmt_child)
{
    if( not funcDef->funcName )
    {
        if( do_error_reporting )
        {
            cout<< "ERROR: could not define function name '" << funcDef->name <<"' " << funcDef->loc << endl;
        }
        funcDef->symbol_table_verified = false;
    }
    else if( not funcDef->funcType )
    {
        if( do_error_reporting )
        {
            cout<< "ERROR: could not define overload for function '" << funcDef->name <<"' " << funcDef->loc << endl;
        }
        funcDef->symbol_table_verified = false;
    }
    else
    {
        funcDef->symbol_table_verified = true;
    }
}

void verify_symbol_table::expressionStatement_up(expression_statement_AST_node* expStmt)
{
    expStmt->symbol_table_verified = expStmt->expression->symbol_table_verified;
}


void verify_symbol_table::definitionStmt_up(definition_statement_AST_node* defStmt)
{
    if( defStmt->var_type->symbol_table_verified )
    {
        if(not defStmt->variable_symbol)
        {
            if(do_error_reporting)
            {
                cout << "ERROR: variable '" << defStmt->var_name << "' could not be defined " << defStmt->loc << endl;
            }
            defStmt->symbol_table_verified = false;
        }
        else
        {
            defStmt->symbol_table_verified = true;
        }
    }
    else
    {
        defStmt->symbol_table_verified = false;
    }
}

void verify_symbol_table::assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child)
{
    if(not assignStmt->variable_symbol)
    {
        if(do_error_reporting)
        {
            cout << "ERROR: variable '" << assignStmt->var_name << "' does not exist " << assignStmt->loc << endl;
        }
        assignStmt->symbol_table_verified = false;
    }
    else if( not assignStmt->expression->expression_return_type )
    {
        assignStmt->symbol_table_verified = false; // type of expression could not be determined. Assume errored elsewhere.
    }
    else // check type
    {
        auto var_type = assignStmt->variable_symbol->var_type;
        auto exp_type = assignStmt->expression->expression_return_type;

        if( var_type->get_has_assignment( exp_type.get() ) )
        {
            assignStmt->operation_function_name = var_type->get_assignment_name( exp_type.get() );
            assignStmt->symbol_table_verified = true; // YAY!
        }
        else
        {
            if(do_error_reporting)
            {
                cout << "ERROR: type '" << var_type->definition_name << "' can not assign from type '" << exp_type->definition_name << "' " << assignStmt->loc << endl;
            }
            assignStmt->symbol_table_verified = false;
        }


    }
}

void verify_symbol_table::functionCall_Stmt_up(functionCall_statement_AST_node* funcCall, AST_visitor_base* expression_child)
{
    auto calling_type = funcCall->expression->expression_return_type;
    if( calling_type )
    {
        if( calling_type->is_callable() )
        {
            funcCall->specific_overload = calling_type->get_resolved_call(funcCall->loc, funcCall->symbol_table);
            if( not funcCall->specific_overload )
            {
                throw gen_exception("ERROR: cannot get resolved call. This should not be reached.");
                funcCall->symbol_table_verified = false;
            }
            else
            {
                funcCall->symbol_table_verified = true;
            }
        }
        else
        {
            if(do_error_reporting)
            {
                cout << "ERROR: type '" << calling_type->definition_name << "' is not callable " << funcCall->loc << endl;
            }
            funcCall->symbol_table_verified = false;
        }
    }
    else
    {
        funcCall->symbol_table_verified = false;
    }
}

void verify_symbol_table::intLiteral_up(intLiteral_expression_AST_node* intLitExp)
{
    if( intLitExp->expression_return_type )
    {
        intLitExp->symbol_table_verified = true;
    }
    else
    {
        intLitExp->expression_return_type = false;
        if(do_error_reporting)
        {
            cout << "ERROR: cannot define int literal. This error should never be reached" << endl;
        }
    }
}

void verify_symbol_table::binOperator_up(binOperator_expression_AST_node* binOprExp)
{
    if( binOprExp->left_operand->symbol_table_verified and binOprExp->right_operand->symbol_table_verified )
    {
        if( binOprExp->expression_return_type )
        {
            binOprExp->symbol_table_verified = true;
        }
        else
        {
            if(do_error_reporting)
            {
                varType_ptr LHS_type = binOprExp->left_operand->expression_return_type;
                varType_ptr RHS_type = binOprExp->right_operand->expression_return_type;
                cout << "ERROR: LHS type " << LHS_type->definition_name << " and RHS type " << LHS_type->definition_name <<
                " " << binOprExp << " do not have defined addition" << endl;
            }
            binOprExp->symbol_table_verified = true;
        }
    }
    else
    {
        binOprExp->symbol_table_verified = false;
    }
}

void verify_symbol_table::varReferance_up(varReferance_expression_AST_node* varRefExp)
{
    if(not varRefExp->variable_symbol)
    {
        if(do_error_reporting)
        {
            cout << "ERROR: variable '" << varRefExp->var_name << "' does not exist " << varRefExp->loc << endl;
        }
        varRefExp->symbol_table_verified = false;
    }
    else
    {
        varRefExp->symbol_table_verified = true;
    }
}

