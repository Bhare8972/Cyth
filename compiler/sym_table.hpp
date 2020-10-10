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

class sym_table_base;
typedef std::shared_ptr<sym_table_base> sym_table_ptr;

class call_argument_list;
typedef std::shared_ptr<call_argument_list> argumentList_AST_ptr;

class DefClassType;
typedef std::shared_ptr<DefClassType> ClassType_ptr;

class MethodType;
typedef std::shared_ptr<MethodType> MethodType_ptr;


// TODO:   LESSONS LEARNED THAT SHOULD BE APPLIED
// expression writers should be able to write C statements and return C expressions, generally they need to return C expression writers.
// ditto for LHS references
// functions will, in general, need the AST node
// AST nodes and var type pointers should be replaced with shared pointers
// functions that can error should be able to return messages

class call_argument_list;

class expression_AST_node;
typedef std::shared_ptr<expression_AST_node> expression_AST_ptr;



class expression_call_writer
// This class is returned by some expressions. Essentually needed to control when the cleanup happens
// eventually, all code needs to be refractored so that expression writers return this and not a simple C-expression
// simularly, every writer that takes a C-expression should take a exp_writer_ptr and not a string
{
public:
    virtual csu::utf8_string get_C_expression()=0; // this may be called multiple times
    virtual void write_cleanup()=0;  // this is called once, always after get_C_expression is called at least once and written to C
};
typedef std::shared_ptr<expression_call_writer> exp_writer_ptr;

class simple_expression_writer : public expression_call_writer
{
public:
    csu::utf8_string exp;

    simple_expression_writer(){}

    simple_expression_writer(csu::utf8_string _exp) :
        exp(_exp)
        {}

    csu::utf8_string get_C_expression() {  return exp; }
    void write_cleanup(){}
};


class varType : public std::enable_shared_from_this<varType> // shared_from_this()
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

    // TODO: complete these for each type-of-type
    ClassType_ptr as_class();
    MethodType_ptr as_method();


    csu::utf8_string definition_name; // probably need to generalize this away from string.
    csu::utf8_string C_name; // Imagine we are writing to C. Everything will be flat, no namespaces. There will only be variables, functions, and structs
    // NOTE: there are types that are not symbols. They have no real name, thus the behavior of C_name and defiition_name may be weird.
    // examples unnamed C type (when import name from C and do not know type): definition_name is UNNAMED_C_TYPE, is in symbol table
    // defined functions (becouse they can be overloaded). definition_name for all are DEFINED_FUNCTION.  Is NOT in symbol table.
    // resolved function overloads,  definition name is "RESOLVED_FUNCTION", not in symbol table, but HOLDS a symbol table

    csu::location_span loc;
    bool is_ordered;

    virtual bool is_equivalent(varType* RHS)=0;
    virtual varType_ptr copy(csu::utf8_string _definition_name="" )=0;

    // some types (ex. callables) may take awhile to get fully defined.
    // if this returns false, we cannot do or test anything with this type....yet
    virtual bool type_is_fully_defined(){return true; }


    /// DEFINITION /// this is just defined as "making a new one in C". Not construction/initiation
    //default Returns false if Ctype is unknown, or if this type has other issue with being defined
    virtual bool can_be_defined();
    // define new variable of this type.
    //Does NOT end with ';'  ( this should change!!)
    virtual void C_definition_name(csu::utf8_string& var_name, std::ostream& output);
    virtual void initialize(csu::utf8_string& var_exp, std::ostream& output) {}
    // initialize should do things that only need be done once per type. (such as set the vtable(s) ).
    // good luck calling this in all the right places!



    /// CALLING ///
    // default is null, returns the expected return type
// TODO: make it so argument_node_ptr can be null if we explicitly want call with no arguments or params
    virtual varType_ptr is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg);
    virtual exp_writer_ptr write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
        std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output, bool call_virtual=false);



    /// ADDITION ///
    // default is false
// TODO: combine these two.
    virtual bool get_has_LHSaddition(varType* RHS_type);
    virtual varType_ptr get_LHSaddition_type(varType* RHS_type);
    virtual exp_writer_ptr write_LHSaddition(expression_AST_ptr LHS_ast, csu::utf8_string& LHS_exp, expression_AST_ptr RHS_ast,
       csu::utf8_string& RHS_exp, std::ostream& output, bool call_virtual=false);



    /// ASSIGNMENT /// What types can assign TO this type?
    // default is false
    virtual bool get_has_assignment(varType* RHS_type);
    // DOES write end ';'
