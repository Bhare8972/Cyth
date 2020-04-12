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
#include <vector>

#include "lexer.hpp"
#include "UTF8.hpp"



class sym_table_base;

//class ResolvedFunction;
//typedef std::shared_ptr<ResolvedFunction> ResolvedFunction_ptr;

class varType;
typedef std::shared_ptr<varType> varType_ptr;

class varName;
typedef std::shared_ptr<varName> varName_ptr;

class func_parameter_info;
typedef std::shared_ptr<func_parameter_info> func_param_ptr;

class funcCall_argument_info;
typedef std::shared_ptr<funcCall_argument_info> callArg_types_ptr;

class arguments_to_parameter_mapper;
typedef std::shared_ptr<arguments_to_parameter_mapper> funcCall_param_map_ptr;

class sym_table_base;
typedef std::shared_ptr<sym_table_base> sym_table_ptr;

class argumentList_AST_ptr;
TODO: write these functions here, and overloads below
class function_call_writer // subclass this for more complex behavior.
{
public:
    // use this to write the arguments.
    funcCall_param_map_ptr param_to_arg_map;
    varType_ptr return_type;

    // this only writes the name. not the arguments (which means it does not write parenthesis)
   // virtual void write_call(csu::utf8_string& LHS, std::ostream& output ); // this has a simple default
    virtual void write_call(csu::utf8_string& LHS, argumentList_AST_ptr arguments_AST, std::ostream& output ); // this has a simple default
    virtual utf8_string write_arguments(argumentList_AST_ptr arguments_AST, std::ostream& output); //writes just the arguments, defined by param_to_arg_map and arguments_AST. Does not write parens (obviously).
};
typedef std::shared_ptr<function_call_writer> funcCallWriter_ptr;

// TODO: expression writers should be able two write C statments and return C expressions.
// ditto for LHS references

class varType
{

public:
    enum varType_type
    {
        empty,
        c_import_t,
        c_pointer_reference,
        defined_function_t,
        function_pntr_t,
        defined_class_t,
        method_function_t,
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

    virtual bool is_equivalent(varType* RHS)=0;

    // some types (ex. callables) may take awhile to get fully defined.
    // if this returns false, we cannot do or test anything with this type....yet
    virtual bool type_is_fully_defined(){return true; }


    /// DEFINITION /// this is just defined as "making a new one in C". Not construction/initiation
    //default Returns false if Ctype is unknown, or if this type has other issue with being defined
    virtual bool can_be_defined();
    // define new variable of this type.
    //Does NOT end with ';'
    virtual void C_definition_name(csu::utf8_string& var_name, std::ostream& output);


    /// CALLING ///
    // default is false
    virtual bool is_callable();
    virtual funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments);


    /// ADDITION ///
    // default is false
    virtual bool get_has_LHSaddition(varType* RHS_type);
    virtual varType_ptr get_LHSaddition_type(varType* RHS_type);
    // surrounds expression with parans
    virtual void write_LHSaddition(varType* RHS_type, csu::utf8_string& LHS_exp, csu::utf8_string& RHS_exp, std::ostream& output);


    /// ASSIGNMENT /// What types can assign TO this type?
    // default is false
    virtual bool get_has_assignment(varType* RHS_type);
    // DOES write end ';'
    virtual void write_assignment(varType* RHS_type, csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output);
    virtual varType_ptr get_auto_type(); // returns null if this doesn't have an auto type.


    /// CLASS THINGS ///

    /// members
          // RHS
          // use when accses membor, but a referance is not needed (could be a copy)
    // only returns null if problem, not print error
    virtual varType_ptr member_has_getter(csu::utf8_string& member_name); // default is null
    // note this wraps new exp in parenthesis. "expression" parameter must have this type
    // could write C statements. Returns C expression
    virtual csu::utf8_string write_member_getter(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output)
        { return ""; }
          //LHS
    // same, but getter MUST be an actual reference. Used in LHS expressions.
    virtual varType_ptr member_has_getref(csu::utf8_string& member_name); // default is null
    // note this wraps new exp in parenthesis. "expression" parameter must have this type
    // could write C statements. Returns C expression
    virtual csu::utf8_string write_member_getref(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output)
        { return ""; }
    // LHS setter (for top level of LHS reference)
     // checks if a member can be set. RHS_type is for checking validity, if null, do not check
     // only returns null if problem, not print error
     // return must be indepenent of RHS_type.
     // default is null
    virtual varType_ptr set_member(csu::utf8_string& member_name, varType* RHS_type=nullptr);
    // set a member to a RHS. note that LHS_expression should return THIS object, not the member
    virtual void write_member_setter(csu::utf8_string& LHS_expression, csu::utf8_string& member_name, varType* RHS_type, csu::utf8_string& RHS_expression, std::ostream& output){}


//    virtual varType_ptr access_member(csu::utf8_string& member_name);
//    virtual void write_member_access(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output){}

