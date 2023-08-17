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
#include "module_manager.hpp"

using namespace csu;
using namespace std;


/// set symbol table ///
set_symbol_table::set_symbol_table(module_AST_node* module)
{
    //module->module_name = module_name;
    auto TMP = utf8_string(module->module_name);
    module->top_symbol_table.set_name( TMP  ); // odd this needs to be done here?
    symbol_table = module->symbol_table;
    current_node = module;
}

set_symbol_table::set_symbol_table(AST_visitor_base* _parent) :
    AST_visitorTree( _parent )
{
    symbol_table = nullptr;
    current_node = nullptr;
}

shared_ptr< AST_visitor_base > set_symbol_table::make_child(int number)
{
    auto A = make_shared<set_symbol_table>( this );
    return A;
}


void set_symbol_table::ASTnode_down(AST_node* ASTnode)
{
    current_node = ASTnode;
    if(parent)
    {
        set_symbol_table* par = dynamic_cast<set_symbol_table*>( parent );
        current_node->parent_node = par->current_node; // the current nodes parent is the parents current node?  .... I guess that makes sence!

        if( not symbol_table)
        {
            ASTnode->symbol_table = par->symbol_table;
            symbol_table = ASTnode->symbol_table;
        }

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
    // parent table
    set_symbol_table* par = dynamic_cast<set_symbol_table*>( parent );
    class_node->symbol_table = par->symbol_table;

    // new table
    auto table_name = class_node->name+"_namespace_";
    table_name += class_node->symbol_table->get_unique_string();
    class_node->inner_symbol_table = make_shared<local_sym_table>(table_name, class_node->symbol_table);

    symbol_table = class_node->inner_symbol_table.get();

    //self variable
    utf8_string def_name = "self";
    bool is_exclusive = true;
    bool is_ordered = false;
    class_node->self_name = symbol_table->new_variable( nullptr, def_name, class_node->loc, is_exclusive, is_ordered);

    if(not is_exclusive)
    {
        throw gen_exception("weird error A in set_symbol_table::ClassDef_down. This should not be reached");
    }
    else if(not class_node->self_name)
    {
        throw gen_exception("weird error B in set_symbol_table::ClassDef_down. This should not be reached");
    }
}

void set_symbol_table::methodDef_down(method_AST_node* methodDef)
{
    set_symbol_table* par_vstr = dynamic_cast<set_symbol_table*>( parent );
    methodDef->symbol_table = par_vstr->symbol_table;

    // get parent table, nesting is weird here.
    auto class_table = dynamic_cast<local_sym_table*>( par_vstr->symbol_table );
    //methodDef->symbol_table = class_table->parent_table;

    // make and set inner table
    auto table_name = methodDef->name+"_namespace_";
    table_name += methodDef->symbol_table->get_unique_string();
    methodDef->inner_symbol_table = make_shared<local_sym_table>(table_name, methodDef->symbol_table); // note that the methods inner table is a child of the table enclosign the class, NOT the class table
    symbol_table = methodDef->inner_symbol_table.get(); // now the lower symbols, block and parameter list, will have the inner symbol table

    // becouse of the wierd nesting, we have to get the self name, and place it in this symbol table.

    utf8_string selfstr( "self" );
    bool order = false;
    auto self_name = class_table->get_variable_global(selfstr, methodDef->loc, order);

    if( not self_name )
    {
        throw gen_exception("weird error A in  set_symbol_table::methodDef_down. This should not be reached");
    }

    bool exclusive = true;
    symbol_table->add_variable( self_name, exclusive );

    if( not exclusive )
    {
        throw gen_exception("weird error B in  set_symbol_table::methodDef_down. This should not be reached");
    }

}

void set_symbol_table::flowControl_down(flowControl_AST_node* controlNode)
{
    set_symbol_table* par = dynamic_cast<set_symbol_table*>( parent );
    controlNode->symbol_table = par->symbol_table;

    utf8_string table_name( "flowControl_namespace_" );
    table_name += controlNode->symbol_table->get_unique_string();
    controlNode->inner_symbol_table = make_shared<local_sym_table>(table_name, controlNode->symbol_table);
    symbol_table = controlNode->inner_symbol_table.get(); // now the lower symbols will have the inner symbol table
}





/// define_names ///
define_names::define_names(module_manager *_cyth_module_manager)
{
    cyth_module_manager = _cyth_module_manager;
    at_module_level = false;
    is_module = true;
}

define_names::define_names(module_manager *_cyth_module_manager, bool _at_module_level)
{
    cyth_module_manager = _cyth_module_manager;
    at_module_level = _at_module_level;
    is_module = false;
}

shared_ptr< AST_visitor_base > define_names::make_child(int number)
{
    return make_shared<define_names>(cyth_module_manager, is_module );
}

void define_names::cImports_up(import_C_AST_node* ASTnode)
{
    // note: completely set verification_state

    if( ASTnode->verification_state ==0 )
    {
        return; // this could be set by module manager!!
    }

    /// type of the variable
    utf8_string TMP = "UNNAMED_C_TYPE";
    bool check_order = false;
    auto unnamed_C_type = ASTnode->symbol_table->get_type_global(TMP, ASTnode->loc, check_order);
    ASTnode->variable_type = unnamed_C_type;
    if( not unnamed_C_type)
    {
        throw gen_exception("cannot import UNNAMED_C_TYPE. This should never be reached.");
        ASTnode->verification_state = 0;
    }

    /// named variable
    if( ASTnode->verification_state!=0)
    {
        bool check_exclusive = true;
        ASTnode->variable = ASTnode->symbol_table->new_variable( unnamed_C_type, ASTnode->usage_name, ASTnode->loc, check_exclusive, false);
        if( not check_exclusive )
        {
            cout << "cannot import name " << ASTnode->usage_name << " already defined. Imported "<<ASTnode->loc<<endl;
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
        T->set_pointers( unnamed_C_type );
        T->definition_name = ASTnode->usage_name;
        T->C_name = ASTnode->import_name;
        T->loc = ASTnode->loc;
        T->is_ordered = false;

        ASTnode->type = T;
        bool check_exclusive = false;
        ASTnode->symbol_table->add_type( T, check_exclusive );
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

void define_names::CythImports_up(import_cyth_AST_node* ASTnode)
{
// note: completely set verification_state

    if( ASTnode->verification_state ==0 )
    {
        return; // this could be set by module manager!!
    }

    module_AST_ptr module = cyth_module_manager->get_module( ASTnode->imported_module_fname.to_cpp_string() );

    if( not module)
    {
        cout<< "cannot import cyth file " << ASTnode->import_module_string << endl;
        ASTnode->verification_state = 0;
        return;
    }
    ASTnode->import_module_Cheader_fname = module->C_header_fname;

    module_sym_table* imported_sym_table = &module->top_symbol_table;

    auto imported_name = ASTnode->import_name;

    bool check_order = false;
    auto var_symbol = imported_sym_table->get_variable_global( imported_name, ASTnode->loc, check_order );

    if( not var_symbol )
    {
        check_order = false;
        auto type_symbol = imported_sym_table->get_type_global( imported_name, ASTnode->loc, check_order );

        if( not type_symbol )
        {
            cout<<"cannot import name " << imported_name << " at "<< ASTnode->loc << endl;
            ASTnode->verification_state = 0;
            return;
        }
        else
        {
            //auto new_type  = type_symbol->copy(ASTnode->usage_name );


            bool check_exclusive = true;
            ASTnode->symbol_table->add_type(type_symbol, check_exclusive, ASTnode->usage_name);
            if( not check_exclusive)
            {
                ASTnode->verification_state = 0;
                return;
            }
        }
    }
    else
    {
        bool check_exclusive = true;

        ASTnode->symbol_table->add_variable( var_symbol, check_exclusive, ASTnode->usage_name );
       // auto new_var_symbol = ASTnode->symbol_table->new_variable( var_symbol->var_type, ASTnode->usage_name, var_symbol->loc, check_exclusive, false);

        if( not check_exclusive)
        {
            ASTnode->verification_state = 0;
            return;
        }
        //else
        //{
        //    new_var_symbol->C_name = var_symbol->C_name;
        //}
    }

    ASTnode->verification_state = 1;
}


// TODO: FIX THESE
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

void define_names::definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child)
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

void define_names::definitionNassignment_up(definitionNassignment_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child,
                                     AST_visitor_base* exp_child)
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
    class_node->self_name->var_type = self_pointer_type;


    for( method_AST_ptr mthd : class_node->method_definitions )
    {
        mthd->class_type = class_node->type_ptr;
        //mthd->self_name->var_type = self_pointer_type;
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
        check_order = false;
        auto typ = varRefExp->symbol_table->get_type_global(varRefExp->var_name,   varRefExp->loc, check_order);

        if( typ )
        {
            varRefExp->variable_symbol = make_shared<varName>();
            varRefExp->variable_symbol->var_type = make_shared<MetaType>(typ);
            varRefExp->variable_symbol->definition_name = varRefExp->var_name;
            varRefExp->variable_symbol->C_name = varRefExp->var_name;
            varRefExp->variable_symbol->loc = varRefExp->loc;
            varRefExp->variable_symbol->is_ordered = false;
        }
        else
        {
            cout << "name '" << varRefExp->var_name<< "' has not been defined. Referenced " << varRefExp->loc << endl;
            varRefExp->verification_state = 0;
        }
    }
}

void reference_names::inheritanceList_up( inheritanceList_AST_node* inheritanceList_node)
{
    for( auto &type_name : inheritanceList_node->class_IDs )
    {
        bool check_order = true;
        auto type = inheritanceList_node->symbol_table->get_type_global(type_name, inheritanceList_node->loc, check_order);

        if( not check_order )
        {
            inheritanceList_node->verification_state = 0;
            return;
        }
        else if( not type ) // does not exist
        {
            inheritanceList_node->verification_state = 0;
            cout << "name '" << type_name << "' has not been defined. Referenced " << inheritanceList_node->loc << endl;
            return;
        }
        else
        {
            auto class_type = type->as_class();

            if( not class_type )
            {
                inheritanceList_node->verification_state = 0;
                cout << "Cannot inherit from non class of type '" << type_name << "' . Referenced " << inheritanceList_node->loc << endl;
                return;
            }
            else
            {
                inheritanceList_node->types.emplace_back(class_type);
            }
        }
    }

    inheritanceList_node->verification_state = 1; // if we get here, then we all good
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

    /// MAKE PARAMS by-reference ///
    // required
    if( funcDef->paramList->required_list )
    {
        for( auto &req_param : funcDef->paramList->required_list->param_list )
        {
            auto &var_type = req_param.variable_symbol->var_type;
            bool is_ref;
            var_type->is_reference_type(is_ref);

            if( not is_ref)
            {
                var_type = make_shared<raw_C_pointer_reference>( var_type, raw_C_pointer_reference::full_assign_assignment );
            }
        }
    }
    //defaulted
    if( funcDef->paramList->defaulted_list )
    {
        for( auto &req_param : funcDef->paramList->defaulted_list->param_list )
        {
            auto &var_type = req_param.variable_symbol->var_type;
            bool is_ref;
            var_type->is_reference_type(is_ref);

            if( not is_ref)
            {
                var_type = make_shared<raw_C_pointer_reference>( var_type, raw_C_pointer_reference::full_assign_assignment );
            }
        }
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

    if(  not funcDef->return_type and funcDef->num_returns == 0 )
    {
        bool check_order = false;
        utf8_string void_name = "void";
        funcDef->return_type = funcDef->symbol_table->get_type_global(void_name, funcDef->loc, check_order);

        if( not funcDef->return_type)
        {
            throw gen_exception("cannot get void type in register_overloads::funcDef_up" );
        }
        funcDef->notify_return_type(); // I hope this works?
        funcDef->num_verified_returns = -2;// so we can notify precisely once
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


    /// MAKE PARAMS by-reference ///
    // required
    if( methodDef->paramList->required_list )
    {
        for( auto &req_param : methodDef->paramList->required_list->param_list )
        {
            auto &var_type = req_param.variable_symbol->var_type;
            bool is_ref;
            var_type->is_reference_type(is_ref);

            if( not is_ref)
            {
                var_type = make_shared<raw_C_pointer_reference>( var_type, raw_C_pointer_reference::full_assign_assignment );
            }
        }
    }
    //defaulted
    if( methodDef->paramList->defaulted_list )
    {
        for( auto &req_param : methodDef->paramList->defaulted_list->param_list )
        {
            auto &var_type = req_param.variable_symbol->var_type;
            bool is_ref;
            var_type->is_reference_type(is_ref);

            if( not is_ref)
            {
                var_type = make_shared<raw_C_pointer_reference>( var_type, raw_C_pointer_reference::full_assign_assignment );
            }
        }
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
    else // make a new method!!
    {
        utf8_string selfstr("self");
        bool order = false;
        auto self_name = methodDef->inner_symbol_table->get_variable_global(selfstr, methodDef->loc, order);

        if( not self_name )
        {
            throw gen_exception("weird error in register_overloads::methodDef_up. This should not be reached");
        }

        specific_function_type = make_shared<MethodType>(methodDef->name, methodDef->loc, self_name );
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

    if(  not methodDef->return_type and methodDef->num_returns == 0 )
    {
        bool check_order = false;
        utf8_string void_name = "void";
        methodDef->return_type = methodDef->symbol_table->get_type_global(void_name, methodDef->loc, check_order);

        if( not methodDef->return_type)
        {
            throw gen_exception("cannot get void type in register_overloads::methodDef_up" );
        }
        methodDef->notify_return_type(); // I hope this works?
        methodDef->num_verified_returns = -2;// so we can notify precisely once
    }
}

void register_overloads::ClassDef_up( class_AST_node* clss_node, std::list<AST_visitor_base*>& var_def_children,
                     std::list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child )
{
    auto class_type = clss_node->type_ptr;

    if( clss_node->inheritanceList and clss_node->inheritanceList->verification_state != 1)
    {
        clss_node->verification_state = 0;
        return;
    }


    // here we do things with the defined methods

    /// check default constructor ///
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = clss_node->inner_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);

    shared_ptr<MethodType> init_method_type;
    if(init_method_var)
    {
        init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
    }
    else
    {
        init_method_type = make_shared<MethodType>(initSTR, clss_node->loc, clss_node->self_name );
        bool exclusive = false;
        init_method_var = clss_node->inner_symbol_table->new_variable( init_method_type, initSTR, clss_node->loc, exclusive, false );
    }

    // set method type
    init_method_type->type_of_method = MethodType::constructor;

    // find default constructor, make if doesn't exist
    // also check there are no return statements!!
    bool found_default_constuctor = false;
    for( auto &instance : init_method_type->overloads )
    {
        if( instance->parameters->total_size() == 0 )
        {
            found_default_constuctor = true;
            break;
        }

        if( not instance->return_type )// this is only set (to void) if there are no return statments
        {
            cout << "class constructor at " << instance->define_loc << " has a return statement. This is a no-no." << endl;
            clss_node->verification_state = 0;
            return;
        }
    }

    if( not found_default_constuctor)
    {
        func_param_ptr empty_parameter_info = make_shared<func_parameter_info>();
        clss_node->default_constructor_overload = init_method_type->define_overload( clss_node->loc, empty_parameter_info, clss_node->inner_symbol_table.get());

        bool check_order = false;
        utf8_string void_name = "void";
        auto void_type = clss_node->symbol_table->get_type_global(void_name, clss_node->loc, check_order);
        init_method_type->define_overload_return( clss_node->default_constructor_overload, void_type );

        clss_node->write_default_constructor = true;
    }


    /// copy constructors ///
    // first we find all copy constructors, and set them to a C pointer type. I hope this works...
    bool found_selfCopy = false; //did ya? did ya punk? did you find it?
    list<varType_ptr> types_with_copy_constructor; // fill this with ALL types that can be copy-constructed from (implicit and explicit)
    for( auto &instance : init_method_type->overloads )
    {
        auto &parameters = instance->parameters;

        if( parameters->required_parameters.size() == 1 and parameters->optional_parameters.size() == 0 ) // is copy constructor
        {
            auto &param_name = parameters->required_parameters.front();

            bool is_ref;
            auto ref_type = param_name->var_type->is_reference_type( is_ref );
            if( (class_type->is_equivalent( param_name->var_type )) or (is_ref and ref_type and class_type->is_equivalent( ref_type )) ) // self-copy constructor
            {
                found_selfCopy = true;
            }

            //clss_node->type_ptr->copy_constructors.push_back( make_pair(param_name->var_type, instance) );
            types_with_copy_constructor.push_back( param_name->var_type );
            //param_name->var_type = make_shared<raw_C_pointer_reference>( param_name->var_type, raw_C_pointer_reference::no_assignment );
    // TODO: types are already refs. Set to no_assigment (check is c-ref type though)
        }
    }

    if( not found_selfCopy )
    {
        varName_ptr RHS_var = make_shared< varName >(  );
        RHS_var->var_type = make_shared<raw_C_pointer_reference>( class_type,  raw_C_pointer_reference::no_assignment );
        RHS_var->definition_name = "RHS";
        RHS_var->C_name = "__cy__arg_RHS";// hope this isn't an issue
        RHS_var->loc = clss_node->loc;
        RHS_var->is_ordered = false;

        func_param_ptr one_parameter_info = make_shared<func_parameter_info>();
        one_parameter_info->required_parameters.push_back( RHS_var );

        clss_node->default_CopyConstructor_overload = init_method_type->define_overload( clss_node->loc, one_parameter_info, clss_node->inner_symbol_table.get());

        bool check_order = false;
        utf8_string void_name = "void";
        auto void_type = clss_node->symbol_table->get_type_global(void_name, clss_node->loc, check_order);
        init_method_type->define_overload_return( clss_node->default_CopyConstructor_overload, void_type );

        clss_node->write_selfCopy_constructor = true;
        types_with_copy_constructor.push_back( class_type );
    }

    /// all constructors should be non-virtual
    for( auto &init_instance : init_method_type->overloads )
    {
        init_instance->is_virtual = false;
    }




    /// check explicit constructors ////
    utf8_string exInitSTR("__exInit__");
    check_order = false;
    auto exInit_method_var = clss_node->inner_symbol_table->get_variable_local(exInitSTR, tmp_loc, check_order);

    shared_ptr<MethodType> exInit_method_type;
    if(exInit_method_var)
    {
        auto exInit_method_type = dynamic_pointer_cast<MethodType>( exInit_method_var->var_type );
        // set method type
        exInit_method_type->type_of_method = MethodType::explicit_constructor;

        for( auto &instance : exInit_method_type->overloads )
        {
            // check no returns
            if( not instance->return_type )// this is only set (to void) if there are no return statments
            {
                cout << "explicit class constructor at " << instance->define_loc << " has a return statement. This is a no-no." << endl;
                clss_node->verification_state = 0;
                return;
            }

            // convert copy constructors, check no constcutors with 0 params
            auto &parameters = instance->parameters;

            if( parameters->required_parameters.size() == 0 and parameters->optional_parameters.size() == 0 )
            {
                cout << "default explicit class constructor at " << instance->define_loc << " should not exist." << endl;
                clss_node->verification_state = 0;
                return;
            }
            if( parameters->required_parameters.size() == 1 and parameters->optional_parameters.size() == 0 ) // is copy constructor
            {
                auto &param_name = parameters->required_parameters.front();

                if( class_type->is_equivalent( param_name->var_type ) ) // self-copy constructor
                {
                    found_selfCopy = true;
                }

                //types_with_copy_constructor.push_back( param_name->var_type );
                //param_name->var_type = make_shared<raw_C_pointer_reference>( param_name->var_type, raw_C_pointer_reference::no_assignment );
            }
        }

        /// all constructors should be non-virtual
        for( auto &init_instance : exInit_method_type->overloads )
        {
            init_instance->is_virtual = false;
        }
    }





    /// assignment!! ///
    // check  assignment exists
    utf8_string assignSTR("__assign__");
    check_order = false;
    auto assign_method_var = clss_node->inner_symbol_table->get_variable_local(assignSTR, tmp_loc, check_order);

    shared_ptr<MethodType> assign_method_type;
    if(assign_method_var)
    {
        assign_method_type = dynamic_pointer_cast<MethodType>( assign_method_var->var_type );
    }
    else
    {
        assign_method_type = make_shared<MethodType>(assignSTR, clss_node->loc, clss_node->self_name );
        bool exclusive = false;
        assign_method_var = clss_node->inner_symbol_table->new_variable( assign_method_type, assignSTR, clss_node->loc, exclusive, false );
    }
    assign_method_type->type_of_method = MethodType::assignment;

    // check known assignmets!
    list<varType_ptr> types_with_assignment;
    for( auto &instance : assign_method_type->overloads )
    {
        auto &parameters = instance->parameters;

        if( parameters->required_parameters.size() == 1 and parameters->optional_parameters.size() == 0 ) // good!
        {
            // make a pointer!
            auto &param_name = parameters->required_parameters.front();
            types_with_assignment.push_back( param_name->var_type );
            //param_name->var_type = make_shared<raw_C_pointer_reference>( param_name->var_type, raw_C_pointer_reference::no_assignment );
        }
        else
        {
            cout << "assignment at " << instance->define_loc << " for class "<< clss_node->name <<" has other than 1 required argument." << endl;
            clss_node->verification_state = 0;
            return;
        }
    }

    // make needed assigment for each copy constructor
    for( auto &copy_const_type : types_with_copy_constructor )
    {
        bool is_ref;
        auto ref_type = copy_const_type->is_reference_type( is_ref );

        bool found_assignment = false;
        for( auto &assigned_type : types_with_assignment )
        {
            if( copy_const_type->is_equivalent( assigned_type ) or  (is_ref and ref_type and copy_const_type->is_equivalent( ref_type )) )
            {
                found_assignment = true;
                break;
            }
        }

        if( not found_assignment)
        {
            varName_ptr RHS_var = make_shared< varName >(  );

            if( not is_ref )
            {
                RHS_var->var_type = make_shared<raw_C_pointer_reference>( copy_const_type,  raw_C_pointer_reference::no_assignment );
            }
            else
            {
                 RHS_var->var_type = copy_const_type;
            }

            RHS_var->definition_name = "RHS";
            RHS_var->C_name = "__cy__arg_RHS";// hope this isn't an issue
            RHS_var->loc = clss_node->loc;
            RHS_var->is_ordered = false;

            func_param_ptr one_parameter_info = make_shared<func_parameter_info>();
            one_parameter_info->required_parameters.push_back( RHS_var );

            auto new_assign_overload = assign_method_type->define_overload( clss_node->loc, one_parameter_info, clss_node->inner_symbol_table.get() );

            bool check_order = false;
            utf8_string void_name = "void";
            auto void_type = clss_node->symbol_table->get_type_global(void_name, clss_node->loc, check_order);
            assign_method_type->define_overload_return( new_assign_overload, void_type );

            clss_node->assignments_to_default.push_back( make_pair(new_assign_overload, RHS_var) );
        }
    }

    /// assignTo ///
    utf8_string assignToSTR("__assignTo__");
    check_order = false;
    auto assignTo_method_var = clss_node->inner_symbol_table->get_variable_local(assignToSTR, tmp_loc, check_order);

    if(assignTo_method_var)
    {
        shared_ptr<MethodType> assignTo_method_type = dynamic_pointer_cast<MethodType>( assignTo_method_var->var_type );
        assignTo_method_type->type_of_method = MethodType::assignTo;

        for( auto &instance : assignTo_method_type->overloads )
        {
            auto &parameters = instance->parameters;

            if( parameters->required_parameters.size() == 1 and parameters->optional_parameters.size() == 0 )
            {
                //auto &param_name = parameters->required_parameters.front();
                //param_name->var_type = make_shared<raw_C_pointer_reference>( param_name->var_type, raw_C_pointer_reference::full_assign_assignment );
            }
            else // wrong number parameters
            {
                cout << "class assignTo operator at " << instance->define_loc << " has wrong number of parameters" << endl;
                clss_node->verification_state = 0;
                return;
            }
        }
    }




    /// DESTRUCTOR ///
    /// check there is only one parameter-less destructor ///
    utf8_string delSTR("__del__");
    check_order = false;
    auto del_method_var = clss_node->inner_symbol_table->get_variable_local(delSTR, tmp_loc, check_order);


    if(del_method_var)
    {
        shared_ptr<MethodType> del_method_type = dynamic_pointer_cast<MethodType>( del_method_var->var_type );
        del_method_type->type_of_method = MethodType::destructor;

        if( del_method_type->overloads.size() ==1 )
        {
            auto resolved_del_method = del_method_type->overloads.front();
            if( resolved_del_method->parameters->total_size() > 0 )
            {
                cout << "destructor at " << resolved_del_method->define_loc << " for class "<< clss_node->name <<" has arguments. This doesn't make sence." << endl;
                clss_node->verification_state = 0;
                return;
            }
        }
        else
        {
            cout << "class " << clss_node->name << " has more than 1 destructor. This is silly." << endl;
            clss_node->verification_state = 0;
            return;
        }
    }
    else
    {
        auto del_method_type = make_shared<MethodType>(delSTR, clss_node->loc, clss_node->self_name );
        del_method_type->type_of_method = MethodType::destructor;
        bool exclusive = false;
        del_method_var = clss_node->inner_symbol_table->new_variable( del_method_type, delSTR, clss_node->loc, exclusive, false );

        func_param_ptr empty_parameter_info = make_shared<func_parameter_info>();
        clss_node->default_destructor_overload = del_method_type->define_overload( clss_node->loc, empty_parameter_info, clss_node->inner_symbol_table.get());

        bool check_order = false;
        utf8_string void_name = "void";
        auto void_type = clss_node->symbol_table->get_type_global(void_name, clss_node->loc, check_order);
        del_method_type->define_overload_return( clss_node->default_destructor_overload, void_type );

        clss_node->write_default_deconstructor = true;
    }





    /// check __convert__ ///
    utf8_string convertSTR("__convert__");
    check_order = false;
    auto convert_method_var = clss_node->inner_symbol_table->get_variable_local(convertSTR, tmp_loc, check_order);

    if(convert_method_var)
    {
        shared_ptr<MethodType> convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
        convert_method_type->type_of_method = MethodType::conversion;

        for( auto &instance : convert_method_type->overloads )
        {
            auto &parameters = instance->parameters;

            if( parameters->required_parameters.size() == 1 and parameters->optional_parameters.size() == 0 )
            {
                //auto &param_name = parameters->required_parameters.front();
                //param_name->var_type = make_shared<raw_C_pointer_reference>( param_name->var_type, raw_C_pointer_reference::full_assign_assignment );
            }
            else // wrong number parameters
            {
                cout << "implicit class conversion operator at " << instance->define_loc << " has wrong number of parameters" << endl;
                clss_node->verification_state = 0;
                return;
            }
        }
    }

    /// check __exConvert__ ///
    utf8_string exConvertSTR("__exConvert__");
    check_order = false;
    auto exConvert_method_var = clss_node->inner_symbol_table->get_variable_local(exConvertSTR, tmp_loc, check_order);

    if(exConvert_method_var)
    {
        shared_ptr<MethodType> exConvert_method_type = dynamic_pointer_cast<MethodType>( exConvert_method_var->var_type );
        exConvert_method_type->type_of_method = MethodType::explicit_conversion;

        for( auto &instance : exConvert_method_type->overloads )
        {
            auto &parameters = instance->parameters;

            if( parameters->required_parameters.size() == 1 and parameters->optional_parameters.size() == 0 )
            {
                //auto &param_name = parameters->required_parameters.front();
                //param_name->var_type = make_shared<raw_C_pointer_reference>( param_name->var_type, raw_C_pointer_reference::full_assign_assignment );
            }
            else // wrong number parameters
            {
                cout << "explicit class conversion operator at " << instance->define_loc << " has wrong number of parameters" << endl;
                clss_node->verification_state = 0;
                return;
            }
        }
    }



    /// MOVED ///
    /// check there is only one parameter-less __moved__ ///
    utf8_string movedSTR("__moved__");
    check_order = false;
    auto moved_method_var = clss_node->inner_symbol_table->get_variable_local(movedSTR, tmp_loc, check_order);


    if(moved_method_var)
    {
        shared_ptr<MethodType> moved_method_type = dynamic_pointer_cast<MethodType>( moved_method_var->var_type );
        moved_method_type->type_of_method = MethodType::inform_moved;

        if( moved_method_type->overloads.size() ==1 )
        {
            auto resolved_del_method = moved_method_type->overloads.front();
            if( resolved_del_method->parameters->total_size() > 0 )
            {
                cout << "__moved__ at " << resolved_del_method->define_loc << " for class "<< clss_node->name <<" has arguments." << endl;
                clss_node->verification_state = 0;
                return;
            }
        }
        else
        {
            cout << "class " << clss_node->name << " has more than 1 __moved__." << endl;
            clss_node->verification_state = 0;
            return;
        }
    }




/// do simple inheritance things ///
    class_type->top_level_inheritances.reserve( clss_node->inheritanceList->types.size() );

    for( auto& inherited_type : clss_node->inheritanceList->types )
    {
        class_type->top_level_inheritances.emplace_back( inherited_type );
    }
}


/// BUILD CLASSES
shared_ptr< AST_visitor_base > build_classes::make_child(int number)
{
    return make_shared<build_classes>( );
}

// this is a function we need below
// this recurses through each parent generating the list of all inherited classes.
//       the childern list should be initialized empty, used to make sure inheritance has no loops
// this also sets which method points to where on the v-table
// this should be re-examined as things change, as it may not be very efficent...
bool get_inheritance_tree(ClassType_ptr clss_type, list< ClassType_ptr > &childern)
    {
        // make sure we have no loops
        for( auto& c : childern)
        {
            if( clss_type->is_equivalent( c ) )
            {
                cout << "detected inheritance cycle in class " << clss_type->definition_name << " defined at " << clss_type->loc << endl;
                return false;
            }
        }
        childern.emplace_back( clss_type );

        // do stuff if necisary
        if( clss_type->full_inheritance_tree.size() <  clss_type->top_level_inheritances.size() )
        {
            // first, construct inheritance tree
            // inheritance tree is not fully constructed
            for( auto& top_level_parent :  clss_type->top_level_inheritances)// loop over each top-level parent
            {
                // get parent's tree, note: this can construct said tree
                if( not get_inheritance_tree(top_level_parent, childern) )
                {
                    return false;
                }

                vector< ClassType_ptr > &parent_class_tree = top_level_parent->full_inheritance_tree;
                vector< int > &parent_height_tree = top_level_parent->inheritance_tree_level;

                // now merge their tree into yours
                int current_length = clss_type->full_inheritance_tree.size();
                int new_length = current_length + parent_class_tree.size() + 1;

                clss_type->full_inheritance_tree.reserve( new_length );
                clss_type->inheritance_tree_level.reserve( new_length );


                for( int i=0; i<parent_class_tree.size(); i++)
                {
                    clss_type->full_inheritance_tree.emplace_back( parent_class_tree[i] );
                    int index = parent_height_tree[i];
                    if( index == -1 )
                    {
                        clss_type->inheritance_tree_level.emplace_back( new_length );
                    }
                    else
                    {
                        clss_type->inheritance_tree_level.emplace_back( index+current_length );
                    }
                }

                // finally, add the top-level parent
                clss_type->full_inheritance_tree.emplace_back( top_level_parent );
                clss_type->inheritance_tree_level.emplace_back( -1 );



//                // inform methods of their location in the vtable
//                // first, loop over every method
//TODO: I think this is wrong, and needs to be OUTSIDE the parent loop!!!
//and I think the loop should ONLY be over overloads, becouse THATs the real thing
//                auto method_iter = clss_type->methodIter_begin();
//                auto method_iter_end = clss_type->methodIter_end();
//                for( ; method_iter!=method_iter_end; ++method_iter)
//                {
//                    auto method_type = method_iter.get();
//
//                    // certain method types do not "overload"
//                    if( (method_type->type_of_method==MethodType::constructor) or (method_type->type_of_method==MethodType::explicit_constructor) )
//                    {
//                        continue;
//                    }
//
//                    // now we loop over every parent
//                    for(int parent_i = 0; parent_i<clss_type->full_inheritance_tree.size(); ++parent_i)
//                    {
//                        auto parent_type = clss_type->full_inheritance_tree[ parent_i ];
//
//                        // and we see if that parent has a method that has same name as this one (thus could potentially override)
//                        auto parent_method = parent_type->get_method( method_type->definition_name );
//                        if( parent_method )
//                        {
//// we have two methods with same name
//// now we loop over every overload and see if they have same parameters
//for(auto &method_overload : method_type->overloads)
//{
//    auto parent_method_overload = parent_method->get_indestinguishable_overload( method_overload->parameters );
//    if( parent_method_overload )
//    { // we found an overload!!  parent_method_overload needs to be virtual, or we have an error
//        if( parent_method_overload->is_virtual )
//        {
//           //here, we are good. child method overload overrides a parent method overload (clear?)
//            // though, TBH, we only overload top-level methods
//            // note this function was called on the parent previous, so we assume it is set correctly
//            if( parent_method_overload->overriden_method==nullptr and method_overload->overriden_method==nullptr ) // parent_method_overload top level,  and method_overload hasn't been set yet
//            {
//                // we finally find a overriden function
//                method_overload->parental_vtable_location = parent_i;
//                method_overload->overriden_method  = parent_method_overload;
//                // someday later we need to check the return type (this is done in verify_symbol_table::methodDef_up)
//                // only then will we have time to cry
//            }
//        }
//        else
//        {
//            cout << "method overload of " << method_type->definition_name << " at " << method_overload->define_loc << " overrides method defined at " <<
//            parent_method_overload->define_loc << ". But parent method is not virtual. This isn't C++." << endl;
//            return false;
//        }
//    }
//}
//                        }
//                    }
//                }
            }



            // now we inform methods of their location in the vtable
            // first, loop over every method overload
            auto method_iter = clss_type->methodOverloadIter_begin( false );
            auto method_iter_end = clss_type->methodOverloadIter_end();
            for( ; method_iter != method_iter_end; ++method_iter)
            {
                auto method_type = method_iter.method_get();

                // certain method types do not "overload"
                if( (method_type->type_of_method == MethodType::constructor) or (method_type->type_of_method == MethodType::explicit_constructor) )
                {
                    continue;
                }

                auto method_overload = method_iter.overload_get();

                // now we loop over every parent
                for(int parent_i = 0; parent_i<clss_type->full_inheritance_tree.size(); ++parent_i) // maybe only loop over top-level parents, and also build inhereted methods?
                {
                    auto parent_type = clss_type->full_inheritance_tree[ parent_i ];

                    // and we see if that parent has a method that has same name as this one (thus could potentially override)
                    // this loop includes overloads inherited by higher parents, we filter them out below
                    auto parent_method = parent_type->get_method( method_type->definition_name );
                    if( parent_method )
                    {
// we have two methods with same name
    auto parent_method_overload = parent_method->get_indestinguishable_overload( method_overload->parameters );
    if( parent_method_overload and not parent_method->overload_was_inhereted(parent_method_overload) )
    { // we found an overload!!  parent_method_overload needs to be virtual, or we have an error
        if( parent_method_overload->is_virtual )
        {
           //here, we are good. child method overload overrides a parent method overload (clear?)
            // though, TBH, we only overload top-level methods
            // note this function was called on the parent previous, so we assume it is set correctly
            if( parent_method_overload->overriden_method==nullptr and method_overload->overriden_method==nullptr ) // parent_method_overload top level,  and method_overload hasn't been set yet
            {
                // we finally find a overriden function
                method_overload->parental_vtable_location = parent_i;
                method_overload->overriden_method  = parent_method_overload;
                // someday later we need to check the return type (this is done in verify_symbol_table::methodDef_up)
                // only then will we have time to cry
            }
        }
        else
        {
            cout << "method overload of " << method_type->definition_name << " at " << method_overload->define_loc << " overrides method defined at " <<
            parent_method_overload->define_loc << ". But parent method is not virtual. This isn't C++." << endl;
            return false;
        }
    }
                    }
                }
            }


        }

        return true;
}


void build_classes::ClassDef_up( class_AST_node* clss_node, list<AST_visitor_base*>& var_def_children,
                     list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child )
{
    if( clss_node->verification_state == 0 )
    { return; }



    auto class_type = clss_node->type_ptr;
/// do inheritance things ///
// this can get complicated, as there are things that need to be done in order of inheritance, not in definition order

    // first we form the inheritance tree
    // note the order is WIERD. This function may have already been called on this type, or on parent types..
    // also, we will use this opertunity to inform methods their spot in the vtables






    // build and get inheritance tree

    list< ClassType_ptr > all_classes; //NOTE: this is for checking no loops in inheritance. This can be done better, but is needed to avoid infinite do-loops

    if( not get_inheritance_tree(class_type, all_classes) )
    {
        cout << "class " << class_type->definition_name << " not defined at " << clss_node->loc << " due to inheritance cycle." << endl;
        cout << "   contact Christopher Paolini for more information." << endl;
        clss_node->verification_state = 0;
        return;
    }

    // check a class isn't inherited from twice. TODO: roll this into the loop checking above, they are esentually the same thing
    for( int i=0; i<class_type->full_inheritance_tree.size(); i++ )
    {
        auto first_inherited_class = class_type->full_inheritance_tree[i];

        for( int j = i+1; j<class_type->full_inheritance_tree.size(); j++ )
        {
            auto second_inherited_class = class_type->full_inheritance_tree[j];
            if( first_inherited_class->is_equivalent( second_inherited_class ) )
            {
                cout << "class " << class_type->definition_name << " inherits from " << first_inherited_class->definition_name << " twice. Defined at " << clss_node->loc << endl;
                clss_node->verification_state = 0;
                return;
            }
        }
    }



    // set direct parents as data members in the symbol table
    class_type->parent_class_names.reserve( class_type->top_level_inheritances.size() );
    for( auto& parent_class : class_type->top_level_inheritances )
    {
        bool is_exclusive = true;
        auto parent_var = class_type->class_symbol_table->new_variable(parent_class, parent_class->definition_name, class_type->loc, is_exclusive, false);
        if( not is_exclusive)
        {
            cout << "class '" << class_type->definition_name << "' defined at " << class_type->loc << ". Inherits from " << parent_class->definition_name
            << " (defined at " << parent_class->loc << "), but have member with same name."<<endl;
            clss_node->verification_state = 0;
            return;
        }
        class_type->parent_class_names.emplace_back( parent_var );
    }



    // set all vtable names
    class_type->global_parentVtable_cnames.reserve( class_type->full_inheritance_tree.size() );
    for(auto &parent : class_type->full_inheritance_tree)
    {
        utf8_string vtablename( "__cy__globalVtable_");
        vtablename = vtablename + class_type->definition_name +"_" + parent->definition_name+"_"+class_type->class_symbol_table->get_unique_string();
        class_type->global_parentVtable_cnames.emplace_back( vtablename );
    }




    // now we need to build the method connection table. Tell each top-level method who overrides it
    class_type->method_connection_table.resize( class_type->full_inheritance_tree.size() );
    for( int parent_class_index=0; parent_class_index<(class_type->full_inheritance_tree.size());  ++parent_class_index )
    {
        auto parent_class_type = class_type->full_inheritance_tree[ parent_class_index ];

        // now we loop over every method and every overload in this parent
        auto overloadIter = parent_class_type->methodOverloadIter_begin( false );
        auto overloadIter_end = parent_class_type->methodOverloadIter_end();
        for( ;  overloadIter != overloadIter_end; ++overloadIter)
        {
            auto parent_method = overloadIter.method_get();
            auto parent_overload = overloadIter.overload_get();

            // certain method types do not override
            if( (parent_method->type_of_method==MethodType::constructor) or (parent_method->type_of_method==MethodType::explicit_constructor) )
            {
                continue;
            }

            // check is at the top of the inheritance.  Do NOT check if virtual, will do that later (in order to check if non-virtual functions are overloaded)
            if( parent_overload->overriden_method == nullptr )
            {
                MethodType::ResolvedMethod_ptr current_override = nullptr;
                ClassType_ptr overriding_class = nullptr;
                //MethodType_ptr overrideing_method = nullptr;

                // now we loop over every overload of every method, checking for overrides. We go bottom-up in case we can terminate early
                // starting with this class
                auto test_method = class_type->get_method( parent_method->definition_name );
                if( test_method )
                {
                    //has method with same name
                    auto test_overload = test_method->get_indestinguishable_overload( parent_overload->parameters );
                    if( test_overload and not test_method->overload_was_inhereted(test_overload) ) //  have overload with same indestingishable parameters!!
                    {
                        current_override = test_overload;
                        overriding_class = class_type;
                        //overrideing_method = test_method;
                    }
                }
                // now we try the "lower" parent classes, including the current parent class
                //   such that current_override will be set to parent_overload if it is not overriden
                for( int test_parent_index=class_type->full_inheritance_tree.size()-1;  test_parent_index>=parent_class_index and current_override==nullptr; --test_parent_index )
                {
                    auto &test_parent_class = class_type->full_inheritance_tree[ test_parent_index ];

                    // now we check for overriding methods
                    test_method = test_parent_class->get_method( parent_method->definition_name );
                    if( test_method )
                    {
                        //has method with same name
                        auto test_overload = test_method->get_indestinguishable_overload( parent_overload->parameters );
                        if( test_overload and not test_method->overload_was_inhereted(test_overload) ) //  have overload with same indestingishable parameters!!
                        {
                            current_override = test_overload;
                            overriding_class = test_parent_class;
                            //overrideing_method = test_method;
                        }
                    }
                }


                // now we know that parent_overload in class parent_class_type is overriden by overload current_override in class overriding_class

                // check if we are truly overriden, then the parent is virtual or a constructor
                if( current_override!=parent_overload  and  (not parent_overload->is_virtual)  )
                {
                    cout << "method overload of " << parent_method->definition_name << " at " << current_override->define_loc << " overrides method defined at " <<
                    parent_overload->define_loc << ". But parent method is not virtual. This isn't C++." << endl;
                    clss_node->verification_state = 0;
                    return;
                }

                // insert into the table
                method_connection connect{ parent_overload, overriding_class, current_override };
                auto &connection_map = class_type->method_connection_table[ parent_class_index ];
                connection_map.emplace(parent_method->definition_name,  connect);



                // note, EVERY inherited method-overload is visited here
                // therefore, we use this opportunity to insert inherited overloads into the classes symbol table
                if( not overriding_class->is_equivalent( class_type ) ) // check we are actually inheriting
                {
                    //first check already has method
                    cout << parent_method->definition_name << endl;
                    auto methodTYP = class_type->get_method( parent_method->definition_name );

                    //if not, make it
                    if( not methodTYP )
                    {
                        methodTYP = make_shared<MethodType>(parent_method->definition_name, clss_node->loc, clss_node->self_name );
                        methodTYP->type_of_method = parent_method->type_of_method;
                        bool exclusive = true;
                        auto new_method_var = clss_node->inner_symbol_table->new_variable( methodTYP, parent_method->definition_name, clss_node->loc, exclusive, false );

                        if( not exclusive )
                        {
                            cout << "class at "<< clss_node->loc << " inherits a method of name " << parent_method->definition_name << " but this interferes with a non-method variable" << endl;
                            clss_node->verification_state = 0;
                            return;
                        }
                    }

                    // now insert the overload directly. Hope this doesn't cause problems
                    methodTYP->overloads.emplace_back( current_override );
                }



            }
        }
    }
}






/// build the types of everything ///
build_types::build_types()
{
    changes_were_made = false;
    debug = false;
    typed_VarDef_signal = 0;
}

build_types::build_types(bool _debug)
{
    changes_were_made = false;
    debug = _debug;
    typed_VarDef_signal = 0;
}

shared_ptr< AST_visitor_base > build_types::make_child(int number)
{
    return make_shared<build_types>( debug );
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
        if(debug){ cout<< "build_types::callableDef_up changed" <<endl;}
        changes_were_made = true;

        if( not callDef->return_type and callDef->num_verified_returns==0 ) // may be set earlier?
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
                if(debug){ cout<< "build_types::baseArguments_up changed" <<endl;}
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
        if(debug){ cout<< "build_types::baseArguments_up changed" <<endl;}
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

                if(debug){ cout<< "build_types::callArguments_up changed" <<endl;}
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
                if(debug){ cout<< "build_types::callArguments_up changed" <<endl;}
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
        if(debug){ cout<< "build_types::callArguments_up changed" <<endl;}
        changes_were_made = true;
    }
}

void build_types::reqParams_up(function_parameter_list::required_params* reqParams, std::list<AST_visitor_base*>& visitor_children)
{
    if( reqParams->verification_state != -1 )
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
            if(debug){ cout<< "build_types::reqParams_up changed" <<endl;}
            changes_were_made = true;
            return;
        }
    }

    // if we make it here then we are good!
    reqParams->verification_state = 1;
    if(debug){ cout<< "build_types::reqParams_up changed" <<endl;}
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
            if(debug){ cout<< "build_types::defaultParams_up changed" <<endl;}
            changes_were_made = true;
            return;
        }
        if( exp->verification_state == -1 )
        {
            return;// retreat to verify another day
        }

        //auto var_type = (*param_iter).var_type_ASTnode->resolved_type;
        auto var_type = (*param_iter).variable_symbol->var_type;
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

            if(debug){ cout<< "build_types::defaultParams_up changed" <<endl;}
            changes_were_made = true;
            return;
        }

        /// now check compatability ///
        if( not var_type->has_implicit_copy_constructor( exp_type ) )
        {
            cout << "ERROR: type '" << var_type->definition_name << "' can not copy from from type '" << exp_type->definition_name << "' " << (*param_iter).loc << endl;
            defParams->verification_state = 0;
            if(debug){ cout<< "build_types::defaultParams_up changed" <<endl;}
            changes_were_made = true;
            return;
        }
/// TODO, check conversion operator

        ++param_iter;
        ++default_exp_iter;
    }

    // if we make it here then we are good!
    defParams->verification_state = 1;
    if(debug){ cout<< "build_types::defaultParams_up changed" <<endl;}
    changes_were_made = true;

}