// needs to be fixed BADLY, and made possibly virtual
    virtual void write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output);

    virtual varType_ptr get_auto_type(); // returns null if this doesn't have an auto type.

    // default is false
    virtual bool get_has_assignTo(varType* LHS_type);
    virtual void write_assignTo(varType* LHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS,
                        csu::utf8_string& RHS_exp, std::ostream& output, bool call_virtual=false);



    /// REFERENCING ///
// TODO: may need to re-think this, especially is_static_type
    // sets is_ref to true or false accordingly. Returns null if can reference any type, otherwise returns specific type
    virtual varType_ptr is_reference_type(bool &is_ref);
    virtual bool can_take_reference(varType* RHS_type); // must return true at least for the type returned by is_reference_type
    virtual void take_reference(varType* RHS_type, csu::utf8_string& LHS, csu::utf8_string& referencable_RHS_exp, std::ostream& output);
// NOTE: can_take_reference does NOT call can_get_pointer (below) (so casting is controlled!)
//   but take_reference will ALWAYS call get_pointer. Assumes always can get pointer to own type


    // by default, will allow getting own pointer
    // this also, in general, allows implicit pointer casting to parent types
    virtual bool can_get_pointer(varType* output_type); // in general, should return true for equivlent type, at least
    // return C expression should be pointer to output_type. exp needs to be referancable c-expression to this type
    // MUST work for own type.
    virtual csu::utf8_string get_pointer(varType* output_type, csu::utf8_string& exp, std::ostream& output);

    bool is_static_type(){ return true; } // IE, is this just as it appears, or can there be wierd indirection. (IE, can you call a non-virtual destructor?)
// note that non-static types should automatically call virtual methods.
//     Thus, calling code should generally assume type is static, call non-virtual methods, and not worry about it.
//     The non-static type will intercept this and realize the call should be virtual


    /// CLASS THINGS ///


    /// members
          // RHS
          // use when accses membor, a referance is allowed but not needed (could be a copy)
    // only returns null if problem, not print error
    virtual varType_ptr member_has_getter(csu::utf8_string& member_name); // default is null
    // note this wraps new exp in parenthesis. "expression" parameter must have this type
    // could write C statements. Returns C expression
    virtual exp_writer_ptr write_member_getter(csu::utf8_string& expression, csu::utf8_string& member_name,
            std::ostream& output, bool call_virtual=false);

        //LHS
    // same, but getter MUST be an actual reference. Used in LHS expressions.
    virtual varType_ptr member_has_getref(csu::utf8_string& member_name); // default is null
    // note this wraps new exp in parenthesis. "expression" parameter must have this type
    // could write C statements. Returns C expression
    virtual exp_writer_ptr write_member_getref(csu::utf8_string& expression, csu::utf8_string& member_name,
            std::ostream& output, bool call_virtual=false);

    // LHS setter (for top level of LHS reference)
     // checks if a member can be set. RHS_type is for checking validity, if null, do not check
     // only returns null if problem, not print error  (what is it returning when good??? type of WHAT?)
     // if RHS_type is null and member exists, return member type
     // if RHS_type is non-null, return member type if RHS type can be assinged to this member, else null
     // return null if member does not exist, or RHS is not null and cannot be assigned to this member
    virtual varType_ptr set_member(csu::utf8_string& member_name, varType* RHS_type=nullptr);
    // set a member to a RHS. note that LHS_expression should return THIS object, not the member
// make possibly virtual
    virtual void write_member_setter(expression_AST_node* exp_AST_node, csu::utf8_string& LHS_expression, csu::utf8_string& member_name,
             varType* RHS_type, csu::utf8_string& RHS_expression, std::ostream& output, bool call_virtual=false){}



    /// CONSTRUCTORS ///
    // does nothing if no default constructor
    virtual void write_default_constructor(csu::utf8_string& var_name, std::ostream& output){}