    //construction
    // explicit construction
    // implicit conversion
    // explicit conversion
    // destruction

    };

    // Note that

    //      just "that". Just the word "that". just note it.
    //               please

class void_type : public varType
{
public:

    void_type()
    {
        type_of_type = varType::empty;
        definition_name = "void";
        C_name = "void";
        is_ordered = false;
    }

    bool is_equivalent(varType* RHS) override {return false;}
    bool can_be_defined() override { return true; } // ... don't ask ....
    bool is_callable() override { return false; }
    bool get_has_LHSaddition(varType* RHS_type) override { return false; }
    bool get_has_assignment(varType* RHS_type) override { return false; }
};

// Ctype is weird. Will record whenever a get_has* function is called, and always return true.
// the AST node will accses which functions are needed, and write them.
// NOTE: this is NOT presently well implemented!!
class varType_fromC : public varType
{

public:
    std::weak_ptr<varType> unnamed_C_type;// this MUST be set!
    std::weak_ptr<varType> self;


    varType_fromC();
    void set_pointers(std::weak_ptr<varType> _self, std::weak_ptr<varType> _unnamed_C_type);

    bool is_equivalent(varType* RHS) override;

    /// CALLING ///
    bool is_callable() override { return true; }
    funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments) override;


    /// ADDITION /// we can add all C types
    bool get_has_LHSaddition(varType* RHS_type) override;
    varType_ptr get_LHSaddition_type(varType* RHS_type) override;
    void write_LHSaddition(varType* RHS_type, csu::utf8_string& LHS_exp, csu::utf8_string& RHS_exp, std::ostream& output) override;

    /// ASSIGNMENT /// we can assign from all C types
    bool get_has_assignment(varType* RHS_type) override;
    void write_assignment(varType* RHS_type,  csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;
    std::shared_ptr<varType> get_auto_type() override;
};

class raw_C_pointer_reference : public varType
// provides a reference to anouther variable of some type. Used for cyth internals, not in the symbol table
// one use is the "self" variable in methods
{
public:
    varType_ptr referenced_type;

    raw_C_pointer_reference(varType_ptr _ref_type);

    bool is_equivalent(varType* RHS) override;

    /// DEFINITION /// hope this works!
    virtual bool can_be_defined() override;
    virtual void C_definition_name(csu::utf8_string& var_name, std::ostream& output) override;

    /// CALLING /// just call underlying functions with no modification. Hope it works!
    virtual bool is_callable() override;
    virtual funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments) override;

    /// ADDITION /// calls underlying functions, and just de-references LHS name
    virtual bool get_has_LHSaddition(varType* RHS_type) override;
    virtual varType_ptr get_LHSaddition_type(varType* RHS_type) override;
    virtual void write_LHSaddition(varType* RHS_type, csu::utf8_string& LHS_exp, csu::utf8_string& RHS_exp, std::ostream& output) override;

    /// ASSIGNMENT /// again just wraps over reference type, and dereferences LHS when appropriate.
    virtual bool get_has_assignment(varType* RHS_type) override;
    virtual void write_assignment(varType* RHS_type, csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;
    virtual varType_ptr get_auto_type() override {return false;} // just don't deal with this right now...

    /// CLASS THINGS /// again, mimics underlying type with dereferencing
    /// members
    virtual varType_ptr member_has_getter(csu::utf8_string& member_name) override;
    virtual csu::utf8_string write_member_getter(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output) override;
    virtual varType_ptr member_has_getref(csu::utf8_string& member_name) override;
    virtual csu::utf8_string write_member_getref(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output) override;
    virtual varType_ptr set_member(csu::utf8_string& member_name, varType* RHS_type=nullptr) override;
    virtual void write_member_setter(csu::utf8_string& LHS_expression, csu::utf8_string& member_name, varType* RHS_type, csu::utf8_string& RHS_expression, std::ostream& output) override;
};
typedef std::shared_ptr<raw_C_pointer_reference> CPntrRef_ptr;


class func_parameter_info
{
public:
    std::vector<varName_ptr> required_parameters;
    std::vector<varName_ptr> optional_parameters;

    int total_size();
    varName_ptr parameter_from_index(int i);
    int index_from_name(csu::utf8_string& name);
    // returns -1 if name not in parameters.

