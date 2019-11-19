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

#ifndef SYM_TABLE_180127164658
#define SYM_TABLE_180127164658

#include <map>
#include <memory>
#include <exception>
#include <string>
#include <sstream>
#include <list>

#include "lexer.hpp"
#include "UTF8.hpp"


//// NOTE: the 'name' of a type will need to be generalized away from a string

class BeforeDefinition_exc : public std::exception
{
    csu::utf8_string name;
    csu::location_span def_loc;
    csu::location_span invoke_loc;

    std::string msg;

public:
    //BeforeDefinition_exc(){}
    BeforeDefinition_exc( csu::utf8_string& _name, csu::location_span& _def_loc, csu::location_span& _invoke_loc);

    const char* what();

};

//class NotDefined_exc : public std::exception
//{
//    csu::utf8_string name;
//    csu::location_span invoke_loc;
//
//    std::string msg;
//
//public:
//    NotDefined_exc(){}
//    NotDefined_exc( csu::utf8_string& _name, csu::location_span& _invoke_loc);
//
//    const char* what();
//
//};

class sym_table_base;

class ResolvedFunction;
typedef std::shared_ptr<ResolvedFunction> ResolvedFunction_ptr;

// need to add annotations
class varType
{

public:
    csu::utf8_string definition_name; // probably need to generalize this away from string.
    csu::utf8_string C_name; // Imagine we are writing to C. Everything will be flat, no namespaces. There will only be variables, functions, and structs
    // NOTE: there are types that are not symbols. They have no real name, thus the behavior of C_name and defiition_name may be weird.
    // examples unnamed C type (when import name from C and do not know type): definition_name is UNNAMED_C_TYPE, is in symbol table
    // defined functions (becouse they can be overloaded). definition_name for all are DEFINED_FUNCTION.  Is NOT in symbol table.
    // resolved function overloads,  definition name is "RESOLVED_FUNCTION", not in symbol table, but HOLDS a symbol table

    csu::location_span loc;
    bool is_ordered;

    // I hope I don't need this, it is kind of stupid
    //enum kinds_of_type
    //{
    //    empty,
    //    C_include_t,
    //};
    //kinds_of_type type_kind;

    //bool has_namespace;
    //bool is_callable;
    //bool has_assignment;
    //bool has_addition;



    //bool get_has_namespace(); // need to be able to get namespace
    //bool is_callable();       // need to be able to get calling information

    /// CALLING ///
    virtual bool is_callable();
    virtual bool needs_to_be_resolved(){ return false; }
    virtual ResolvedFunction_ptr get_nonResolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    virtual ResolvedFunction_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope);// TODO: include signiture!


    /// ADDITION ///
    virtual bool get_has_LHSaddition(varType* RHS_type);
    virtual csu::utf8_string get_LHSaddition_name(varType* RHS_type); // eventually these should return arguments that embody function calls
    virtual std::shared_ptr<varType> get_LHSaddition_type(varType* RHS_type);

    /// ASSIGNMENT ///
    virtual bool get_has_assignment(varType* RHS_type);
    virtual csu::utf8_string get_assignment_name(varType* RHS_type);
};
typedef std::shared_ptr<varType> varType_ptr;


// Ctype is weird. Will record whenever a get_has* function is called, and always return true.
// the AST node will accses which functions are needed, and write them.
// NOTE: this is NOT presently well implemented!!
class varType_fromC : public varType
{
private:
    //std::list<varType*> RHStypes_for_LHSaddition;
    //std::list<varType*> types_for_assignment;

public:
    // TODO: fix this!
    varType_ptr unnamed_C_type; // this MUST be set! Is now a small memory leak, as the unnamed_C_type refers to itself.

    /// CALLING /// C types are presently not callable (this needs to change)

    /// ADDITION /// we can add all C types
    bool get_has_LHSaddition(varType* RHS_type);
    csu::utf8_string get_LHSaddition_name(varType* RHS_type);
    varType_ptr get_LHSaddition_type(varType* RHS_type);