// for explicit construction. Will do implicit construction if explicit is not available
    virtual bool has_explicit_constructor(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg);
    virtual void write_explicit_constructor(call_argument_list* argument_node, csu::utf8_string& LHS_Cexp,
                        std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output);


    virtual bool has_implicit_copy_constructor(varType* RHS_type){ return false; }
    virtual void write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS,
                        csu::utf8_string& RHS_exp, std::ostream& output);



    /// DESTRUCTOR ///
    virtual void write_destructor(csu::utf8_string& var_name, std::ostream& output, bool call_virtual=false){}



    /// CASTING ///
// these act like copy constructors on the LHS. Therefore the LHS should not be constructed
// can NOT be used in place of assignment
    virtual bool can_implicit_castTo(varType* LHS_type){return false; }
    // does write ';'.
    virtual void write_implicit_castTo(varType* LHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS, csu::utf8_string& RHS_exp,
                std::ostream& output, bool call_virtual=false){}

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

    varType_ptr copy(csu::utf8_string _definition_name="" ) override
    {
        return std::make_shared<void_type>();
    }

    bool is_equivalent(varType* RHS) override
        { return RHS->definition_name=="void"; }

    bool can_be_defined() override { return true; } // ... don't ask ....
    bool get_has_LHSaddition(varType* RHS_type) override { return false; }
    bool get_has_assignment(varType* RHS_type) override { return false; }
};

// Ctype is weird. Will record whenever a get_has* function is called, and always return true.
// the AST node will accses which functions are needed, and write them.
// NOTE: this is NOT presently well implemented!!
class varType_fromC : public varType
{

public:
    std::weak_ptr<varType> self;// this needs to be depreciated
    std::weak_ptr<varType> unnamed_C_type;// this MUST be set!


    varType_fromC();
    void set_pointers(std::weak_ptr<varType> _self, std::weak_ptr<varType> _unnamed_C_type);

    bool is_equivalent(varType* RHS) override;
    varType_ptr copy(csu::utf8_string _definition_name="" ) override;

    /// CALLING ///
    //bool is_callable() override { return true; }
    //funcCallWriter_ptr get_resolved_call(argumentList_AST_ptr argument_list) override;

    varType_ptr is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;
    exp_writer_ptr write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
        std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output, bool call_virtual=false) override;



    /// ADDITION /// we can add all C types
    bool get_has_LHSaddition(varType* RHS_type) override;
    varType_ptr get_LHSaddition_type(varType* RHS_type) override;
    exp_writer_ptr write_LHSaddition(expression_AST_ptr LHS_ast, csu::utf8_string& LHS_exp, expression_AST_ptr RHS_ast,
        csu::utf8_string& RHS_exp, std::ostream& output, bool call_virtual=false) override;

    /// ASSIGNMENT /// we can assign from all C types
    bool get_has_assignment(varType* RHS_type) override;
    void write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;

    std::shared_ptr<varType> get_auto_type() override;

    /// CONSTRUCTOR ///
    bool has_explicit_constructor(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;
    void write_explicit_constructor(call_argument_list* argument_node, csu::utf8_string& LHS_Cexp, std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output) override;

    // can implicit copy only same type
    bool has_implicit_copy_constructor(varType* RHS_type) override;

    void write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;
};

