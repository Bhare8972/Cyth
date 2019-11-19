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

This file defines the Cyth symbol table
*/

#include <algorithm>
#include "sym_table.hpp"
#include "cyth_AST.hpp"

using namespace csu;
using namespace std;

////  exceptions ////

// name invoked before defined
BeforeDefinition_exc::BeforeDefinition_exc( csu::utf8_string& _name, csu::location_span& _def_loc, csu::location_span& _invoke_loc)
{
    name = _name;
    def_loc = _def_loc;
    invoke_loc = _invoke_loc;

    stringstream ss;
    ss << "name '" << name << "' invoked before definition. Defined at: " << def_loc << ". Invoked at: " << invoke_loc << endl;
    msg = ss.str();
}

const char* BeforeDefinition_exc::what()
{
    return msg.c_str();
}

//// name not defined
//NotDefined_exc::NotDefined_exc( csu::utf8_string& _name, csu::location_span& _invoke_loc)
//{
//    name = _name;
//    invoke_loc = _invoke_loc;
//
//    stringstream ss;
//    ss << "name '" << name << "' invoked without definition. Invoked at: " << invoke_loc << endl;
//    msg = ss.str();
//}
//
//const char* NotDefined_exc::what()
//{
//    return msg.c_str();
//}



/////// types /////////

// varType (BASE)

/// CALLING ///
bool varType::is_callable()
{
    return false;
}

ResolvedFunction_ptr varType::get_nonResolved_call(location_span call_loc, sym_table_base* calling_scope)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " IS NOT CALLABLE ", call_loc);
    return nullptr;
}
ResolvedFunction_ptr varType::get_resolved_call(location_span call_loc, sym_table_base* calling_scope)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " IS NOT CALLABLE ", call_loc);
    return nullptr;
}

/// ADDITION ///
bool varType::get_has_LHSaddition(varType* RHS_type)
{
    return false;
}

utf8_string varType::get_LHSaddition_name(varType* RHS_type)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have addition ");
    return "";
}

shared_ptr<varType> varType::get_LHSaddition_type(varType* RHS_type)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have addition ");
    return nullptr;
}

/// ASSIGNMENT ///
bool varType::get_has_assignment(varType* RHS_type)
{
    return false;
}

utf8_string varType::get_assignment_name(varType* RHS_type)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have assignment ");
    return "";
}

// C import

/// ADDITION ///
bool varType_fromC::get_has_LHSaddition(varType* RHS_type)
{
//    auto it = find(RHStypes_for_LHSaddition.begin(), RHStypes_for_LHSaddition.end(), RHS_type);
//    if( it == RHStypes_for_LHSaddition.end() )
//    {
//        RHStypes_for_LHSaddition.push_back( RHS_type );
//    }
    return true;
}

csu::utf8_string varType_fromC::get_LHSaddition_name(varType* RHS_type)
{
    return "__Ctype_add_macro__";
}

varType_ptr varType_fromC::get_LHSaddition_type(varType* RHS_type)
{
    return unnamed_C_type;
}

/// ASSIGNMENT ///
bool varType_fromC::get_has_assignment(varType* RHS_type)
{
//    auto it = find(types_for_assignment.begin(), types_for_assignment.end(), RHS_type);
//    if( it == types_for_assignment.end() )
//    {
//        types_for_assignment.push_back( RHS_type );
//    }
    return true;
}

csu::utf8_string varType_fromC::get_assignment_name(varType* RHS_type)
{
//    auto it = find(types_for_assignment.begin(), types_for_assignment.end(), RHS_type);
//    if( it == types_for_assignment.end() )
//    {
//        types_for_assignment.push_back( RHS_type );
//    }
//    return (C_name+"__assignment__") + RHS_type->C_name;
    return "__Ctype_assignment_macro__";
}

// resolved function
ResolvedFunction::ResolvedFunction(location_span _loc, sym_table_base* _scope)
{
    definition_name = "RESOLVED_FUNCTION";
    C_name = "__THIS_IS_ERROR_";
    is_ordered = true;
    loc = _loc;
    symbol_table_scope = _scope;
    self_weak_ptr = shared_ptr<ResolvedFunction>();
}