void build_types::activeCond_up(activeCond_AST_node* condNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional)
{
// this is wierd. It WANTS to verify, or not, now. But the block isn't verified until later.
// Thus we "semi-verify". Later we check all children are verified

    if( condNode->verification_state != -1 )
    {
        return;
    }


    // check lower nodes
    if( condNode->if_expression->verification_state == 0 )
    {
        condNode->verification_state = 0;
        return;
    }
    else if( condNode->if_expression->verification_state == -1  )
    {
        return;
    }

//    if( condNode->block_AST->verification_state == 0 )
//    {
//        condNode->verification_state = 0;
//        return;
//    }
//    else if( condNode->block_AST->verification_state == -1  )
//    {
//        return;
//    }
//
//    if( condNode->child_conditional )
//    {
//        if( condNode->child_conditional->verification_state == 0 )
//        {
//            condNode->verification_state = 0;
//            return;
//        }
//        else if( condNode->child_conditional->verification_state == -1  )
//        {
//            return;
//        }
//    }


    // all lower nodes verified
    // now we check the if-expression
    varType_ptr if_exp_type = condNode->if_expression->expression_return_type;
    // must be c-type, bool, or implicitly cast to bool

    // is c-type
    if( if_exp_type->type_of_type == varType::c_import_t )
    {
        changes_were_made = true;
        condNode->verification_state = 1;
        return;
    }

    // find bool type
    utf8_string bool_name("bool");
    location_span loc;
    bool check_order = false;
    varType_ptr bool_type = condNode->symbol_table->get_type_global(bool_name, loc, check_order);

    if( not bool_type ) // bool type not defined yet.
    {
        cout << "expression is not c-type, and bool is not defined, at " << condNode->if_expression->loc << endl;
        cout << "   exp is type: " << if_exp_type->definition_name << endl;
        condNode->verification_state = 0;
        return;
    }

    // is bool
    if( if_exp_type->is_equivalent(bool_type) )
    {
        if(debug){ cout<< "build_types::activeCond_up changed A" <<endl;}
        changes_were_made = true;
        condNode->verification_state = 1;
        return;
    }

    // can be bool
    if( bool_type->has_implicit_copy_constructor(if_exp_type, cast_enum::pntr_casts) or if_exp_type->can_implicit_castTo(bool_type) )
    {
        if(debug){ cout<< "build_types::activeCond_up changed B" <<endl;}
        changes_were_made = true;
        condNode->verification_state = 1;
        return;
    }

    // no good!
    cout << "expression is not c-type, is not bool, and cannot be a bool, at " << condNode->if_expression->loc << endl;
    cout << "   exp is type: " << if_exp_type->definition_name << endl;
    condNode->verification_state = 0;

}