class raw_C_pointer_reference : public varType
// provides a reference to anouther variable of some type. Used for cyth internals, not in the symbol table
// one use is the "self" variable in methods
{
public:
    enum assignment_mode_t
    {
        empty,
        no_assignment, // cannot assign to this pointer, default
        only_reference_assignment, // can assign if RHS is also a pointer-like thing
        take_reference_assignment, // takes reference of the RHS. Errors if RHS cannot be referenced
        full_assign_assignment, // call underlying assignment operator
    };

    varType_ptr referenced_type;
    assignment_mode_t assignment_mode; // this is very wierd, and I wish I didn't need it


    raw_C_pointer_reference(varType_ptr _ref_type); // default assignment
    raw_C_pointer_reference(varType_ptr _ref_type, assignment_mode_t _assignment_node);

    bool is_equivalent(varType* RHS) override;
    varType_ptr copy(csu::utf8_string _definition_name="" ) override;

    /// DEFINITION /// hope this works!
    bool can_be_defined() override;
    void C_definition_name(csu::utf8_string& var_name, std::ostream& output) override;

    /// CALLING /// just call underlying functions with no modification. Hope it works!
    varType_ptr is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;

    exp_writer_ptr write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
        std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output, bool call_virtual=false) override;


    /// Reference stuff ///
    varType_ptr is_reference_type(bool &is_ref) override; // can return nullptr if can reference general types
    bool can_take_reference(varType* RHS_type) override; // must return true at least for the type returned by is_reference_type
    void take_reference(varType* RHS_type, csu::utf8_string& LHS, csu::utf8_string& referencable_RHS_exp, std::ostream& output) override;

    bool can_get_pointer(varType* output_type) override; // in general, should return true for equivlent type, at least
    // return C expression should be pointer to output_type
    csu::utf8_string get_pointer(varType* output_type, csu::utf8_string& exp, std::ostream& output) override;

    bool is_static_type(){ return false; }


    /// ADDITION /// calls underlying functions, and just de-references LHS name
    bool get_has_LHSaddition(varType* RHS_type) override;
    varType_ptr get_LHSaddition_type(varType* RHS_type) override;
    exp_writer_ptr write_LHSaddition(expression_AST_ptr LHS_ast, csu::utf8_string& LHS_exp, expression_AST_ptr RHS_ast,
         csu::utf8_string& RHS_exp, std::ostream& output, bool call_virtual=false) override;


    /// ASSIGNMENT /// again just wraps over reference type, and dereferences LHS when appropriate.
    bool get_has_assignment(varType* RHS_type) override;
    void write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;

    bool get_has_assignTo(varType* LHS_type) override;
    void write_assignTo(varType* LHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS,
                        csu::utf8_string& RHS_exp, std::ostream& output, bool call_virtual=false) override;

// TODO: auto!

    /// CLASS THINGS /// again, mimics underlying type with dereferencing
    /// members
    varType_ptr member_has_getter(csu::utf8_string& member_name) override;
    exp_writer_ptr write_member_getter(csu::utf8_string& expression, csu::utf8_string& member_name,
                    std::ostream& output, bool call_virtual=false) override ;

    varType_ptr member_has_getref(csu::utf8_string& member_name) override;
    exp_writer_ptr write_member_getref(csu::utf8_string& expression, csu::utf8_string& member_name,
            std::ostream& output, bool call_virtual=false) override;

    varType_ptr set_member(csu::utf8_string& member_name, varType* RHS_type=nullptr) override;

    void write_member_setter(expression_AST_node* exp_AST_node, csu::utf8_string& LHS_expression, csu::utf8_string& member_name,
             varType* RHS_type, csu::utf8_string& RHS_expression, std::ostream& output, bool call_virtual=false) override;

    /// CONSTRUCTORS ///

    // takes reference of RHS when possible.
    bool has_implicit_copy_constructor(varType* RHS_type) override;
    void write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                         csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;


    /// CASTING ///
// TODO: this is weird. Need actual casting!
    bool can_implicit_castTo(varType* LHS_type) override;
    void write_implicit_castTo(varType* LHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS, csu::utf8_string& RHS_exp,
                std::ostream& output, bool call_virtual=false) override;
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

class parameter_to_arguments_mapper
{
    void initialize();
public:
    class argument_wrapper
    {
    public:
        std::vector<expression_AST_node*> unnamed_arguments;
        std::vector<csu::utf8_string> named_argument_names;
        std::vector<expression_AST_node*> named_arguments;

        int total_size();
        void print(std::ostream& output);
        expression_AST_node* expression_from_index(int i);

        argument_wrapper(call_argument_list* argument_node_ptr);
        argument_wrapper( std::vector<expression_AST_node*> &expressions);
    };


    //each value is the index of argument to use. -1 means use default parameters.
    // index is the index of the parameter
    // -2 means index is un-mapped. Should not have if is_good is true
    std::vector<int> param_to_arg_map;

    func_param_ptr parameters;
    argument_wrapper arguments;
    sym_table_base* symbol_table;

    bool is_good;
    int num_defaults_used; // smaller is better
    int num_casts;// smaller is better. num_defaults_used is more important
    int num_pointer_casts; // smaller is better. num_casts is more important

    csu::utf8_string error; // set if is_good is false;

    std::vector<csu::utf8_string> names_to_cleanup;
    std::vector<varType*> types_to_cleanup;

    parameter_to_arguments_mapper(func_param_ptr _parameters, call_argument_list* _argument_node_ptr);
    parameter_to_arguments_mapper(func_param_ptr _parameters, std::vector<expression_AST_node*> &expressions, sym_table_base* _symbol_table);


