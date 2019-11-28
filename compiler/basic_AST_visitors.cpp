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


///// EXCEPTIONS ///
//// symbol not defined
//NameNotDefined_exc::NameNotDefined_exc( csu::utf8_string& _name, csu::location_span& _loc)
//{
//    name = _name;
//    loc = _loc;
//
//    stringstream ss;
//    ss << "name '" << name << "' has not been defined. Referenced " << loc << endl;
//    msg = ss.str();
//}
//
//const char* NameNotDefined_exc::what()
//{
//    return msg.c_str();
//}


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

void set_symbol_table::funcDef_down(function_AST_node* funcDef)
{
    set_symbol_table* par = dynamic_cast<set_symbol_table*>( parent );
    funcDef->symbol_table = par->symbol_table;

    auto table_name = funcDef->name+"_cy_namespace";
    funcDef->inner_symbol_table = make_shared<local_sym_table>(table_name, funcDef->symbol_table);
    symbol_table = funcDef->inner_symbol_table.get(); // now the lower symbols, block and parameter list, will have the inner symbol table
}




/// define_names ///
define_names::define_names()
{
    at_module_level = false;
    is_module = true;
}

define_names::define_names(bool _at_module_level)
{
    at_module_level = _at_module_level;
    is_module = false;
}

shared_ptr< AST_visitor_base > define_names::make_child(int number)
{
    return make_shared<define_names>( is_module );
}

void define_names::cImports_up(import_C_AST_node* ASTnode)
{
    // note: completely set verification_state

    /// type of the variable
    utf8_string TMP = "UNNAMED_C_TYPE";
    bool check_order = false;
    ASTnode->variable_type = ASTnode->symbol_table->get_type_global(TMP, ASTnode->loc, check_order);
    if( not ASTnode->variable_type )
    {
        throw gen_exception("cannot import UNNAMED_C_TYPE. This should never be reached.");
        ASTnode->verification_state = 0;
    }

    /// named variable
    if( ASTnode->verification_state!=0)
    {
        bool check_exclusive = true;
        ASTnode->variable = ASTnode->symbol_table->new_variable( ASTnode->variable_type, ASTnode->usage_name, ASTnode->loc, check_exclusive, false);
        if( not check_exclusive )
        {
            cout<<"cannot import name "<<ASTnode->usage_name<<" already defined. Imported "<<ASTnode->loc<<endl;
            ASTnode->verification_state = 0;
        }
        else
        {
            ASTnode->variable->C_name = ASTnode->import_name;
            ASTnode->verification_state = 1;
        }
    }

    /// named type
    if( ASTnode->verification_state!=0)
    {
        shared_ptr<varType_fromC> T = make_shared<varType_fromC>();
        T->unnamed_C_type = ASTnode->variable_type;
        ASTnode->type = T;
        ASTnode->type->definition_name = ASTnode->usage_name;
        ASTnode->type->C_name = ASTnode->import_name;
        ASTnode->type->loc = ASTnode->loc;
        ASTnode->type->is_ordered = false;

        bool check_exclusive = false;
        ASTnode->symbol_table->add_type( ASTnode->type, check_exclusive );
        if( not check_exclusive )
        {
            throw gen_exception("cannot import C type. This should never be reached.");
            ASTnode->verification_state = 0;
        }
        else
        {
            ASTnode->verification_state = 1;
        }
    }

    if( ASTnode->verification_state == -1 )
    {
        throw gen_exception("strange error in define_names::cImports_up. This should never be reached.");
        ASTnode->verification_state = 0;
    }
}

void define_names::definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child)
{
// partially set verification_state

    bool do_ordered = not at_module_level;
    bool check_exclusive = true;
    defStmt->variable_symbol = defStmt->symbol_table->new_variable(nullptr, defStmt->var_name, defStmt->loc, check_exclusive, do_ordered);

    if( not check_exclusive)
    {
        defStmt->verification_state = 0;
    }
}

void define_names::funcDef_down(function_AST_node* funcDef)
{
    //bool do_ordered = not at_module_level;

    bool check_order = false;
    varName_ptr resolved_name = funcDef->symbol_table->get_variable_global( funcDef->name, funcDef->loc, check_order );

    if( resolved_name )
    {
        if( resolved_name->is_ordered )// and not do_ordered)
        {
            resolved_name->is_ordered  = false; // ... hard to explain why...
        }
    }
    else
    // entirely new stuff!
    {
        bool check_exclusive = false;
        resolved_name = funcDef->symbol_table->new_variable( nullptr, funcDef->name, funcDef->loc, check_exclusive, false ); // ordered needs to be false, essentually becouse defined functions on the whole do not have a location (but specific overloads do)
        if( not resolved_name )
        {
            throw gen_exception("strange error in define_names::funcDef_down. This should never be reached.");
        }
    }

    funcDef->funcName = resolved_name;
}