void build_types::while_up(whileLoop_AST_node* whileLoopNode, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child)
{

    if( whileLoopNode->verification_state != -1 )
    {
        return;
    }


    // check lower nodes
    if( whileLoopNode->while_expression->verification_state == 0 )
    {
        whileLoopNode->verification_state = 0;
        return;
    }
    else if( whileLoopNode->while_expression->verification_state == -1  )
    {
        return;
    }


    // all lower nodes verified
    // now we check the if-expression
    varType_ptr if_exp_type = whileLoopNode->while_expression->expression_return_type;
    // must be c-type, bool, or implicitly cast to bool

    // is c-type
    if( if_exp_type->type_of_type == varType::c_import_t )
    {
        changes_were_made = true;
        whileLoopNode->verification_state = 1;
        return;
    }

    // find bool type
    utf8_string bool_name("bool");
    location_span loc;
    bool check_order = false;
    varType_ptr bool_type = whileLoopNode->symbol_table->get_type_global(bool_name, loc, check_order);

    if( not bool_type ) // bool type not defined yet.
    {
        cout << "expression is not c-type, and bool is not defined, at " << whileLoopNode->while_expression->loc << endl;
        cout << "   exp is type: " << if_exp_type->definition_name << endl;
        whileLoopNode->verification_state = 0;
        return;
    }

    // is bool
    if( if_exp_type->is_equivalent(bool_type) )
    {
        if(debug){ cout<< "build_types::activeCond_up changed A" <<endl;}
        changes_were_made = true;
        whileLoopNode->verification_state = 1;
        return;
    }

    // can be bool
    if( bool_type->has_implicit_copy_constructor(if_exp_type, cast_enum::pntr_casts) or if_exp_type->can_implicit_castTo(bool_type) )
    {
        if(debug){ cout<< "build_types::activeCond_up changed B" <<endl;}
        changes_were_made = true;
        whileLoopNode->verification_state = 1;
        return;
    }

    // no good!
    cout << "expression is not c-type, is not bool, and cannot be a bool, at " << whileLoopNode->while_expression->loc << endl;
    cout << "   exp is type: " << if_exp_type->definition_name << endl;
    whileLoopNode->verification_state = 0;

}

