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


/// set symbol table ///
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
    auto A = make_shared<set_symbol_table>( this );
    return A;
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

    auto table_name = funcDef->name+"_namespace_";
    table_name += funcDef->symbol_table->get_unique_string();
    funcDef->inner_symbol_table = make_shared<local_sym_table>(table_name, funcDef->symbol_table);
    symbol_table = funcDef->inner_symbol_table.get(); // now the lower symbols, block and parameter list, will have the inner symbol table
}

void set_symbol_table::ClassDef_down(class_AST_node* class_node)
{
    set_symbol_table* par = dynamic_cast<set_symbol_table*>( parent );
    class_node->symbol_table = par->symbol_table;

    auto table_name = class_node->name+"_namespace_";
    table_name += class_node->symbol_table->get_unique_string();
    class_node->inner_symbol_table = make_shared<local_sym_table>(table_name, class_node->symbol_table);
    symbol_table = class_node->inner_symbol_table.get(); // now the lower symbols, block and parameter list, will have the inner symbol table
}

void set_symbol_table::methodDef_down(method_AST_node* methodDef)
{
    set_symbol_table* par_vstr = dynamic_cast<set_symbol_table*>( parent );
    methodDef->symbol_table = par_vstr->symbol_table; // the symbol table of the class

    // get parent table, nesting is weird here.
    auto parent_table = dynamic_cast<local_sym_table*>( par_vstr->symbol_table );

    // make and set inner table
    auto table_name = methodDef->name+"_namespace_";
    table_name += methodDef->symbol_table->get_unique_string();
    methodDef->inner_symbol_table = make_shared<local_sym_table>(table_name, parent_table->parent_table); // note that the methods inner table is a child of the table enclosign the class, NOT the class table
    symbol_table = methodDef->inner_symbol_table.get(); // now the lower symbols, block and parameter list, will have the inner symbol table

    // define self name
    utf8_string def_name = "self";
    location_span loc; // set to some default value
    bool is_exclusive = true;
    bool is_ordered = false;
    methodDef->self_name = symbol_table->new_variable( nullptr, def_name, loc, is_exclusive, is_ordered);

    if(not is_exclusive)
    {
        throw gen_exception("weird error A in set_symbol_table::methodDef_down. This should not be reached");
    }
    else if(not methodDef->self_name)
    {
        throw gen_exception("weird error B in set_symbol_table::methodDef_down. This should not be reached");
    }
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
        T->set_pointers( T, ASTnode->variable_type );
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

    //bool do_ordered = not at_module_level;
    bool check_exclusive = true;
    defStmt->variable_symbol = defStmt->symbol_table->new_variable(nullptr, defStmt->var_name, defStmt->loc, check_exclusive, true);

    if( not check_exclusive)
    {
        defStmt->verification_state = 0;
    }
}