    // returns 1 if this map is preferred. 0 if they are equal, -1 if other is prefered.
    // does not check is_good
    int comparison(std::shared_ptr<parameter_to_arguments_mapper> RHS);

    void write_arguments( std::vector<csu::utf8_string>& argument_Cexpressions,
                          std::vector<csu::utf8_string> &out, std::ostream& output);

    void write_cleanup( std::ostream& output ); // need to call this to call destructors
};



// TODO: replace this with fancy implicit concepts
//// JUST HERE FOR TESTS. WILL BE REMOVED ///
class staticFuncPntr : public varType
{
public:
    // assume, loc, and is_ordered, will be set
    std::weak_ptr<staticFuncPntr> self; // assume this gets set too. should be depreciated!
    func_param_ptr parameters; // assume set externally as well
    varType_ptr return_type; // set externally. assume always known from definition.

    staticFuncPntr();

    bool is_equivalent(varType* RHS) override;
    varType_ptr copy(csu::utf8_string _definition_name="" ) override;

    /// CALLING ///
    varType_ptr is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;

    exp_writer_ptr write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
        std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output, bool call_virtual=false) override;


    /// ASSIGNMENT /// assign from other func-pointers with same signiture
    bool get_has_assignment(varType* RHS_type) override; // can only assign to equivalent types. in future should be able to cast?
    void write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;
    std::shared_ptr<varType> get_auto_type() override;

    /// DEFINITION ///
    bool can_be_defined() override { return true; }
    void C_definition_name(csu::utf8_string& var_name, std::ostream& output) override;


    /// ADDITION /// can't do this

};

class DefFuncType : public varType
{
public:

    class resolved_func
    {
    public:
        bool is_ordered;
        //sym_table_base* defining_scope; need something like this for nested functiosn, not availablee for now!!
        csu::location_span define_loc;
        csu::utf8_string c_reference; // RENAME IN DEFINE_OVERLOAD!!!
        func_param_ptr parameters;
        varType_ptr return_type;

        void write_C_prototype(std::ostream& output);
    };
    typedef std::shared_ptr<resolved_func> ResolvedFunction_ptr;

    typedef std::pair< ResolvedFunction_ptr, std::shared_ptr<parameter_to_arguments_mapper> > resolved_pair;


    std::list< ResolvedFunction_ptr > overloads;
    bool all_overloads_defined;

    DefFuncType(csu::utf8_string name, csu::location_span _loc);

    ResolvedFunction_ptr define_overload(csu::location_span defining_loc, sym_table_base* defining_scope, func_param_ptr parameters, bool is_ordered);
    int num_overloads() { return overloads.size(); }
    void define_overload_return(ResolvedFunction_ptr overload, varType_ptr _return_type);

    /// basic type stuff//
    bool is_equivalent(varType* RHS) override {return false;}
    bool type_is_fully_defined() override { return all_overloads_defined; }

    varType_ptr copy(csu::utf8_string _definition_name="" ) override;

    /// CALLING ///
    resolved_pair get_resolution(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg);

    varType_ptr is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;

    exp_writer_ptr write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
        std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output, bool call_virtual=false) override;



    /// DEFINITION ///
    bool can_be_defined() override {return false;}

    /// ADDITION /// cannot add functions ........ unfortunately...... would be cool though

    /// ASSIGNMENT /// cannot assign to defined functions
    std::shared_ptr<varType> get_auto_type() override; // but we MIGHT have an auto-type
    csu::utf8_string single_overload_Cname(); // used for writing to C
};
typedef std::shared_ptr<DefFuncType> DefFuncType_ptr;



//some rules:
//    constructors are never virtual or inhereted
//    assignment operators can be virtual, but not inheretited
//    conversion operators are normal (can be inhereted and virtual).

class MethodType : public varType
{
public:

    enum method_type
    {
        empty,
        normal,
        constructor,
        explicit_constructor,
        destructor,
        conversion,
        explicit_conversion,
        assignment,
    };

    class resolved_method
    {
    public:
        bool is_virtual; // default is true. Is false if is constructor, or method is non-overridable