void ResolvedFunction::set_selfPTR(weak_ptr<ResolvedFunction> _self)
{
    self_weak_ptr = _self;
}

void ResolvedFunction::set_unordered()
{
    is_ordered = false;
}

 /// CALLING ///
bool ResolvedFunction::is_callable()
{
    return true; // at least I HOPE so!!
}

ResolvedFunction_ptr ResolvedFunction::get_nonResolved_call(location_span call_loc, sym_table_base* calling_scope)
{
    // assume all things have been checked.
    return self_weak_ptr.lock();
}

ResolvedFunction_ptr ResolvedFunction::get_resolved_call(location_span call_loc, sym_table_base* calling_scope)
{
    // assume all things have been checked.
    return self_weak_ptr.lock();
}

// defined function type
DefFuncType::DefFuncType(location_span _loc)
{
    loc = _loc; // what? this doesn't HAVE a location??
    definition_name = "DEFINED_FUNCTION";
    C_name = "__THIS_IS_ERROR_";
    is_ordered = false;
}

ResolvedFunction_ptr DefFuncType::define_overload(location_span defining_loc, sym_table_base* defining_scope)
{
    // first check that new overload does not conflict with current overloads
    for(auto OL : overloads)
    {
        // first check if distinguish by signiture

        // cannot distinguish, see if can override by scope
        if( defining_scope->is_sameScope( OL->symbol_table_scope ) )
        {
            // cannot distinguish between overloads.
            return nullptr; // need to somehow send more detailed info for error reporting
        }
    }

    ResolvedFunction_ptr new_overload = make_shared<ResolvedFunction>( defining_loc, defining_scope );
    new_overload->set_selfPTR( new_overload );
    overloads.push_back( new_overload );
    return new_overload;
}


 /// CALLING ///
bool DefFuncType::is_callable()
{
    return true;
}

ResolvedFunction_ptr DefFuncType::get_nonResolved_call(csu::location_span call_loc, sym_table_base* calling_scope)
{
    if( overloads.size() == 1 )
    {
        return get_resolved_call(loc, calling_scope);
    }
    else
    {
        return nullptr; // cannot resolve at runtime yet
    }
}

ResolvedFunction_ptr DefFuncType::get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope)
{
    ResolvedFunction_ptr current_overload = nullptr;

    for(auto OL : overloads)
    {
        // first check this overload is reasonable
        if( not OL->symbol_table_scope->is_or_inScope( calling_scope ) )
        {
            continue;
        }

        if( OL->is_ordered and not call_loc.strictly_GT( OL->loc ) )
        {
            continue;
        }

        // need to check signiture works

        if( not current_overload )
        {
            current_overload = OL;
        }
        else
        // OOPS! these two conflict, need to find which one is better
        {
            // check signiture here

            // signitures are the same, so now we check which one has the most inner scope
            if( current_overload->symbol_table_scope->is_or_inScope( OL->symbol_table_scope ) )
            {
                // OL is the most inner (note they cannot be the same scope)
                current_overload = OL;
            }
        }
    }

    return current_overload;
}

////// symbol tables //////

// base symbol table
sym_table_base::sym_table_base()
{
    next_variable_ID = 0;
}

varName_ptr sym_table_base::new_variable(varType_ptr var_type, csu::utf8_string& definition_name, csu::location_span& _loc, bool is_ordered)
{
    auto variable = make_shared<varName>();
    variable->definition_name = definition_name;
    variable->C_name = namespace_unique_name + "__";
    variable->C_name += definition_name;
    variable->loc = _loc;
    variable->is_ordered = is_ordered;
    variable->var_type = var_type;

    variable_table[definition_name] = variable;
    return variable;
}

utf8_string sym_table_base::get_unique_variable_name()
{
    utf8_string out_string = namespace_unique_name + "__";
    out_string += to_string(next_variable_ID);
    next_variable_ID += 1;
    return out_string;
}