void define_names::autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child)
{
    //bool do_ordered = not at_module_level;
    bool check_exclusive = true;
    autoStmt->variable_symbol = autoStmt->symbol_table->new_variable(nullptr, autoStmt->var_name, autoStmt->loc, check_exclusive, true);

    if( not check_exclusive)
    {
        autoStmt->verification_state = 0;
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

void define_names::baseParams_up(function_parameter_list::base_parameters_T* baseParams, std::list<AST_visitor_base*>& visitor_children)
{
    for(auto& params : baseParams->param_list)
    {

        bool check_exclusive = true;
        params.variable_symbol = baseParams->symbol_table->new_variable(nullptr, params.var_name, params.loc, check_exclusive, true);

        if( not check_exclusive)
        {
            baseParams->verification_state = 0;
            return;
        }
    }
}

void define_names::ClassDef_down( class_AST_node* class_node)
{
    //at_module_level
    utf8_string C_name = class_node->symbol_table->namespace_unique_name;
    C_name += class_node->name;
    C_name += class_node->symbol_table->get_unique_string();

    class_node->type_ptr = make_shared<DefClassType>( class_node->name, C_name, not at_module_level, class_node->inner_symbol_table,  class_node->loc );
    CPntrRef_ptr self_pointer_type = nullptr;

    bool check_exclusive = true;
    class_node->symbol_table->add_type( class_node->type_ptr, check_exclusive );

    if( not check_exclusive )
    {
        class_node->verification_state = 0;
        class_node->type_ptr = nullptr;
    }
    else
    {
        self_pointer_type = make_shared<raw_C_pointer_reference>(class_node->type_ptr);
    }

    for( method_AST_ptr mthd : class_node->method_definitions )
    {
        mthd->class_type = class_node->type_ptr;
        mthd->self_name->var_type = self_pointer_type;
    }
}

void define_names::ClassVarDef_down( class_varDefinition_AST_node* class_var_def )
{
    bool check_exclusive = true;
    class_var_def->variable_symbol = class_var_def->symbol_table->new_variable(nullptr, class_var_def->var_name, class_var_def->loc, check_exclusive, false);

    if( not check_exclusive)
    {
        class_var_def->verification_state = 0;
    }
}

void define_names::methodDef_down(method_AST_node* methodDef)
{
    if( not methodDef->class_type )
    {
        methodDef->verification_state = 0;
        return;
    }

    bool check_order = false;
    varName_ptr resolved_name = methodDef->symbol_table->get_variable_local( methodDef->name, methodDef->loc, check_order ); // note this is the class symbol table

    if( not resolved_name )
    // entirely new stuff!
    {
        bool check_exclusive = false;
        resolved_name = methodDef->symbol_table->new_variable( nullptr, methodDef->name, methodDef->loc, check_exclusive, false );
        if( not resolved_name )
        {
            throw gen_exception("strange error in define_names::methodDef_down. This should never be reached.");
        }
    }

    methodDef->methodName = resolved_name;
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

void reference_names::LHS_varRef_up(LHS_varReference* varref)
{
    bool check_order = true;
    varref->variable_symbol = varref->symbol_table->get_variable_global( varref->var_name, varref->loc, check_order );

    if( not check_order )
    {
        varref->verification_state = 0;
    }
    else if( not varref->variable_symbol )
    {
        cout << "name '" << varref->var_name<< "' has not been defined. Referenced " << varref->loc << endl;
        varref->verification_state = 0;
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
    current_callable = nullptr;
}

register_overloads::register_overloads(bool _at_module_level, callableDefinition_AST_node* _current_callable)
{
    is_module = false;
    at_module_level = _at_module_level;
    current_callable = _current_callable;
}

shared_ptr< AST_visitor_base > register_overloads::make_child(int number)
{
    return make_shared<register_overloads>( is_module, current_callable );
}

void register_overloads::callableDef_down(callableDefinition_AST_node* callDef)
{
    current_callable = callDef;
}

void register_overloads::returnStatement_down(return_statement_AST_node* returnStmt)
{
    if(not current_callable)
    {
        returnStmt->verification_state = 0;
        cout<< "return statement is not in function "<<returnStmt->loc<<endl;
    }
    else
    {
        current_callable->num_returns += 1;
        returnStmt->callable_to_escape = current_callable;
    }
}

void register_overloads::baseParams_down(function_parameter_list::base_parameters_T* baseParams)
{

    for(auto& param : baseParams->param_list)
    {
        if( param.var_type_ASTnode->verification_state != 1 ) // this completely verified in an earlier visitor
        {
            baseParams->verification_state = 0;
            return;
        }

        param.variable_symbol->var_type = param.var_type_ASTnode->resolved_type;

    }
}

void register_overloads::funcDef_up(function_AST_node* funcDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                                    AST_visitor_base* funcBody_child)
{
    if( funcDef->paramList->verification_state==0 )
    {
        funcDef->verification_state = 0;
        return;
    }

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
    auto parameter_info = funcDef->paramList->get_parameter_info();
    DefFuncType::ResolvedFunction_ptr specific_overload = funcDef->funcType->define_overload( funcDef->loc, funcDef->symbol_table, parameter_info, not at_module_level ); // eventually this could error

    if( not specific_overload )
    {
        funcDef->verification_state = 0;
        return;
    }
    else
    {
        funcDef->specific_overload = specific_overload;
    }

    /// check return type
    if( funcDef->return_type_ASTnode )
    {
        if( funcDef->return_type_ASTnode->verification_state!=1 )
        {
            funcDef->verification_state = 0;
            return;
        }
        else
        {
            funcDef->return_type = funcDef->return_type_ASTnode->resolved_type;
        }
    }
}

void register_overloads::methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child)
{
    if( methodDef->paramList->verification_state==0 )
    {
        methodDef->verification_state = 0;
        return;
    }

    if( not methodDef->methodName )
    {
        throw gen_exception("method name variable: ", methodDef->name, ", " , methodDef->methodName->loc, ". Was not defined. Good luck!");
    }

    /// THE GENERIC FUNCTION TYPE
    MethodType_ptr specific_function_type = nullptr;
    if( methodDef->methodName->var_type )
    {
        if( not ( methodDef->methodName->var_type->type_of_type == varType::method_function_t ) )
        {
            cout << "name " << methodDef->name << " already defined " << methodDef->methodName->var_type->loc << " with type "
                    << methodDef->methodName->var_type->definition_name << ". Define function error " << methodDef->loc << endl;
            methodDef->verification_state = 0;
            return;
        }
        else
        {
            specific_function_type = dynamic_pointer_cast<MethodType>( methodDef->methodName->var_type );
        }
    }
    else
    {
        specific_function_type = make_shared<MethodType>(methodDef->name, methodDef->loc, methodDef->self_name );
    }

    methodDef->funcType = specific_function_type;
    methodDef->methodName->var_type = specific_function_type;


    /// THE SPECIFIC OVERLOAD
    auto parameter_info = methodDef->paramList->get_parameter_info();
    MethodType::ResolvedMethod_ptr specific_overload = methodDef->funcType->define_overload( methodDef->loc, parameter_info, methodDef->symbol_table);

    if( not specific_overload )
    {
        methodDef->verification_state = 0;
        return;
    }
    else
    {
        methodDef->specific_overload = specific_overload;
    }

    /// check return type
    if( methodDef->return_type_ASTnode )
    {
        if( methodDef->return_type_ASTnode->verification_state !=1 )
        {
            methodDef->verification_state = 0;
            return;
        }
        else
        {
            methodDef->return_type = methodDef->return_type_ASTnode->resolved_type;
        }
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

void build_types::callableDef_up(callableDefinition_AST_node* callDef, AST_visitor_base* paramList_child)
{
    if( callDef->verification_state == 0 )
    {
        return;
    }

    if( callDef->num_verified_returns == -1 )
    {
        callDef->verification_state = 0;
        return;
    }


    if( callDef->num_verified_returns == callDef->num_returns)
    {
        changes_were_made = true;

        if( not callDef->return_type and callDef->num_verified_returns==0 )
        {
            bool check_order = false;
            utf8_string void_name = "void";
            callDef->return_type = callDef->symbol_table->get_type_global(void_name, callDef->loc, check_order);

            if( not callDef->return_type)
            {
                throw gen_exception("cannot get void type in build_types::callableDef_up" );
            }
        }

        callDef->notify_return_type(); // I hope this works?
        callDef->num_verified_returns = -2;// so we can notify precisely once
    }
}

void build_types::baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children)
{
    // potentially verify
    if( argList->verification_state == -1 )
    {
        // check that we are ready to verify
        for(auto& exp : argList->arguments )
        {
            if( exp->verification_state == 0 )
            {
                changes_were_made = true;
                argList->verification_state = 0; // this is bad if children are bad
                return;
            }
            else if( not exp->expression_return_type )
            {
                return; // we are not ready! Type not found yet.
            }
            else if( not exp->expression_return_type->type_is_fully_defined() )
            {
                return; // type is found, but not fully defined
            }
        }

        // if we are ready, then all expressions are good!
        changes_were_made = true;
        argList->verification_state = 1;
    }
}

void build_types::callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs)
{
    // potentially verify
    if( callArgs->verification_state == -1)
    {
        callArgs->verification_state = 1; // assume. Change if needed.

        if(callArgs->unnamed_argument_list)
        {
            if( callArgs->unnamed_argument_list->verification_state == 0 )
            {
                changes_were_made = true;
                callArgs->verification_state = 0;
                return;
            }
            else if( callArgs->unnamed_argument_list->verification_state == -1 )
            {
                callArgs->verification_state = -1;
                return;
            }
        }

        if(callArgs->named_argument_list)
        {
            if( callArgs->named_argument_list->verification_state == 0 )
            {
                changes_were_made = true;
                callArgs->verification_state = 0;
                return;
            }
            else if( callArgs->named_argument_list->verification_state == -1 )
            {
                callArgs->verification_state = -1;
                return;
            }
        }

        // now verified!
        changes_were_made = true;
    }
}

void build_types::reqParams_up(function_parameter_list::required_params* reqParams, std::list<AST_visitor_base*>& visitor_children)
{
    if( reqParams->verification_state!=-1 )
    {
        return;
    }

    for(auto& param : reqParams->param_list)
    {

        if( not param.variable_symbol->var_type->type_is_fully_defined() )
        {
            return; // type not fully defined yet. cannot do anything.
        }
        else if( not param.variable_symbol->var_type->can_be_defined() )
        {
            cout << "type " << param.variable_symbol->var_type->definition_name << " cannot be defined. Referenced "  << param.loc << endl;
            reqParams->verification_state = 0;
            changes_were_made = true;
            return;
        }
    }

    // if we make it here then we are good!
    reqParams->verification_state = 1;
    changes_were_made = true;
}

void build_types::defaultParams_up(function_parameter_list::defaulted_params* defParams,
                                  std::list<AST_visitor_base*>& param_name_visitors, std::list<AST_visitor_base*>& default_exp_visitors)
{
    if( defParams->verification_state!=-1 )
    {
        return;
    }

    int num = defParams->param_list.size();
    if( num != defParams->parameter_defaults.size() )
    {
        throw gen_exception("strange error in build_types::defaultParams_up. This should not be reached.");
    }

    auto param_iter = defParams->param_list.begin();
    auto default_exp_iter = defParams->parameter_defaults.begin();
    for( int i=0; i<num; i++ )
    {
        /// check goodness of expression ///
        auto exp = *default_exp_iter;
        if( exp->verification_state == 0 )
        {
            defParams->verification_state = 0;
            changes_were_made = true;
            return;
        }
        if( exp->verification_state == -1 )
        {
            return;// retreat to verify another day
        }

        auto var_type = (*param_iter).var_type_ASTnode->resolved_type;
        auto exp_type = exp->expression_return_type;

        /// check goodness of var_type ///
        if( not var_type->type_is_fully_defined() )
        {
            return; // type not fully defined yet. cannot do anything.
        }
        if( not var_type->can_be_defined() )
        {
            cout << "type " << var_type->definition_name << " cannot be defined. Referenced "  << (*param_iter).loc << endl;
            defParams->verification_state = 0;
            changes_were_made = true;
            return;
        }

        /// now check compatability ///
        if( not var_type->get_has_assignment( exp_type.get() ) )
        {
            cout << "ERROR: type '" << var_type->definition_name << "' can not assign from type '" << exp_type->definition_name << "' " << (*param_iter).loc << endl;
            defParams->verification_state = 0;
            changes_were_made = true;
            return;
        }

        ++param_iter;
        ++default_exp_iter;
    }

    // if we make it here then we are good!
    defParams->verification_state = 1;
    changes_were_made = true;

}


void build_types::generic_VarDef_up(General_VarDefinition* var_def, AST_visitor_base* var_type)
{
    // potentially verify

    if( var_def->verification_state != -1 )
    {
        return;
    }

    if( var_def->var_type->verification_state != 1 ) // this completely verified in an earlier visitor
    {
        var_def->verification_state = 0;
        return;
    }

    if( not var_def->variable_symbol->var_type )
    {
        var_def->variable_symbol->var_type = var_def->var_type->resolved_type;
        changes_were_made = true;
    }

    if( var_def->variable_symbol->var_type )
    {
        if( not var_def->variable_symbol->var_type->type_is_fully_defined() )
        {
            return;
        }
        else if( not var_def->variable_symbol->var_type->can_be_defined() )
        {
            changes_were_made = true;
            var_def->verification_state = 0;
            return;
        }
        else
        {
            changes_were_made = true;
            var_def->verification_state = 1;
        }
    }
}

void build_types::assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child)
{
    // we will need to verify that the top level of the LHS reference can be set with the RHS type
    // here is a top level visitor to do this

    class LHS_setter_checker : public AST_visitor_NoChildren
        // NOTE: UP visitors will NEVER be called
    {
    public:
        varType_ptr RHS_type; // set in constructor

        LHS_setter_checker( varType_ptr _RHS_type )
        {
            RHS_type = _RHS_type;
        }

        void LHS_varRef_down(LHS_varReference* varref)
        {
            if( not varref->reference_type->get_has_assignment( RHS_type.get() ) )
            {
                cout << "ERROR: type '" << varref->reference_type->definition_name << "' can not assign from type '" << RHS_type->definition_name << "'" << endl;
                varref->verification_state = 0;
            }
        }

        void LHS_accessor_down(LHS_accessor_AST_node* LHSaccess)
        {
            auto T = LHSaccess->LHS_exp->reference_type->set_member( LHSaccess->name, RHS_type.get() );

            if( not T )
            {
                cout << "ERROR: type '" << LHSaccess->LHS_exp->reference_type->definition_name << "' can not be set from type '" << RHS_type->definition_name << "'" << endl;
                LHSaccess->verification_state = 0;
            }
        }
    };

    auto reference_type = assignStmt->LHS->reference_type;
    auto exp_type = assignStmt->expression->expression_return_type;
    if((assignStmt->verification_state==-1) and reference_type and exp_type )
    {
        if( not reference_type->type_is_fully_defined() )
        {
            return;
        }
        if( not exp_type->type_is_fully_defined() )
        {
            return;
        }

//        if( reference_type->get_has_assignment( exp_type.get() ) )
//        {
//            changes_were_made = true;
//            assignStmt->verification_state = 1;
//        }
//        else
//        {
//            cout << "ERROR: type '" << reference_type->definition_name << "' can not assign from type '" << exp_type->definition_name << "' " << assignStmt->loc << endl;
//            assignStmt->verification_state = 0;
//            return;
//        }

        LHS_setter_checker checker( exp_type );
        assignStmt->LHS->apply_visitor( &checker );

        if( assignStmt->LHS->verification_state == 0 )
        {
            cout << assignStmt->loc << endl;
            assignStmt->verification_state = 0;
        }
        else
        {
            assignStmt->verification_state = 1;
            changes_were_made = true;
            return;
        }
    }
}

void build_types::autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child)
{

    auto exp_type = autoStmt->expression->expression_return_type;

    if( exp_type and (not autoStmt->variable_symbol->var_type) and autoStmt->verification_state == -1 )
    {
        if( not exp_type->type_is_fully_defined() )
        {
            return;
        }

        auto autotype = exp_type->get_auto_type();
        if(not autotype)
        {
            cout << "ERROR: type '" << exp_type->definition_name << "' does not have auto-type. " << autoStmt->loc << endl;
            autoStmt->verification_state = 0;
            changes_were_made = true;
            return;
        }

        autoStmt->variable_symbol->var_type = autotype;
        //autoStmt->verification_state = 1;
        changes_were_made = true;
    }

    if( autoStmt->variable_symbol->var_type and autoStmt->verification_state == -1  )
    {
        auto type = autoStmt->variable_symbol->var_type;

        if( not type->type_is_fully_defined() )
        {
            return;
        }

        if(not type->can_be_defined())
        {
            cout << "type " << type->definition_name << " cannot be defined. Referenced "  << autoStmt->loc << endl;
            autoStmt->verification_state = 0;
            return;
        }

        autoStmt->verification_state = 1;
        changes_were_made = true;
    }
}

void build_types::returnStatement_up(return_statement_AST_node* returnStmt, AST_visitor_base* expression_child)
{

    if( returnStmt->verification_state == -1  and returnStmt->expression->verification_state !=-1 )
    {
        if( returnStmt->callable_to_escape->num_verified_returns == -1 ) // cannot verify if other return statements are bad
        {
            returnStmt->verification_state = 0;
            return;
        }

        if( returnStmt->expression->verification_state == 0 )
        {
            returnStmt->verification_state = 0;
            returnStmt->callable_to_escape->num_verified_returns = -1;
            return;
        }

        if( returnStmt->expression->expression_return_type->definition_name == "void" )
        {
            cout << "cannot return void " << returnStmt->loc << endl;
            returnStmt->verification_state = 0;
            returnStmt->callable_to_escape->num_verified_returns = -1;
            return;
        }

        if( returnStmt->callable_to_escape->returnType_mode == callableDefinition_AST_node::by_returnStatement_M )
        {
            // return statements implictly define return type
            // check all rules!
            if( not returnStmt->callable_to_escape->return_type ) // not defined!, set it ourselves
            {
                returnStmt->callable_to_escape->return_type = returnStmt->expression->expression_return_type;
                returnStmt->verification_state = 1;
                returnStmt->callable_to_escape->num_verified_returns += 1;
                changes_were_made = true;
                return;
            }
            else
            {
                auto ret_T = returnStmt->callable_to_escape->return_type;
                auto exp_T = returnStmt->expression->expression_return_type;

                if( not ret_T->type_is_fully_defined() )
                {
                    return;
                }
                if( not exp_T->type_is_fully_defined() )
                {
                    return;
                }

                if( ret_T->is_equivalent( exp_T.get() ) )
                {
                    returnStmt->verification_state = 1;
                    returnStmt->callable_to_escape->num_verified_returns += 1;
                    changes_were_made = true;
                    return;
                }
                else
                {
                    cout<< "return types must be equivalent " << returnStmt->loc << endl;
                    returnStmt->verification_state = 0;
                    returnStmt->callable_to_escape->num_verified_returns = -1;
                    changes_were_made = true;
                    return;

                }
            }
        }
        else // returnStmt->callable_to_escape->returnType_mode == callableDefinition_AST_node::by_explicitDefinition_M
        {
            auto ret_T = returnStmt->callable_to_escape->return_type;
            auto exp_T = returnStmt->expression->expression_return_type;

            if( not ret_T->type_is_fully_defined() )
            {
                return;
            }
            if( not exp_T->type_is_fully_defined() )
            {
                return;
            }

            if( ret_T->get_has_assignment( exp_T.get() ) )
            {
                returnStmt->verification_state = 1;
                returnStmt->callable_to_escape->num_verified_returns += 1;
                changes_were_made = true;
                return;
            }
            else
            {
                cout<< "cannot convert type " << (exp_T->definition_name) << " to " << (ret_T->definition_name) <<
                        " in return " << returnStmt->loc << endl;
                returnStmt->verification_state = 0;
                returnStmt->callable_to_escape->num_verified_returns = -1;
                changes_were_made = true;
                return;
            }
        }
    }
}

void build_types::LHS_varRef_up(LHS_varReference* varref)
{
    // fully verified
    if( varref->verification_state == -1 )
    {


        if( (not varref->reference_type) and varref->variable_symbol->var_type)
        {
            varref->reference_type  = varref->variable_symbol->var_type;

    //        if( varref->level == 0 )
                // then we do nothing, top level is check later!
    //        else if( varref->level == 1 )
                // we can always reference a varible!

            changes_were_made = true;
            varref->verification_state = 1;
        }

    }
}

void build_types::LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor)
{
    if( LHSaccess->verification_state == -1 )
    {
        if( LHSaccess->LHS_exp->verification_state == 1 )
        {
            auto T = LHSaccess->LHS_exp->reference_type;

            varType_ptr retT = nullptr;
            if( LHSaccess->level==0 ) // top level, use setter
            {
                retT = T->set_member( LHSaccess->name ); // check type later
            }
            else if( LHSaccess->level==1 )
            {
                retT = T->member_has_getref( LHSaccess->name );
            }

            if( retT )
            {
                LHSaccess->reference_type = retT;
                LHSaccess->verification_state = 1;
                changes_were_made = true;
            }
            else
            {
                cout<<"ERROR type \""<< T->definition_name << "\" cannot set member \"" << LHSaccess->name << "\" " << LHSaccess->loc << endl;
                cout<< "      (but I bet your Dad does!)" << endl;
                LHSaccess->verification_state = 0;
                return;
            }
        }
        else if(LHSaccess->LHS_exp->verification_state == 0)
        {
            LHSaccess->verification_state = 0;
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
        if( not LHS_type->type_is_fully_defined() )
        {
            return;
        }
        if( not RHS_type->type_is_fully_defined() )
        {
            return;
        }

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

void build_types::ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor)
{
    if( parenGroupExp->expression->verification_state==0 )
    {
        parenGroupExp->verification_state = 0;
        return;
    }

    varType_ptr EXP_type = parenGroupExp->expression->expression_return_type;
    if( EXP_type and not parenGroupExp->expression_return_type)
    {
        parenGroupExp->expression_return_type = EXP_type;
        changes_were_made = true;
        parenGroupExp->verification_state = 1;
    }
}

void build_types::functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child)
{
    if( (funcCall->verification_state == -1) and funcCall->expression->expression_return_type and (funcCall->argument_list->verification_state==1) )
    {
        if( not funcCall->expression->expression_return_type->type_is_fully_defined() )
        {
            return;
        }

        if( not funcCall->expression->expression_return_type->is_callable() )
        {
            cout << "type " << funcCall->expression->expression_return_type->definition_name << " cannot be called. Referenced "  << funcCall->loc << endl;
            funcCall->verification_state = 0;
            return;
        }

        auto argument_types = funcCall->argument_list->get_argument_info();
        funcCall->function_to_write = funcCall->expression->expression_return_type->get_resolved_call(funcCall->loc, funcCall->symbol_table, argument_types);

        if( not funcCall->function_to_write )
        {
            cout<< "ERROR: cannot resolved call. " << funcCall->loc << endl;
            funcCall->verification_state = 0;
            return;
        }

        auto return_type = funcCall->function_to_write->return_type;
        if( not return_type )
        {
            throw gen_exception("Strange error in build_types::functionCall_Exp_up. This should not be reached.");
        }

        funcCall->expression_return_type = return_type;

        funcCall->verification_state = 1;
        changes_were_made = true;
    }

}

void build_types::accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor)
{
    if( accessorExp->verification_state == -1 )
    {
        if( accessorExp->expression->verification_state == 1 )
        {
            auto T = accessorExp->expression->expression_return_type;
            auto retT = T->member_has_getter( accessorExp->name );
            if(retT)
            {
                accessorExp->expression_return_type = retT;
                accessorExp->verification_state = 1;
                changes_were_made = true;
            }
            else
            {
                cout<<"ERROR type \""<< T->definition_name << "\" does not have member \"" << accessorExp->name << "\" " << accessorExp->loc << endl;
                cout<< "      (but I bet your mom does!)" << endl;
                accessorExp->verification_state = 0;
                return;
            }
        }
        else if(accessorExp->expression->verification_state == 0)
        {
            accessorExp->verification_state = 0;
            return;
        }
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