        bool is_ordered;
        csu::utf8_string c_reference; // this is the name of the static function,   RENAME IN DEFINE_OVERLOAD!!!
        csu::location_span define_loc;
        func_param_ptr parameters;
        varType_ptr return_type;
        varName_ptr self_ptr_name; // TODO??? this is the type of where the function is defined, which may be a parent of the current type

        //std::vector< int > parental_vtable_locations; // indeces, relating to full_inheritance_tree, to show which parents have vtable entries for this function
//std::vector< csu::utf8_string > vtable_entry_Cnames; // for every entry in parental_vtable_locations, give cname of the relevent entry in that vtable
        //std::vector< std::shared_ptr<resolved_method> > overriden_methods;
        // note, because lower in the inheritance tree is prefered, and lower is higher indeces,
        //     then parental_vtable_locations will be sorted minimum to maximum, and the last entry is preferred
        // these are generated in build_classes::ClassDef_up::get_inheritance_tree
        // after that, if these are empty and is_virtual is true,
        //      than this function is included in this class vtable under the name c_reference

        //if overriden_method is not null, then this function overrides at least one parental virtual class
        // this gives one parent method that this overrides. The exact one doesn't matter much (right?)
        // parental_vtable_location is the index in full_inheritance_tree of the owning class that says which class owned the overriden method
        unsigned int parental_vtable_location;
        std::shared_ptr<resolved_method> overriden_method;

        void write_C_prototype(std::ostream& output);
    };
    typedef std::shared_ptr<resolved_method> ResolvedMethod_ptr;
    typedef std::pair< ResolvedMethod_ptr, std::shared_ptr<parameter_to_arguments_mapper> > resolved_pair;


    class method_access_writer : public expression_call_writer
    // this is needed, because methods are weird. note that the "c-expression" should have the type of parent type (type of self_ptr_name, below)
    {
    public:
        csu::utf8_string parent_exp;
        bool virtual_call_allowed; // if false, use static call, it true, use virtual call if possible

        method_access_writer(csu::utf8_string _parent_exp, bool _virtual_call_allowed) :
            parent_exp(_parent_exp)
            { virtual_call_allowed = _virtual_call_allowed; }

        csu::utf8_string get_C_expression() {  return parent_exp; }
        void write_cleanup(){}
    };




    typedef std::list< ResolvedMethod_ptr >::iterator overloads_iterator;
    std::list< ResolvedMethod_ptr > overloads;
    int num_undefined_overloads;
    varName_ptr self_ptr_name; // originally null. Set when we know
    method_type type_of_method;// this can be altered externally

    MethodType(csu::utf8_string name, csu::location_span _loc, varName_ptr _self_ptr_name);

    ResolvedMethod_ptr define_overload(csu::location_span defining_loc, func_param_ptr parameters, sym_table_base* defining_scope);
    int num_overloads() { return overloads.size(); }
    void define_overload_return(ResolvedMethod_ptr overload, varType_ptr _return_type);

    /// basic type stuff//
    bool is_equivalent(varType* RHS) override {return false;}
    bool type_is_fully_defined() override { return num_undefined_overloads==0; }

    varType_ptr copy(csu::utf8_string _definition_name="" ) override;

    /// CALLING ///
    resolved_pair get_resolution(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg);

    varType_ptr is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;

    exp_writer_ptr write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
       std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output, bool call_virtual=false) override;

//    exp_writer_ptr write_nonvirtual_call(call_argument_list* argument_node,
//               csu::utf8_string& LHS_Cexp, std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output);
//
     exp_writer_ptr write_call(ResolvedMethod_ptr resolved_method, std::vector<expression_AST_node*> &argument_expressions,
               sym_table_base* _symbol_table, exp_writer_ptr LHS_Cexp, std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output);


    /// DEFINITION ///
    bool can_be_defined() override {return false;}


    /// helper functions
    ResolvedMethod_ptr get_indestinguishable_overload(func_param_ptr parameters);
    bool overload_was_inhereted(ResolvedMethod_ptr ovrld);
};



class method_connection
{
public:
    MethodType::ResolvedMethod_ptr methodOverload_to_override;

    ClassType_ptr class_that_overrides;
    MethodType::ResolvedMethod_ptr methodOverload_that_overrides;
};

