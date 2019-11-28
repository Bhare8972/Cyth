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
#include <ostream>

#include "lexer.hpp"
#include "UTF8.hpp"



class sym_table_base;

//class ResolvedFunction;
//typedef std::shared_ptr<ResolvedFunction> ResolvedFunction_ptr;

class varType;
typedef std::shared_ptr<varType> varType_ptr;

class function_call_writer
{
public:
    virtual void write_call(csu::utf8_string& LHS, std::ostream& output )=0; // write like is an expression
};
typedef std::shared_ptr<function_call_writer> funcCallWriter_ptr;

class varType
{

public:
    enum varType_type
    {
        empty,
        c_import_t,
        defined_function_t,
    };

    varType_type type_of_type;


    csu::utf8_string definition_name; // probably need to generalize this away from string.
    csu::utf8_string C_name; // Imagine we are writing to C. Everything will be flat, no namespaces. There will only be variables, functions, and structs
    // NOTE: there are types that are not symbols. They have no real name, thus the behavior of C_name and defiition_name may be weird.
    // examples unnamed C type (when import name from C and do not know type): definition_name is UNNAMED_C_TYPE, is in symbol table
    // defined functions (becouse they can be overloaded). definition_name for all are DEFINED_FUNCTION.  Is NOT in symbol table.
    // resolved function overloads,  definition name is "RESOLVED_FUNCTION", not in symbol table, but HOLDS a symbol table

    csu::location_span loc;
    bool is_ordered;



    /// can define new variables? ///
    //Returns false if Ctype is unknown, or if this type has other issue with being defined
    virtual bool can_be_defined();
    // define new variable of this type.
    virtual void C_definition_name(csu::utf8_string& var_name, std::ostream& output); //Does NOT end with ';'


    /// CALLING ///
    virtual bool is_callable();
    virtual funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    // eventually include parameter info


    //virtual bool is_callable();
    //virtual bool needs_to_be_resolved(){ return false; } // TODO: remove this!
    // nonsensical. Should be fully replaced by can_be_referenced
    //virtual ResolvedFunction_ptr get_nonResolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    //virtual ResolvedFunction_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope);// TODO: include signiture!


    /// ADDITION ///
    virtual bool get_has_LHSaddition(varType* RHS_type);
    virtual std::shared_ptr<varType> get_LHSaddition_type(varType* RHS_type);
    virtual void write_LHSaddition(csu::utf8_string& LHS_exp, csu::utf8_string& RHS_exp, std::ostream& output);


    /// ASSIGNMENT ///
    virtual bool get_has_assignment(varType* RHS_type);
    virtual void write_assignment( csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output);
};


// Ctype is weird. Will record whenever a get_has* function is called, and always return true.
// the AST node will accses which functions are needed, and write them.
// NOTE: this is NOT presently well implemented!!
class varType_fromC : public varType
{

public:
    std::weak_ptr<varType> unnamed_C_type;// this MUST be set!


    varType_fromC();

    /// CALLING /// C types are presently not callable (this needs to change)

    /// ADDITION /// we can add all C types
    bool get_has_LHSaddition(varType* RHS_type);
    varType_ptr get_LHSaddition_type(varType* RHS_type);
    void write_LHSaddition(csu::utf8_string& LHS_exp, csu::utf8_string& RHS_exp, std::ostream& output);

    /// ASSIGNMENT /// we can assign to all C types
    bool get_has_assignment(varType* RHS_type);
    void write_assignment( csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output);
};

/*
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
};*/

class DefFuncType : public varType
{
public:
    class resolved_func : public function_call_writer
    {
    public:
        bool is_ordered;
        sym_table_base* defining_scope;
        csu::location_span define_loc;
        csu::utf8_string c_reference;

        void write_call(csu::utf8_string& LHS, std::ostream& output );
    };
    typedef std::shared_ptr<resolved_func> ResolvedFunction_ptr;

private:
    std::list< ResolvedFunction_ptr > overloads;

public:
    DefFuncType(csu::utf8_string name, csu::location_span _loc);

    ResolvedFunction_ptr define_overload(csu::location_span defining_loc, sym_table_base* defining_scope, bool is_ordered);
    int num_overloads() { return overloads.size(); }

    /// CALLING ///
    bool is_callable();
    funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    /*
    bool needs_to_be_resolved() { return true; }
    ResolvedFunction_ptr get_nonResolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    ResolvedFunction_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope);
    */

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

    // if is_exclusive is true, then functions print if two names are same in same scope.
    // if is_exclusive return true, then we are good. If false, then bad. If is_exclusive is false, then it is set to true
    // returns nullptr if and only if is_exclusive returns false
    virtual varName_ptr new_variable(varType_ptr var_type, csu::utf8_string& definition_name, csu::location_span& _loc, bool& is_exclusive, bool is_ordered=true);
    virtual void add_type(varType_ptr new_type, bool& is_exclusive);

    // if variable does not exist
    //     check_order is not changed, returns null
    //  else
    //     if name is orderd and ref is before def
    //         if check_order
    //             print error, check_order is false, return symbol
    //         else:  check_order is false, return symbol
    //     else
    //         check_order is true, return symbol

    // local ONLY check this table.
    virtual varName_ptr get_variable_local(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_local(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);

    // global checks this table and outer namespaces as well.
    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)=0;
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order)=0;

    // to check scope for things that are in the scope but not the symbol table
    virtual bool is_or_inScope( sym_table_base* other_scope )=0; // returns true if other is same, or other is an outer scope. IE. if this is In the Scope of other
    virtual bool is_sameScope( sym_table_base* other_scope ){ return this==other_scope; } // returns true if these two scopes are exactly the same
};

typedef std::shared_ptr<sym_table_base> sym_table_ptr;

class local_sym_table : public sym_table_base
{
    sym_table_base* parent_table;
public:

    local_sym_table(csu::utf8_string& _name, sym_table_base* parent);

    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);

    bool is_or_inScope( sym_table_base* other_scope );

};

class module_sym_table : public sym_table_base
{
    //std::list< std::shared_ptr<AST_node> > built_in_info;

public:
    module_sym_table();
    void set_name(csu::utf8_string& _name);

    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);

    bool is_or_inScope( sym_table_base* other_scope );
};

#endif