void build_types::for_up(forLoop_AST_node* forLoopNode, AST_visitor_base* initialStmt_child, AST_visitor_base* updateStmt_child, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child)
{

    if( forLoopNode->verification_state != -1 )
    {
        return;
    }


    // check lower nodes
    if( forLoopNode->while_expression->verification_state == 0 )
    {
        forLoopNode->verification_state = 0;
        return;
    }
    else if( forLoopNode->while_expression->verification_state == -1  )
    {
        return;
    }


    // all lower nodes verified
    // now we check the if-expression
    varType_ptr if_exp_type = forLoopNode->while_expression->expression_return_type;
    // must be c-type, bool, or implicitly cast to bool

    // is c-type
    if( if_exp_type->type_of_type == varType::c_import_t )
    {
        changes_were_made = true;
        forLoopNode->verification_state = 1;
        return;
    }

    // find bool type
    utf8_string bool_name("bool");
    location_span loc;
    bool check_order = false;
    varType_ptr bool_type = forLoopNode->symbol_table->get_type_global(bool_name, loc, check_order);

    if( not bool_type ) // bool type not defined yet.
    {
        cout << "expression is not c-type, and bool is not defined, at " << forLoopNode->while_expression->loc << endl;
        cout << "   exp is type: " << if_exp_type->definition_name << endl;
        forLoopNode->verification_state = 0;
        return;
    }

    // is bool
    if( if_exp_type->is_equivalent(bool_type) )
    {
        if(debug){ cout<< "build_types::activeCond_up changed A" <<endl;}
        changes_were_made = true;
        forLoopNode->verification_state = 1;
        return;
    }

    // can be bool
    if( bool_type->has_implicit_copy_constructor(if_exp_type, cast_enum::pntr_casts) or if_exp_type->can_implicit_castTo(bool_type) )
    {
        if(debug){ cout<< "build_types::activeCond_up changed B" <<endl;}
        changes_were_made = true;
        forLoopNode->verification_state = 1;
        return;
    }

    // no good!
    cout << "expression is not c-type, is not bool, and cannot be a bool, at " << forLoopNode->while_expression->loc << endl;
    cout << "   exp is type: " << if_exp_type->definition_name << endl;
    forLoopNode->verification_state = 0;

}