    // if two functions with same names names in same scope have these parameters, could they be distinguished
    bool is_equivalent(func_parameter_info* RHS);
    bool is_distinguishable(std::shared_ptr<func_parameter_info> other_parameters); // simply not is_equivalent. Hoepfully stays that simple, but don't assume


    void write_to_C(std::ostream& output, bool write_void_for_zero_params=false);// does NOT include parenthesis.
    void print(std::ostream& output);
};

class funcCall_argument_info
{
public:
    std::vector<varType_ptr> unnamed_arguments;
    std::vector<varType_ptr> named_arguments;
    std::vector<csu::utf8_string> argument_names;

    int total_size();
    varType_ptr argType_from_index(int i);

    void print(std::ostream& output);
};

class arguments_to_parameter_mapper
{
public:

    //each value is the index of argument to use. -1 means use default parameters.
    // index is the index of the parameter
    // -2 means index is un-mapped. Should not have if is_good is true
    std::vector<int> param_to_arg_map;

    callArg_types_ptr arguments;
    func_param_ptr parameters;

    bool is_good;
    int num_defaults_used; // smaller is better
    int num_casts;// smaller is better. num_defaults_used is more important

    csu::utf8_string error; // set if is_good is false;

    arguments_to_parameter_mapper(callArg_types_ptr _arguments, func_param_ptr _parameters);

    // returns 1 if this map is prefered. 0 if they are equal, -1 if other is prefered.
    // does not check is_good
    int comparison(funcCall_param_map_ptr RHS);
};


// TODO: replace this with fancy implicit concepts
class staticFuncPntr : public varType
{
public:
    // assume, loc, and is_ordered, will be set
    std::weak_ptr<staticFuncPntr> self; // assume this gets set too
    func_param_ptr parameters; // assume set externally as well
    varType_ptr return_type; // set externally. assume always known from definition.

    staticFuncPntr();

    bool is_equivalent(varType* RHS) override;

    /// CALLING ///
    bool is_callable() override { return true; }
    funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments) override;

    /// ASSIGNMENT /// assign from other func-pointers with same signiture
    bool get_has_assignment(varType* RHS_type) override; // can only assign to equivalent types. in future should be able to cast?
    void write_assignment(varType* RHS_type,  csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;
    std::shared_ptr<varType> get_auto_type() override;

    /// DEFINITION ///
    bool can_be_defined() override { return true; }
    void C_definition_name(csu::utf8_string& var_name, std::ostream& output) override;


    /// ADDITION /// can't do this
};

class DefFuncType : public varType
{
public:

    class writer : public function_call_writer
    {
    public:

//    funcCall_param_map_ptr param_to_arg_map; // is inhereted
//    varType_ptr return_type; // inhereted
        csu::utf8_string c_name;
        void write_call(csu::utf8_string& LHS, std::ostream& output ); // TODO: this should write arguments as well!!
    };
    typedef std::shared_ptr<writer> writer_ptr;

    class resolved_func
    {
    public:
        bool is_ordered;
        sym_table_base* defining_scope;
        csu::location_span define_loc;
        csu::utf8_string c_reference; // RENAME IN DEFINE_OVERLOAD!!!
        func_param_ptr parameters;
        varType_ptr return_type;

        void write_C_prototype(std::ostream& output);
    };
    typedef std::shared_ptr<resolved_func> ResolvedFunction_ptr;


    std::list< ResolvedFunction_ptr > overloads;
    bool all_overloads_defined;

    DefFuncType(csu::utf8_string name, csu::location_span _loc);

    ResolvedFunction_ptr define_overload(csu::location_span defining_loc, sym_table_base* defining_scope, func_param_ptr parameters, bool is_ordered);
    int num_overloads() { return overloads.size(); }
    void define_overload_return(ResolvedFunction_ptr overload, varType_ptr _return_type);

    /// basic type stuff//
    bool is_equivalent(varType* RHS) override {return false;}
    bool type_is_fully_defined() override { return all_overloads_defined; }

    /// CALLING ///
    bool is_callable() override ;

    funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments) override;

    /// DEFINITION ///
    bool can_be_defined() override {return false;}

    /// ADDITION /// cannot add functions ........ unfortunately...... would be cool though

    /// ASSIGNMENT /// cannot assign to defined functions
    std::shared_ptr<varType> get_auto_type() override; // but we MIGHT have an auto-type
    csu::utf8_string single_overload_Cname(); // used for writing to C
};
typedef std::shared_ptr<DefFuncType> DefFuncType_ptr;



/// CLASSES ///
class DefClassType : public varType
{
public:
    sym_table_ptr class_symbol_table;