/// reference_names ///
shared_ptr< AST_visitor_base > reference_names::make_child(int number)
{
    return make_shared<reference_names>( );
}

void reference_names::varTypeRepr_up(varType_ASTrepr_node* varTypeRepr)
{
    // completely set verification_state

    bool check_order = true;
    varTypeRepr->resolved_type = varTypeRepr->symbol_table->get_type_global(varTypeRepr->name, varTypeRepr->loc, check_order);

    if( not check_order )
    {
        varTypeRepr->verification_state = 0;
    }
    else if( not varTypeRepr->resolved_type ) // does not exist
    {
        varTypeRepr->verification_state = 0;
        cout << "name '" << varTypeRepr->name << "' has not been defined. Referenced " << varTypeRepr->loc << endl;
    }
    else
    {
        varTypeRepr->verification_state = 1;
    }
}

void reference_names::assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child)
{
    bool check_order = true;
    assignStmt->variable_symbol = assignStmt->symbol_table->get_variable_global( assignStmt->var_name, assignStmt->loc, check_order );

    if( not check_order )
    {
        assignStmt->verification_state = 0;
    }
    else if( not assignStmt->variable_symbol )
    {
        cout << "name '" << assignStmt->var_name<< "' has not been defined. Referenced " << assignStmt->loc << endl;
        assignStmt->verification_state = 0;
    }
}

void reference_names::varReferance_up(varReferance_expression_AST_node* varRefExp)
{

    bool check_order = true;

    varRefExp->variable_symbol = varRefExp->symbol_table->get_variable_global( varRefExp->var_name, varRefExp->loc, check_order );

    if( not check_order )
    {
        varRefExp->verification_state = 0;
    }
    else if( not varRefExp->variable_symbol )
    {
        cout << "name '" << varRefExp->var_name<< "' has not been defined. Referenced " << varRefExp->loc << endl;
        varRefExp->verification_state = 0;
    }
}




/// register overloads of functions and methods ///

register_overloads::register_overloads()
{
    is_module = true;
    at_module_level = false;
}

register_overloads::register_overloads(bool _at_module_level)
{
    is_module = false;
    at_module_level = _at_module_level;
}

shared_ptr< AST_visitor_base > register_overloads::make_child(int number)
{
    return make_shared<register_overloads>( is_module );
}

void register_overloads::funcDef_down(function_AST_node* funcDef)
{
    if( not funcDef->funcName )
    {
        throw gen_exception("function variable: ", funcDef->name, ", " , funcDef->funcName->loc, ". Was not defined. Good luck!");
    }

    /// THE GENERIC FUNCTION TYPE
    DefFuncType_ptr specific_function_type = nullptr;
    if( funcDef->funcName->var_type )
    {
        if( not ( funcDef->funcName->var_type->type_of_type == varType::defined_function_t ) )
        {
            cout << "name " << funcDef->name << " already defined " << funcDef->funcName->var_type->loc << " with type " << funcDef->funcName->var_type->definition_name
               << ". Define function error " << funcDef->loc << endl;
            funcDef->verification_state = 0;
            return;
        }
        else
        {
            specific_function_type = dynamic_pointer_cast<DefFuncType>( funcDef->funcName->var_type );
        }
    }
    else
    {
        specific_function_type = make_shared<DefFuncType>(funcDef->name, funcDef->loc );
    }

    funcDef->funcType = specific_function_type;
    funcDef->funcName->var_type = specific_function_type;


    /// THE SPECIFIC OVERLOAD
    DefFuncType::ResolvedFunction_ptr specific_overload = funcDef->funcType->define_overload( funcDef->loc, funcDef->symbol_table, not at_module_level ); // eventually this could error

    if( not specific_overload )
    {
        funcDef->verification_state = 0;
        return;
    }
    else
    {
        funcDef->specific_overload = specific_overload;
    }
}





/// build the types of everything ///
build_types::build_types()
{
    changes_were_made = false;
}

shared_ptr< AST_visitor_base > build_types::make_child(int number)
{
    return make_shared<build_types>( );
}

void build_types::ASTnode_up(AST_node* ASTnode)
{
    for(auto it : children)
    {
        shared_ptr< build_types > chil = dynamic_pointer_cast<build_types>( it );
        if( chil->changes_were_made )
        {
            changes_were_made = true;
            break;
        }
    }
}

void build_types::definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child)
{
    // completely verify

    if( defStmt->verification_state == 0 )
    {
        return;
    }

    if( defStmt->var_type->verification_state != 1 ) // this completely verified in an earlier visitor
    {
        defStmt->verification_state = 0;
        return;
    }

    if( not defStmt->variable_symbol->var_type )
    {

        defStmt->variable_symbol->var_type = defStmt->var_type->resolved_type;
        changes_were_made = true;

        if( not defStmt->variable_symbol->var_type->can_be_defined() )
        {
            cout << "type " << defStmt->variable_symbol->var_type->definition_name << " cannot be defined. Referenced "  << defStmt->loc << endl;
            defStmt->verification_state = 0;
            return;
        }

        defStmt->verification_state = 1;
    }
}