void sym_table_base::add_type(varType_ptr new_type)
{
    type_table[new_type->definition_name] = new_type;
}

varName_ptr sym_table_base::get_variable_local(csu::utf8_string& name, csu::location_span& ref_loc)
{
    auto table_itterator = variable_table.find( name );
    if(table_itterator != variable_table.end())
    {

        varName_ptr variable = table_itterator->second;
        if( variable->is_ordered and not ref_loc.strictly_GT( variable->loc ) )
        {
            throw BeforeDefinition_exc(name, variable->loc, ref_loc);
        }

        return variable;
    }

    return nullptr; //cannot find it
}

varType_ptr sym_table_base::get_type_local(csu::utf8_string& name, csu::location_span& ref_loc)
{
    auto table_itterator = type_table.find( name );
    if(table_itterator != type_table.end())
    {
        varType_ptr type = table_itterator->second;
        if( type->is_ordered and not ref_loc.strictly_GT( type->loc ) )
        {
            throw BeforeDefinition_exc(name, type->loc, ref_loc);
        }

        return type;
    }

    return nullptr; //cannot find it
}




// local symbol table
local_sym_table::local_sym_table(csu::utf8_string& _name, sym_table_ptr parent)
{
    name = _name;
    namespace_unique_name = parent->namespace_unique_name + "__" + name;
    parent_table = parent.get();
}

varName_ptr local_sym_table::get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc)
{
    varName_ptr ret = get_variable_local( name, ref_loc );

    if(not ret)
        ret = parent_table->get_variable_global(name, ref_loc);

    return ret;
}

varType_ptr local_sym_table::get_type_global(csu::utf8_string& name, csu::location_span& ref_loc)
{
    varType_ptr ret = get_type_local( name, ref_loc );

    if(not ret)
        ret = parent_table->get_type_global(name, ref_loc);

    return ret;
}

bool local_sym_table::is_or_inScope( sym_table_base* other_scope )
{
    sym_table_base* this_scope = dynamic_cast< sym_table_base* >(this);
    if( this_scope == other_scope )
    {
        return true;
    }
    else
    {
        return parent_table->is_or_inScope( other_scope );
    }
}


// module level symbol table
module_sym_table::module_sym_table()//(csu::utf8_string& _name)
{
    //name = _name;
    //namespace_unique_name = "__cy__" + name;

    // defined un-named C type
    auto variable_type = make_shared<varType_fromC>();
    variable_type->definition_name = "UNNAMED_C_TYPE";
    variable_type->C_name = "__THIS_IS_ERROR_";
    variable_type->is_ordered = false;
    variable_type->unnamed_C_type = variable_type; // YAY MEMORY LEAK!

    add_type(variable_type);
    /*
    // define int
    auto int_def_AST = make_shared<AST_node>();
    built_in_info.push_back( int_def_AST );
    auto int_def_symbol = make_shared<varType_fromC>();
    int_def_symbol->C_name = "long";
    int_def_symbol->definition_AST_node = int_def_AST.get();
    int_def_symbol->is_ordered = false;
    int_def_symbol->name = "int";

    add_type( int_def_symbol );*/
}

void module_sym_table::set_name(csu::utf8_string& _name)
{
    name = _name;
    namespace_unique_name = "__cy__" + name;
}

varName_ptr module_sym_table::get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc)
{
    varName_ptr ret = get_variable_local( name, ref_loc );

    // check imports here?

    return ret;
}

varType_ptr module_sym_table::get_type_global(csu::utf8_string& name, csu::location_span& ref_loc)
{
    varType_ptr ret = get_type_local( name, ref_loc );

    // check imports here?

    return ret;
}

bool module_sym_table::is_or_inScope( sym_table_base* other_scope )
{
    sym_table_base* this_scope = dynamic_cast< sym_table_base* >(this);
    if( this_scope == other_scope )
    {
        return true;
    }
    else
    {
        return false;
    }
}