/// CLASSES ///
class DefClassType : public varType
{
public:
    sym_table_ptr class_symbol_table;
    csu::utf8_string vtableType_cname; // this is the type of the vtable
    csu::utf8_string global_vtableVar_cname; // This is the global name of the vtable, that this vtable should be set to. Need one for every parent type, so this will need to be a tree of some sort

    std::vector< ClassType_ptr > top_level_inheritances; // later is lower
    std::vector< varName_ptr > parent_class_names; // these are part of the class symbol table, as members. // set in build_classes

    std::vector< ClassType_ptr > full_inheritance_tree; // earlier is further up the tree; therefore later classes override earlier classes
    std::vector< int > inheritance_tree_level; //each item in full_inheritance_tree has an index here. The index shows
    // which item in full_inheritance_tree is the child of the relavent class.
    //-1 means the class is the immediate parent of this class.
    std::vector< csu::utf8_string > global_parentVtable_cnames; // c-names of each global vtable, set in build_classes. One entry per full_inheritance_tree
    std::vector< std::multimap< csu::utf8_string, method_connection > > method_connection_table;
    // method_connection_table says which method-overloads are overriden, or not, by which other method-overloads. outer vector corresponds to full_inheritance_tree
    // inner multimap has an entry for every  top-level (non-overriding) method-overload in that class.
    // Each entry says where that method-overload is overriden. If a method is NOT overridden then methodOverload_that_overrides==methodOverload_to_override


    DefClassType(csu::utf8_string _name, csu::utf8_string _c_name, bool _is_ordered, sym_table_ptr _class_symbol_table,
                 csu::location_span _loc);

    bool is_equivalent(varType* RHS) override;
    varType_ptr copy(csu::utf8_string _definition_name="" ) override;



    /// DEFINITION ///
    bool can_be_defined() override { return true; }
    void C_definition_name(csu::utf8_string& var_name, std::ostream& output) override;
    void initialize(csu::utf8_string& var_exp, std::ostream& output) override;


    /// REFERENCING ///
    // by default, will allow getting own pointer
    bool can_get_pointer(varType* output_type) override; // in general, should return true for equivlent type, at least
    // return C expression should be pointer to output_type. exp needs to be referancable c-expression to this type
    csu::utf8_string get_pointer(varType* output_type, csu::utf8_string& exp, std::ostream& output) override;


    /// CLASS THINGS ///

    //given a member name, find parent with that member. Doesn't worry about vtables.
    // parent_index set to -1 for this class, -2 if variable not found
    varName_ptr get_member_full_inheritance(int& parent_index, csu::utf8_string& member_name);


// note getter and getref are mostly the same
// in case of known method, just returns expression to self. Hopefully such expression is sent to method type BEFORE writing C.
// self is cast to the type of the class the defines the method (even if it isn't the one with the vtable entry).

    varType_ptr member_has_getter(csu::utf8_string& member_name) override;
    exp_writer_ptr write_member_getter(csu::utf8_string& expression, csu::utf8_string& member_name,
                std::ostream& output, bool call_virtual=false) override;

    varType_ptr member_has_getref(csu::utf8_string& member_name) override;
    exp_writer_ptr write_member_getref(csu::utf8_string& expression, csu::utf8_string& member_name,
        std::ostream& output, bool call_virtual=false) override;

    varType_ptr set_member(csu::utf8_string& member_name, varType* RHS_type=nullptr) override;
    // virtual bit doesn't really work, as we don't have setters yet,
    void write_member_setter(expression_AST_node* exp_AST_node, csu::utf8_string& LHS_expression, csu::utf8_string& member_name,
             varType* RHS_type, csu::utf8_string& RHS_expression, std::ostream& output, bool call_virtual=false) override;

    /// CONSTRUCTORS ///
    void write_default_constructor(csu::utf8_string& var_name, std::ostream& output) override;

    bool has_explicit_constructor(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;
    void write_explicit_constructor(call_argument_list* argument_node, csu::utf8_string& LHS_Cexp, std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output) override;

    bool has_implicit_copy_constructor(varType* RHS_type) override;
    void write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output) override;


    /// DESTRUCTORS ///
    void write_destructor(csu::utf8_string& var_name, std::ostream& output, bool call_virtual=false) override;