void build_types::functionCall_Stmt_up(functionCall_statement_AST_node* funcCall, AST_visitor_base* expression_child)
{
    if( funcCall->expression->expression_return_type  and not funcCall->function_to_write)
    {
        if( not funcCall->expression->expression_return_type->is_callable() )
        {
            cout << "type " << funcCall->expression->expression_return_type->definition_name << " cannot be called. Referenced "  << funcCall->loc << endl;
            funcCall->verification_state = 0;
            return;
        }

        // TODO: fix this when we have signitures
        funcCall->function_to_write = funcCall->expression->expression_return_type->get_resolved_call(funcCall->loc, funcCall->symbol_table);
        if( not funcCall->function_to_write )
        {
            cout<< "ERROR: cannot get resolved call. " << funcCall->loc << endl;
            funcCall->verification_state = 0;
            return;
        }

        funcCall->verification_state = 1;
        changes_were_made = true;
    }
}

void build_types::assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child)
{
    if( assignStmt->verification_state == 0 )
    {
        return;
    }


    auto var_type = assignStmt->variable_symbol->var_type;
    auto exp_type = assignStmt->expression->expression_return_type;
    if((assignStmt->verification_state==-1) and var_type and exp_type )
    {
        if( var_type->get_has_assignment( exp_type.get() ) )
        {
            changes_were_made = true;
            assignStmt->verification_state = 1;
        }
        else
        {
            cout << "ERROR: type '" << var_type->definition_name << "' can not assign from type '" << exp_type->definition_name << "' " << assignStmt->loc << endl;
            assignStmt->verification_state = 0;
            return;
        }
    }
}

void build_types::intLiteral_up(intLiteral_expression_AST_node* intLitExp)
{
// fully verfied!
    if( not intLitExp->expression_return_type)
    {
        utf8_string TMP = "UNNAMED_C_TYPE";
        bool check_order = false;
        intLitExp->expression_return_type = intLitExp->symbol_table->get_type_global(TMP, intLitExp->loc, check_order);

        if( not intLitExp->expression_return_type )
        {
            throw gen_exception( "cannot get UNNAMED_C_TYPE in build_types::intLiteral_up. This should never be reached" );
        }

        intLitExp->verification_state = 1;
        changes_were_made = true;
    }

}

void build_types::binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
{
    // NOTE: we are assuming LHS + RHS at this point
    if( binOprExp->left_operand->verification_state==0 or binOprExp->right_operand->verification_state==0  )
    {
        binOprExp->verification_state = 0;
        return;
    }

    varType_ptr LHS_type = binOprExp->left_operand->expression_return_type;
    varType_ptr RHS_type = binOprExp->right_operand->expression_return_type;
    if( LHS_type and RHS_type and not binOprExp->expression_return_type)
    {

        if( LHS_type->get_has_LHSaddition( RHS_type.get() ) )
        {
            binOprExp->expression_return_type = LHS_type->get_LHSaddition_type( RHS_type.get() );
            changes_were_made = true;

            binOprExp->verification_state = 1;
        }
        else
        {
            binOprExp->verification_state = 0;
            cout << LHS_type->definition_name << " and " << RHS_type->definition_name << " do not have addition " << binOprExp->loc << endl;
        }

    }
}

void build_types::varReferance_up(varReferance_expression_AST_node* varRefExp)
{
    // fully verified

    if( varRefExp->verification_state == 0 )
    {
        return;
    }


    if( (not varRefExp->expression_return_type) and varRefExp->variable_symbol->var_type)
    {

        varRefExp->expression_return_type  = varRefExp->variable_symbol->var_type;

        changes_were_made = true;
        varRefExp->verification_state = 1;
    }
}


//// verify symbol table ////

verify_symbol_table::verify_symbol_table()
{
    is_verified = false;
    do_acclimation = true;
    verification_state = -1;
}

shared_ptr< AST_visitor_base > verify_symbol_table::make_child(int number)
{
    return make_shared<verify_symbol_table>( );
}

void verify_symbol_table::ASTnode_up(AST_node* ASTnode)
{
    if( ASTnode->verification_state != -1 )
    {
        verification_state = ASTnode->verification_state;
    }
    else if( do_acclimation )
    {
        ASTnode->verification_state = 1;

        for( auto child : children )
        {
            auto verify_child = dynamic_pointer_cast<verify_symbol_table>( child );
            if( verify_child->verification_state == 0 )
            {
                ASTnode->verification_state = 0;
                break;
            }
            else if( verify_child->verification_state == -1 )
            {
                throw gen_exception("strange problem in verify_symbol_table::ASTnode_up. This should not be reached!");
            }
        }

        verification_state = ASTnode->verification_state;
    }
}