    /// ASSIGNMENT /// we can assign to all C types
    bool get_has_assignment(varType* RHS_type);
    csu::utf8_string get_assignment_name(varType* RHS_type);
};

// this may need to be a sub-class of a broader type that represents signiture of things that can be called and have a specific signiture
class ResolvedFunction : public varType
// represents a specific function resolution, based on signiture. Can be used in function pointers
// need to define signiture
{
public:
    sym_table_base* symbol_table_scope;
    std::weak_ptr<ResolvedFunction> self_weak_ptr;

    ResolvedFunction(csu::location_span _loc, sym_table_base* _scope); // TODO: also need to define signiture
    void set_selfPTR(std::weak_ptr<ResolvedFunction> _self);
    void set_unordered(); // is ordered by default

     /// CALLING ///
    bool is_callable();
    bool needs_to_be_resolved() { return false; }
    ResolvedFunction_ptr get_nonResolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    ResolvedFunction_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope);

    /// ADDITION /// cannot add functions
    /// ASSIGNMENT /// cannot assign functions (maybe this should be true??)
};


class DefFuncType : public varType
{
private:
    std::list< ResolvedFunction_ptr > overloads;

public:
    DefFuncType(csu::location_span _loc);

    ResolvedFunction_ptr define_overload(csu::location_span defining_loc, sym_table_base* defining_scope);
    int num_overloads() { return overloads.size(); }

    /// CALLING ///
    bool is_callable();
    bool needs_to_be_resolved() { return true; }
    ResolvedFunction_ptr get_nonResolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    ResolvedFunction_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope);


    /// ADDITION /// cannot add functions
    /// ASSIGNMENT /// cannot assign to defined functions
};
typedef std::shared_ptr<DefFuncType> DefFuncType_ptr;




class varName
{
public:
    varType_ptr var_type;
    csu::utf8_string definition_name;
    csu::utf8_string C_name; // Imagine we are writing to C. Everything will be flat, no namespaces. There will only be variables, functions, and structs
    csu::location_span loc;
    bool is_ordered;
};
typedef std::shared_ptr<varName> varName_ptr;




// note that get_*** should probably have a switch to check location or not. Maybe just catch the exception?
class sym_table_base
{
    unsigned int next_variable_ID;
    std::map<csu::utf8_string, varName_ptr> variable_table;
    std::map<csu::utf8_string, varType_ptr > type_table;

public:

    csu::utf8_string name;
    csu::utf8_string namespace_unique_name;

    sym_table_base();

    csu::utf8_string get_unique_variable_name();

    virtual varName_ptr new_variable(varType_ptr var_type, csu::utf8_string& definition_name, csu::location_span& _loc, bool is_ordered=true);
    virtual void add_type(varType_ptr new_type);

    // local ONLY check this table.
    virtual varName_ptr get_variable_local(csu::utf8_string& name, csu::location_span& ref_loc);
    virtual varType_ptr get_type_local(csu::utf8_string& name, csu::location_span& ref_loc);

    // global checks this table and outer namespaces as well.
    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc)=0;
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc)=0;

    // to check scope for things that are in the scope but not the symbol table
    virtual bool is_or_inScope( sym_table_base* other_scope )=0; // returns true if other is same, or other is an outer scope. IE. if this is In the Scope of other
    virtual bool is_sameScope( sym_table_base* other_scope ){ return this==other_scope; } // returns true if these two scopes are exactly the same
};

typedef std::shared_ptr<sym_table_base> sym_table_ptr;

class local_sym_table : public sym_table_base
{
    sym_table_base* parent_table;
public:

    local_sym_table(csu::utf8_string& _name, sym_table_ptr parent);

    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc);
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc);

    bool is_or_inScope( sym_table_base* other_scope );

};

class module_sym_table : public sym_table_base
{
    //std::list< std::shared_ptr<AST_node> > built_in_info;

public:
    module_sym_table();
    void set_name(csu::utf8_string& _name);

    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc);
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc);

    bool is_or_inScope( sym_table_base* other_scope );
};

#endif