    /// ASSIGNMENT ///  just replicates copy constructor (implicit copy constructror now, should be explicit)
    virtual bool get_has_assignment(varType* RHS_type);
    virtual void write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  csu::utf8_string& LHS, csu::utf8_string& RHS_exp, std::ostream& output);
    //virtual varType_ptr get_auto_type();


    /// CASTING ///
    bool can_implicit_castTo(varType* LHS_type) override;
    void write_implicit_castTo(varType* LHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS, csu::utf8_string& RHS_exp,
                std::ostream& output, bool call_virtual=false) override;


/// special interface things needed for just classes
    MethodType_ptr get_method(csu::utf8_string &name);  // note, this method can include inherited overloads. But DOESN'T search parent classes

    /// some looping helpers
    /// loop over each method
//TODO: make looping over inhereted methods an option for this iterator?
//this propbably shouldnt even exist!
//    class methodIter // SHOULD this loop-over?? inhereted methods??
//    {
//    public:
//        std::map<csu::utf8_string, varName_ptr>::iterator internal_iter;
//        std::map<csu::utf8_string, varName_ptr>::iterator end_iter;
//
//        MethodType_ptr get();
//        methodIter& operator++();
//        bool operator!=(const methodIter& RHS);
//        bool operator==(const methodIter& RHS);
//    };
//    methodIter methodIter_begin();
//    methodIter methodIter_end();

    /// loop over each overload of every method
    class methodOverloadIter
    {
    public:
        std::map<csu::utf8_string, varName_ptr>::iterator method_iter; // if these are equal, this iterator is at end.
        std::map<csu::utf8_string, varName_ptr>::iterator method_end_iter;
        MethodType::overloads_iterator overload_iter;
        MethodType::overloads_iterator overload_end_iter;
        bool do_inhereted_overloads;

        MethodType_ptr method_get();
        MethodType::ResolvedMethod_ptr overload_get();

        methodOverloadIter& operator++();
        bool operator!=(const methodOverloadIter& RHS);
        bool operator==(const methodOverloadIter& RHS);
    };
    methodOverloadIter methodOverloadIter_begin(bool do_inhereted_overloads=false); //if false, skip overloads that were only inhereted
    methodOverloadIter methodOverloadIter_end();

    ///parents
    int get_parent_index( ClassType_ptr parent_class ); // return the index in inheritance tree of the parent, -1 if this class doesn't inherit from parent
    int get_immediate_parent_index( ClassType_ptr parent_class ); // only searches immediate parents, -1 if not immediate parent

    class parentIter
    {
        public:
        int current_parent_index; // -1 is this class, -2 means end
        ClassType_ptr self_type;

        ClassType_ptr get();
        int get_parent_index();
        parentIter& operator++();
        bool operator!=(const parentIter& RHS); // only compares the index, not the self_type
        bool operator==(const parentIter& RHS);
    };
    parentIter parentIter_begin( int parent_index ); // parentIter initially points to the parent
    parentIter parentIter_end();


    // given a parent index, and referencable C expression with this type, return referencable c-exp with parent type
    csu::utf8_string write_parent_access(int parent_index, csu::utf8_string& exp, std::ostream& output);
};



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
public:

    std::map<csu::utf8_string, varName_ptr> variable_table;
    std::map<csu::utf8_string, varType_ptr > type_table;

    csu::utf8_string name;
    csu::utf8_string namespace_unique_name;

    sym_table_base();

    //virtual csu::utf8_string get_unique_variable_name()=0;
    virtual csu::utf8_string get_unique_string()=0;

    // if is_exclusive is true, then functions print if two names are same in same scope.
    // if is_exclusive return true, then we are good. If false, then bad. If is_exclusive is false, then it is set to true
    // returns nullptr if and only if is_exclusive returns false
    virtual varName_ptr new_variable(varType_ptr var_type, csu::utf8_string& definition_name, csu::location_span& _loc, bool& is_exclusive, bool is_ordered=true);
    virtual void add_variable(varName_ptr new_var, bool& is_exclusive);
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
    virtual varName_ptr get_variable_local(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_local(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order);

    // global checks this table and outer namespaces as well.
    virtual varName_ptr get_variable_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)=0;
    virtual varType_ptr get_type_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)=0;

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

    virtual varName_ptr get_variable_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order);

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

    virtual varName_ptr get_variable_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order);
    virtual varType_ptr get_type_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order);

    bool is_or_inScope( sym_table_base* other_scope );
};

#endif
