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

/////// types /////////

// varType (BASE)

/// variable definition ///
bool varType::can_be_defined() // Returns false if Ctype is unknown
{
    return not (C_name == "");
}

// define new variable of this type.
void varType::C_definition_name(csu::utf8_string& var_name, ostream& output)
{
    if( can_be_defined() )
    {
        output << C_name << " " << var_name ;
    }
}

/// CALLING ///
bool varType::is_callable()
{
    return false;
}


funcCallWriter_ptr varType::get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope)
{
    throw gen_exception("ERROR: TYPE IS NOT CALLABLE ", call_loc);
    return nullptr;
}

/*
ResolvedFunction_ptr varType::get_nonResolved_call(location_span call_loc, sym_table_base* calling_scope)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " IS NOT CALLABLE ", call_loc);
    return nullptr;
}
ResolvedFunction_ptr varType::get_resolved_call(location_span call_loc, sym_table_base* calling_scope)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " IS NOT CALLABLE ", call_loc);
    return nullptr;
}*/



/// ADDITION ///
bool varType::get_has_LHSaddition(varType* RHS_type)
{
    return false;
}

void varType::write_LHSaddition(utf8_string& LHS_exp, utf8_string& RHS_exp, ostream& output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have addition ");
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

void varType::write_assignment( utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have assignment ");
}




// C import

varType_fromC::varType_fromC()
{
    type_of_type = varType::c_import_t;
}

/// ADDITION ///
bool varType_fromC::get_has_LHSaddition(varType* RHS_type)
{
    return true;
}

varType_ptr varType_fromC::get_LHSaddition_type(varType* RHS_type)
{
    return unnamed_C_type.lock();
}

void varType_fromC::write_LHSaddition(csu::utf8_string& LHS_exp, csu::utf8_string& RHS_exp, std::ostream& output)
{
    output << '(' << LHS_exp << '+' << RHS_exp << ')';
}

/// ASSIGNMENT ///
bool varType_fromC::get_has_assignment(varType* RHS_type)
{
    return true;
}

void varType_fromC::write_assignment( utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    output << LHS << '=' << RHS_exp <<';';
}



/*
// resolved function
ResolvedFunction::ResolvedFunction(location_span _loc, sym_table_base* _scope)
{
    definition_name = "RESOLVED_FUNCTION";
    C_name = "";
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
*/




// defined function type
DefFuncType::DefFuncType(utf8_string name, location_span _loc)
{
    loc = _loc; // what??? this doesn't HAVE a location!!
    definition_name = name;
    C_name = "";
    is_ordered = false;
    type_of_type = varType::defined_function_t;
}

DefFuncType::ResolvedFunction_ptr DefFuncType::define_overload(location_span defining_loc, sym_table_base* defining_scope, bool is_ordered)
{
    // first check that new overload does not conflict with current overloads
    for(auto OL : overloads)
    {
        // first check if distinguish by signiture

        // cannot distinguish, see if can override by scope
        if( defining_scope->is_sameScope( OL->defining_scope ) )
        {
            cout << "function defined twice in same scope " << OL->define_loc << " and " << defining_loc << endl;
            return nullptr;
        }
    }

    ResolvedFunction_ptr new_overload = make_shared<resolved_func>( );
    new_overload->defining_scope = defining_scope;
    new_overload->define_loc = defining_loc;
    new_overload->is_ordered = is_ordered;
    new_overload->c_reference = defining_scope->namespace_unique_name + "__";
    new_overload->c_reference += definition_name;
    new_overload->c_reference += utf8_string("__");
    new_overload->c_reference += to_string( num_overloads() );

    overloads.push_back( new_overload );
    return new_overload;
}


 /// CALLING ///
bool DefFuncType::is_callable()
{
    return true;
}

funcCallWriter_ptr DefFuncType::get_resolved_call(location_span call_loc, sym_table_base* calling_scope)
{
    ResolvedFunction_ptr current_overload = nullptr;

    for(auto OL : overloads)
    {
        // first check this overload is reasonable
        if( not OL->defining_scope->is_or_inScope( calling_scope ) )
        {
            continue;
        }

        if( OL->is_ordered and not call_loc.strictly_GT( OL->define_loc ) )
        {
            continue;
        }

        // TODO: need to check signiture works HERE

        if( not current_overload )
        {
            current_overload = OL;
        }
        else
        // OOPS! these two conflict, need to find which one is better
        {
            // check signiture here

            // signitures are the same, so now we check which one has the most inner scope
            if( current_overload->defining_scope->is_or_inScope( OL->defining_scope ) )
            {
                // OL is the most inner (note they cannot be the same scope)
                current_overload = OL;
            }
        }
    }

    return current_overload;
}

void DefFuncType::resolved_func::write_call(csu::utf8_string& LHS, std::ostream& output )
{
    output << c_reference << "()";
}

/*
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
}*/





////// symbol tables //////

// base symbol table
sym_table_base::sym_table_base()
{
    next_variable_ID = 0;
}

utf8_string sym_table_base::get_unique_variable_name()
{
    utf8_string out_string = namespace_unique_name + "__";
    out_string += to_string(next_variable_ID);
    next_variable_ID += 1;
    return out_string;
}

varName_ptr sym_table_base::new_variable(varType_ptr var_type, csu::utf8_string& definition_name, csu::location_span& _loc, bool& is_exclusive, bool is_ordered )
{
    if( is_exclusive )
    {
        bool check_order = false;
        varName_ptr otherV = get_variable_local( definition_name, _loc, check_order);
        check_order = false;
        varType_ptr otherT = get_type_local( definition_name, _loc, check_order);

        if( otherV )
        {
            //throw MultipleDefinition_exc(definition_name, _loc, otherV->loc);
            cout << "name '" << definition_name << "' has multiple definitions. Defined at: " << _loc << " and " << otherV->loc << endl;
            is_exclusive = false;
            return nullptr;
        }
        else if( otherT )
        {
            //throw MultipleDefinition_exc(definition_name, new_type->loc, otherT->loc);
            cout << "name '" << definition_name << "' has multiple definitions. Defined at: " << _loc << " and " << otherT->loc << endl;
            is_exclusive = false;
            return nullptr;
        }
        else
        {
            is_exclusive = true;
        }
    }
    else
    {
        is_exclusive = true;
    }

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

void sym_table_base::add_type(varType_ptr new_type, bool& is_exclusive)
{
    if( is_exclusive )
    {
        bool check_order = false;
        varName_ptr otherV = get_variable_local( new_type->definition_name, new_type->loc, check_order);
        check_order = false;
        varType_ptr otherT = get_type_local( new_type->definition_name, new_type->loc, check_order);

        if( otherV )
        {
            //throw MultipleDefinition_exc(definition_name, _loc, otherV->loc);
            cout << "name '" << new_type->definition_name << "' has multiple definitions. Defined at: " << new_type->loc << " and " << otherV->loc << endl;
            is_exclusive = false;
            return;
        }
        else if( otherT )
        {
            //throw MultipleDefinition_exc(definition_name, new_type->loc, otherT->loc);
            cout << "name '" << new_type->definition_name << "' has multiple definitions. Defined at: " << new_type->loc << " and " << otherT->loc << endl;
            is_exclusive = false;
            return;
        }
        else
        {
            is_exclusive = true;
        }
    }
    else
    {
        is_exclusive = true;
    }


    type_table[new_type->definition_name] = new_type;
}

varName_ptr sym_table_base::get_variable_local(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)
{
    auto table_itterator = variable_table.find( name );
    if(table_itterator != variable_table.end())
    {
        // var found
        varName_ptr variable = table_itterator->second;
        if( variable->is_ordered and not ref_loc.strictly_GT( variable->loc )  )
        {
            // out of order!
            if( check_order ) // we care
            {
                cout << "name '" << name << "' invoked before definition. Defined at: " << variable->loc << ". Referenced at: " << ref_loc << endl;
            }

            check_order = false;
            return variable;

        }
        else
        {
            // IN ORDER!
            check_order = true;
            return variable;
        }
    }
    else
    {
        return nullptr;
    }
}

varType_ptr sym_table_base::get_type_local(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)
{
    auto table_itterator = type_table.find( name );
    if(table_itterator != type_table.end())
    {
        // var found
        varType_ptr type = table_itterator->second;
        if( type->is_ordered and not ref_loc.strictly_GT( type->loc )  )
        {
            // out of order!
            if( check_order ) // we care
            {
                cout << "name '" << name << "' invoked before definition. Defined at: " << type->loc << ". Referenced at: " << ref_loc << endl;
            }

            check_order = false;
            return type;
        }
        else
        {
            // IN ORDER!
            check_order = true;
            return type;
        }
    }
    else
    {
        return nullptr;
    }
}




// local symbol table
local_sym_table::local_sym_table(csu::utf8_string& _name, sym_table_base* parent)
{
    name = _name;
    namespace_unique_name = parent->namespace_unique_name + "__" + name;
    parent_table = parent;
}

varName_ptr local_sym_table::get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)
{
    varName_ptr ret = get_variable_local( name, ref_loc, check_order );

    if(not ret)
        ret = parent_table->get_variable_global(name, ref_loc, check_order);

    return ret;
}

varType_ptr local_sym_table::get_type_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)
{
    varType_ptr ret = get_type_local( name, ref_loc, check_order );

    if(not ret)
        ret = parent_table->get_type_global(name, ref_loc, check_order);

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

    // defined un-named C type
    auto variable_type = make_shared<varType_fromC>();
    variable_type->definition_name = "UNNAMED_C_TYPE";
    variable_type->C_name = "";
    variable_type->is_ordered = false;
    variable_type->unnamed_C_type = variable_type;

    bool is_exclusive = true;
    add_type(variable_type, is_exclusive);


}

void module_sym_table::set_name(csu::utf8_string& _name)
{
    name = _name;
    namespace_unique_name = "__cy__" + name;
}

varName_ptr module_sym_table::get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)
{
    varName_ptr ret = get_variable_local( name, ref_loc, check_order );

    // check imports here?

    return ret;
}

varType_ptr module_sym_table::get_type_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)
{
    varType_ptr ret = get_type_local( name, ref_loc, check_order );

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