void build_types::typed_VarDef_up(Typed_VarDefinition* var_def, AST_visitor_base* var_type)
{
    // potentially verify

    if( var_def->verification_state != -1 )
    {
        return;
    }

    if( var_def->var_type->verification_state == 0 ) // this completely verified in an earlier visitor
    {
        var_def->verification_state = 0;
        return;
    }
    else if( var_def->var_type->verification_state == 1 )
    {
        if( not var_def->variable_symbol->var_type )
        {
            if(not var_def->var_type->resolved_type)
            {
                // this should never happen
                throw gen_exception( "ERROR in build_types::typed_VarDef_up. This should never be reached" );
            }

            auto type = var_def->var_type->resolved_type;


            if( not type->type_is_fully_defined() )
            {
                return;
            }
            else if( not type->can_be_defined() )
            {
                if(debug){ cout<< "build_types::typed_VarDef_up changed" <<endl;}
                changes_were_made = true;
                var_def->verification_state = 0;
                return;
            }
            else
            {
                var_def->variable_symbol->var_type = type;

                if( typed_VarDef_signal==0 ) // allowed to verify
                {
                    if(debug){ cout<< "build_types::typed_VarDef_up changed" <<endl;}
                    changes_were_made = true;
                    var_def->verification_state = 1;
                }
            }
        }
    }
}