    DefClassType(csu::utf8_string _name, csu::utf8_string _c_name, bool _is_ordered, sym_table_ptr _class_symbol_table,
                 csu::location_span _loc);

    bool is_equivalent(varType* RHS) override;

    /// DEFINITION ///
    bool can_be_defined() override { return true; }
    void C_definition_name(csu::utf8_string& var_name, std::ostream& output) override;

    /// CLASS THINGS ///
// note getter and getref are mostly the same
// in case of known method, just returns expression to self. Hopefully such expression is sent to method type BEFORE writing C.

    varType_ptr member_has_getter(csu::utf8_string& member_name) override;
    csu::utf8_string write_member_getter(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output) override;

    varType_ptr member_has_getref(csu::utf8_string& member_name) override;
    csu::utf8_string write_member_getref(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output) override;

    //std::shared_ptr<varType> access_member(csu::utf8_string& member_name) override;
    //void write_member_access(csu::utf8_string& expression, csu::utf8_string& member_name, std::ostream& output) override;
    varType_ptr set_member(csu::utf8_string& member_name, varType* RHS_type=nullptr) override;
    void write_member_setter(csu::utf8_string& LHS_expression, csu::utf8_string& member_name, varType* RHS_type, csu::utf8_string& RHS_expression, std::ostream& output) override;
};
typedef std::shared_ptr<DefClassType> ClassType_ptr;


// TODO: improve method type so can take class as parameter!
class MethodType : public varType
{
public:

    class writer : public function_call_writer
    {
    public:
        csu::utf8_string c_name;
        void write_call(csu::utf8_string& LHS, std::ostream& output ); // TODO: this should write arguments as well!!
    };
    typedef std::shared_ptr<writer> writer_ptr;

    class resolved_method
    {
    public:
        bool is_ordered;
        csu::utf8_string c_reference; // RENAME IN DEFINE_OVERLOAD!!!
        csu::location_span define_loc;
        func_param_ptr parameters;
        varType_ptr return_type;
        varName_ptr self_ptr_name;

        void write_C_prototype(std::ostream& output);
    };
    typedef std::shared_ptr<resolved_method> ResolvedMethod_ptr;


    std::list< ResolvedMethod_ptr > overloads;
    int num_undefined_overloads;
    varName_ptr self_ptr_name;

    MethodType(csu::utf8_string name, csu::location_span _loc, varName_ptr _self_ptr_name);

    ResolvedMethod_ptr define_overload(csu::location_span defining_loc, func_param_ptr parameters, sym_table_base* defining_scope);
    int num_overloads() { return overloads.size(); }
    void define_overload_return(ResolvedMethod_ptr overload, varType_ptr _return_type);

    /// basic type stuff//
    bool is_equivalent(varType* RHS) override {return false;}
    bool type_is_fully_defined() override { return num_undefined_overloads==0; }

    /// CALLING ///
    bool is_callable() override ;

    funcCallWriter_ptr get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments) override;

    /// DEFINITION ///
    bool can_be_defined() override {return false;}
};
typedef std::shared_ptr<MethodType> MethodType_ptr;



class varName
{
public:
    varType_ptr var_type;
    csu::utf8_string definition_name;
    csu::utf8_string C_name; // Imagine we are writing to C. Everything will be flat, no namespaces. There will only be variables, functions, and structs
    csu::location_span loc;
    bool is_ordered;
};




// note that get_*** should probably have a switch to check location or not. Maybe just catch the exception?
class sym_table_base
{
    std::map<csu::utf8_string, varName_ptr> variable_table;
    std::map<csu::utf8_string, varType_ptr > type_table;

public:

    csu::utf8_string name;
    csu::utf8_string namespace_unique_name;

    sym_table_base();

    //virtual csu::utf8_string get_unique_variable_name()=0;
    virtual csu::utf8_string get_unique_string()=0;

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


class local_sym_table : public sym_table_base
{
public:
    sym_table_base* parent_table;

    local_sym_table(csu::utf8_string& _name, sym_table_base* parent);
    csu::utf8_string get_unique_string();

    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);

    bool is_or_inScope( sym_table_base* other_scope );

};

class module_sym_table : public sym_table_base
{
    //std::list< std::shared_ptr<AST_node> > built_in_info;
    unsigned long next_variable_ID;

public:
    module_sym_table();
    void set_name(csu::utf8_string& _name);

    csu::utf8_string get_unique_string();

    virtual varName_ptr get_variable_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_global(csu::utf8_string& name, csu::location_span& ref_loc, bool& check_order);

    bool is_or_inScope( sym_table_base* other_scope );
};

#endif