void build_types::definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child)
{
    // note this is called BEFORE typed_VarDef_up, but requires its results. Thus increases loop of visitor. Thus is inefficent, but is probably fine.
    typed_VarDef_signal = 1; // so that typed_VarDef_up will not verify and keep this from running.

    if( defStmt->verification_state == 0 or defStmt->argument_list->verification_state==0 )
    {
        defStmt->verification_state = 0;
        return;
    }
    else if( defStmt->verification_state==-1 and defStmt->variable_symbol->var_type and defStmt->argument_list->verification_state==1 ) // check variable and type is all defined
    {
        auto type = defStmt->variable_symbol->var_type ;

        utf8_string errmsg;
        bool has_constructor = type->has_explicit_constructor( defStmt->argument_list->get_argument_types(), errmsg );

        if( not has_constructor)
        {
            cout << errmsg << endl;
            cout << "ERROR: cannot resolve constructor. " << defStmt->loc << endl;
            defStmt->verification_state = 0;
            return;
        }

        defStmt->verification_state = 1;
        if(debug){ cout<< "build_types::definitionNconstruction_up changed" <<endl;}
        changes_were_made = true;
    }
}

void build_types::definitionNassignment_up(definitionNassignment_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* exp_child)
{
    // note this is called BEFORE typed_VarDef_up, but requires its results. Thus increases loop of visitor. Thus is inefficent, but is probably fine.
    typed_VarDef_signal = 1; // so that typed_VarDef_up will not verify and keep this from running.

    if( defStmt->verification_state == 0 or defStmt->expression->verification_state==0 )
    {
        defStmt->verification_state = 0;
        return;
    }
    else if( defStmt->verification_state==-1 and defStmt->variable_symbol->var_type and defStmt->expression->verification_state==1 ) // check variable and type is all defined
    {
        auto type = defStmt->variable_symbol->var_type ;

        if( not type->type_is_fully_defined() ) // this shouldn't be needed. But shouldn't hurt, and may protect if something goes wrong.
        {
            return; // try anouther day
        }


        utf8_string errmsg;
        if( not type->has_explicit_copy_constructor(defStmt->expression->expression_return_type, errmsg) )
        {
            if( not defStmt->expression->expression_return_type->can_explicit_castTo( type ) )
            {
                cout << errmsg << endl;
                cout << "ERROR: cannot resolve constructor. " << defStmt->loc << endl;
                cout << " and cannot cast type " << defStmt->expression->expression_return_type->definition_name << " to " << type->definition_name << endl;
                defStmt->verification_state = 0;
                return;
            }
        }

        defStmt->verification_state = 1;
        if(debug){ cout<< "build_types::definitionNassignment_up changed" <<endl;}
        changes_were_made = true;
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
            if( not varref->reference_type->get_has_assignment( RHS_type ) )
            {
                //if( not RHS_type->can_implicit_castTo( varref->reference_type.get() ) )
                if( not RHS_type->get_has_assignTo( varref->reference_type ) )
                {
                    cout << "ERROR: type '" << varref->reference_type->definition_name << "' can not assign from type '" << RHS_type->definition_name << "'" << endl;
                    varref->verification_state = 0;
                }
            }
        }

        void LHS_accessor_down(LHS_accessor_AST_node* LHSaccess)
        {
            auto T = LHSaccess->LHS_exp->reference_type->set_member( LHSaccess->name, RHS_type );

            if( not T )
            {
                cout << "ERROR: item '" << LHSaccess->name << "' can not be set from type '" << RHS_type->definition_name << "'" << endl;
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
            if(debug){ cout << "build_types::assignmentStmt_up changed" << endl;}
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
            if(debug){ cout<< "build_types::autoDefStmt_up changed" <<endl;}
            changes_were_made = true;
            return;
        }

        autoStmt->variable_symbol->var_type = autotype;
        //autoStmt->verification_state = 1;
        if(debug){ cout<< "build_types::autoDefStmt_up changed" <<endl;}
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
        if(debug){ cout<< "build_types::autoDefStmt_up changed" <<endl;}
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

        if( not returnStmt->expression->expression_return_type->can_be_defined() )
        {
            cout << "cannot return undefinable type (" << returnStmt->expression->expression_return_type->definition_name << ") at "<< returnStmt->loc << endl;
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
                if(debug){ cout<< "build_types::returnStatement_up changed" <<endl;}
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

                if( ret_T->is_equivalent( exp_T ) )
                {
                    returnStmt->verification_state = 1;
                    returnStmt->callable_to_escape->num_verified_returns += 1;
                    if(debug){ cout<< "build_types::returnStatement_up changed" <<endl;}
                    changes_were_made = true;
                    return;
                }
                else
                {
                    cout<< "return types must be equivalent " << returnStmt->loc << endl;
                    returnStmt->verification_state = 0;
                    returnStmt->callable_to_escape->num_verified_returns = -1;
                    if(debug){ cout<< "build_types::returnStatement_up changed" <<endl;}
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

            if( ret_T->has_implicit_copy_constructor( exp_T ) or exp_T->can_implicit_castTo( ret_T ) )
            {
                returnStmt->verification_state = 1;
                returnStmt->callable_to_escape->num_verified_returns += 1;
                if(debug){ cout<< "build_types::returnStatement_up changed" <<endl;}
                changes_were_made = true;
                return;
            }
            else
            {
                cout<< "cannot convert type " << (exp_T->definition_name) << " to " << (ret_T->definition_name) <<
                        " in return " << returnStmt->loc << endl;
                returnStmt->verification_state = 0;
                returnStmt->callable_to_escape->num_verified_returns = -1;
                if(debug){ cout<< "build_types::returnStatement_up changed" <<endl;}
                changes_were_made = true;
                return;
            }
        }
    }
}

void build_types::loopCntrlStatement_up(loopCntrl_statement_AST_node* cntrlStmt)
{

// first we make a list of affected loops.
    class loopFinder : public AST_visitor_NoChildren
    // this is to start at this node, and travel UP the AST looking for loops controlled by this node
    {
    public:
        list< loop_AST_node* > loops;
        int max_num_loops;

        loopFinder(int _max_num_loops)
        {
            max_num_loops = _max_num_loops;
        }

        void loop_down(loop_AST_node* loopNode) override
        {
            if( loops.size() < max_num_loops )
            {
                loops.push_back(loopNode);
            }
        }
    };


    if( cntrlStmt->verification_state != -1 )
    {
        return;
    }

    loopFinder fndr( (cntrlStmt->depth+1) );
    apply_visitor_upwards(cntrlStmt,  &fndr);

    if( fndr.loops.size() < (cntrlStmt->depth+1) )
    {
        cntrlStmt->verification_state = 0;
        cout << "cntrl statment at " << cntrlStmt->loc << " breaks out of more loops than is embedded in." << endl;
        if(debug){ cout<< "build_types::loopCntrlStatement_up changed" <<endl;}
        changes_were_made = true;
        return;
    }

// now we build the list of loops
    // start with the top loop
    auto list_iter = fndr.loops.begin(); // garrunteed to not be end

    loop_AST_node* current_loop = *list_iter;
    current_loop->control_statments.emplace_back();
    loop_AST_node::loop_list_node* current_node = &(current_loop->control_statments.back());
    current_node->cntrl_node = cntrlStmt;
    current_node->current_loop = current_loop;
    current_node->next = nullptr;

    cntrlStmt->top_loop = current_node;

    auto list_end = fndr.loops.end();
    ++list_iter;
    loop_AST_node::loop_list_node* previous_node = current_node;
    for(  ; list_iter != list_end; ++list_iter)
    {
        current_loop = *list_iter;
        current_loop->control_statments.emplace_back();

        current_node = &(current_loop->control_statments.back());
        current_node->cntrl_node = cntrlStmt;
        current_node->current_loop = current_loop;
        current_node->next = nullptr;

        previous_node->next = current_node;
    }

// now finishing actions
    cntrlStmt->verification_state = 1;
//var_control_name needs to be set in writeAST_to_C.
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

            if(debug){ cout << "build_types::LHS_varRef_up changed" << endl;}
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
                if(debug){ cout<< "build_types::LHS_accessor_up changed" <<endl;}
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
        // first try to get cyth int... hope this is right!
        utf8_string TMP = "int";
        bool check_order = false;
        intLitExp->expression_return_type = intLitExp->symbol_table->get_type_global(TMP, intLitExp->loc, check_order);

        if( (not intLitExp->expression_return_type) or (intLitExp->expression_return_type->loc.start.file_name==intLitExp->loc.start.file_name) )
        {  // int literal not defined. try unnamed C type

            TMP = "UNNAMED_C_TYPE";
            bool check_order = false;
            intLitExp->expression_return_type = intLitExp->symbol_table->get_type_global(TMP, intLitExp->loc, check_order);

            if( not intLitExp->expression_return_type )
            {
                throw gen_exception( "cannot get type int or UNNAMED_C_TYPE in build_types::intLiteral_up. This should never be reached" );
            }

        }

        intLitExp->verification_state = 1;
        if(debug){ cout << "build_types::intLiteral_up changed" << endl;}
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


        binOprExp->mode_of_operation = binOperator_expression_AST_node::LHS_m;// assume, change if wrong
        varType_ptr return_type;
        switch( binOprExp->type_of_operation )
        {
        case binOperator_expression_AST_node::empty:
            cout << "ERROR in build_types::binOperator_up. This should NOT be reached!" << endl;
            cout << "    You did done messed up." << endl;
        break;
    /// numerical operators ///
        // **
        case binOperator_expression_AST_node::power_t:
            return_type = LHS_type->get_has_LHSpower( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSpower( LHS_type );
            }
        break;
        // *
        case binOperator_expression_AST_node::multiplication_t:
            return_type = LHS_type->get_has_LHSmultiplication( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSmultiplication( LHS_type );
            }
        break;
        // /
        case binOperator_expression_AST_node::division_t:
            return_type = LHS_type->get_has_LHSdivision( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSdivision( LHS_type );
            }
        break;
        // %
        case binOperator_expression_AST_node::modulus_t:
            return_type = LHS_type->get_has_LHSmodulus( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSmodulus( LHS_type );
            }
        break;
        // +
        case binOperator_expression_AST_node::addition_t:
            return_type = LHS_type->get_has_LHSaddition( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSaddition( LHS_type );
            }
        break;
        // -
        case binOperator_expression_AST_node::subtraction_t:
            return_type = LHS_type->get_has_LHSsubtraction( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSsubtraction( LHS_type );
            }
        break;
    /// comparison operators///
        // <
        case binOperator_expression_AST_node::lessThan_t:
            return_type = LHS_type->get_has_LHSlessThan( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSlessThan( LHS_type );
            }

        break;
        // >
        case binOperator_expression_AST_node::greatThan_t:
            return_type = LHS_type->get_has_LHSgreatThan( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSgreatThan( LHS_type );
            }
        break;
        // ==
        case binOperator_expression_AST_node::equalTo_t:
            return_type = LHS_type->get_has_LHSequalTo( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSequalTo( LHS_type );
            }
        break;
        // !=
        case binOperator_expression_AST_node::notEqual_t:
            return_type = LHS_type->get_has_LHSnotEqual( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSnotEqual( LHS_type );
            }
        break;
        // <=
        case binOperator_expression_AST_node::lessEqual_t:
            return_type = LHS_type->get_has_LHSlessEqual( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSlessEqual( LHS_type );
            }
        break;
        // >=
        case binOperator_expression_AST_node::greatEqual_t:
            return_type = LHS_type->get_has_LHSgreatEqual( RHS_type );
            if( not return_type )
            {
                binOprExp->mode_of_operation = binOperator_expression_AST_node::RHS_m;
                return_type = RHS_type->get_has_RHSgreatEqual( LHS_type );
            }
        break;


        default:
             throw gen_exception("bad operation in build_types::binOperator_up this should not be reached");

        }

        if(return_type )
        {
            binOprExp->expression_return_type = return_type;
            if(debug){ cout<< "build_types::binOperator_up changed" << endl;}
            changes_were_made = true;

            binOprExp->verification_state = 1;
        }
        else
        {
            binOprExp->verification_state = 0;
            cout << LHS_type->definition_name << " and " << RHS_type->definition_name << " do not have " << binOprExp->operator_name() << " at " << binOprExp->loc << endl;
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

        if(debug){ cout<< "build_types::varReferance_up changed" <<endl;}
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
        if(debug){ cout<< "build_types::ParenExpGrouping_up changed" <<endl;}
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

        utf8_string errmsg;
        auto returnType = funcCall->expression->expression_return_type->is_callable( funcCall->argument_list->get_argument_types(), errmsg );

        if( not returnType )
        {
            cout << errmsg << endl;
            cout << "type " << funcCall->expression->expression_return_type->definition_name << " cannot be called. Referenced "  << funcCall->loc << endl;
            funcCall->verification_state = 0;
            return;
        }

//        if( not funcCall->function_to_write )
//        {
//            cout<< "ERROR: cannot resolved call. " << funcCall->loc << endl;
//            funcCall->verification_state = 0;
//            return;
//        }
//
//        auto return_type = funcCall->function_to_write->return_type;
//        if( not return_type )
//        {
//            throw gen_exception("Strange error in build_types::functionCall_Exp_up. This should not be reached.");
//        }

        funcCall->expression_return_type = returnType;

        funcCall->verification_state = 1;
        if(debug){ cout << "build_types::functionCall_Exp_up changed" << endl;}
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
            if(debug){ cout<< "build_types::accessorExp_up changed" <<endl;}
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


void build_types::constructElement_up(constructElement_AST_node* constructBlock, AST_visitor_base* exp_child, AST_visitor_base* argList_child)
{

    if( (constructBlock->verification_state == -1) and constructBlock->expression->expression_return_type and (constructBlock->argument_list->verification_state==1) )
    {
        if( not constructBlock->expression->expression_return_type->type_is_fully_defined() )
        {
            return;
        }

        utf8_string errmsg;
        bool has_constructor = constructBlock->expression->expression_return_type->has_explicit_constructor( constructBlock->argument_list->get_argument_types(), errmsg );

        if( not has_constructor )
        {
            cout << errmsg << endl;
            cout << "ERROR: resolved constructor. " << constructBlock->loc << endl;
            constructBlock->verification_state = 0;
            return;
        }

        constructBlock->verification_state = 1;
        if(debug){ cout << "build_types::constructElement_up changed" <<endl;}
        changes_were_made = true;
    }
}






//// verify symbol table ////

verify_symbol_table::verify_symbol_table()
{
    do_children = true;
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
    //if( ASTnode->verification_state != -1 )
    //{
    //    verification_state = ASTnode->verification_state;
    //}
    //else
    if( ASTnode->verification_state == 0 )
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

void verify_symbol_table::methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child)
{
    if( methodDef->verification_state == 0 )
    {
        return;
    }

    auto overload = methodDef->specific_overload;
    auto return_type = overload->return_type;

    if( overload->overriden_method )
    {
        auto &overriden_method = overload->overriden_method;
        auto &overriden_overload_return = overriden_method->return_type;

        if(not overriden_overload_return->is_equivalent( return_type ) )
        {
            cout << "ERROR: method '" << methodDef->name << "' overload at " << overload->define_loc << " has return type " << return_type->definition_name
            << ". But overloads parental override at " << overriden_method->define_loc << ", which has return type " << overriden_overload_return->definition_name
            << endl;
            methodDef->verification_state = 0;
            verification_state = 0;
            return;
        }
    }
}

void verify_symbol_table::ClassDef_up( class_AST_node* clss_node, list<AST_visitor_base*>& var_def_children,
                     list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child )
{
    if( clss_node->verification_state == 0 )
    {
        return;
    }

    // check that every method that overrides has same return type!
    for( auto& connetion_map : clss_node->type_ptr->method_connection_table )
    {
        for(auto &connection_pair : connetion_map)
        {
            auto &parent_method = connection_pair.second.methodOverload_to_override;
            auto &overriding_method = connection_pair.second.methodOverload_that_overrides;

            if(overriding_method and (not parent_method->return_type->is_equivalent( overriding_method->return_type ) ))
            {
                cout << "ERROR: method at " << overriding_method->define_loc << " has return type " << overriding_method->return_type->definition_name
                << ". But overloads parental override at " << parent_method->define_loc << ", which has return type " << parent_method->return_type->definition_name
                << endl;
                clss_node->verification_state = 0;
                verification_state = 0;
                return;
            }
        }
    }
}



