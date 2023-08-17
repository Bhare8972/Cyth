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

// This should probably be split into multiple files

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
#include "c_expressions.hpp"
#include "c_source_writer.hpp"




class varType;
typedef std::shared_ptr<varType> varType_ptr;

class varName;
typedef std::shared_ptr<varName> varName_ptr;

class sym_table_base;
typedef std::shared_ptr<sym_table_base> sym_table_ptr;

class DefClassType;
typedef std::shared_ptr<DefClassType> ClassType_ptr;

class MethodType;
typedef std::shared_ptr<MethodType> MethodType_ptr;

class MetaType;
typedef std::shared_ptr<MetaType> MetaType_ptr;


class func_parameter_info;
typedef std::shared_ptr<func_parameter_info> func_param_ptr;

class function_argument_types;
typedef std::shared_ptr<function_argument_types> function_argument_types_ptr;

class parameter_to_arguments_mapper;
typedef std::shared_ptr<parameter_to_arguments_mapper> parm_arg_map_ptr;


enum class cast_enum
{
    explicit_casts,
    implicit_casts,
    pntr_casts
    //no_casting not implemented
};

// a rule when using C_expression_ptr.
// if one of the methods of varType takes a C_expression_ptr, and returns a C_expression_ptr.
// the return C_expression_ptr will NOT own the C_expression_ptr passed to the method (I think??)

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
        metatype_t,
    };

    varType_type type_of_type;

    // TODO: complete these for each type-of-type
    ClassType_ptr as_class();
    MethodType_ptr as_method();
    MetaType_ptr as_metatype();


    csu::utf8_string definition_name; // probably need to generalize this away from string.
    csu::utf8_string C_name; // Imagine we are writing to C. Everything will be flat, no namespaces. There will only be variables, functions, and structs
    // NOTE: there are types that are not symbols. They have no real name, thus the behavior of C_name and defiition_name may be weird.
    // examples unnamed C type (when import name from C and do not know type): definition_name is UNNAMED_C_TYPE, is in symbol table
    // defined functions (becouse they can be overloaded). definition_name for all are DEFINED_FUNCTION.  Is NOT in symbol table.
    // resolved function overloads,  definition name is "RESOLVED_FUNCTION", not in symbol table, but HOLDS a symbol table

    csu::location_span loc;
    bool is_ordered;

    virtual bool is_equivalent(varType_ptr RHS); // default checks pointer

    // some types (ex. callables) may take awhile to get fully defined.
    // if this returns false, we cannot do or test anything with this type....yet
    virtual bool type_is_fully_defined(){return true; }


    /// DEFINITION /// this is just defined as "making a new one in C". Not construction/initiation
    //default Returns false if Ctype is unknown, or if this type has other issue with being defined
    virtual bool can_be_defined();
    // define new variable of this type.
    //Does NOT end with ';'  ( this should change!!)
    virtual void C_definition_name(csu::utf8_string& var_name, Csource_out_ptr output);
    virtual void initialize(csu::utf8_string& var_exp, Csource_out_ptr output) {}
    // initialize should do things that only need be done once per type. (such as set the vtable(s) ).
    // good luck calling this in all the right places!


    // call when object is C-copied without calling constructor or assignment.
    // Often used when passing too and from a method.
    virtual void inform_moved(csu::utf8_string& var_name, Csource_out_ptr output, bool call_virtual=false){}


    virtual varType_ptr get_auto_type(){ return nullptr; }; // returns null if this doesn't have an auto type.


    /// CALLING ///
    // default is null, returns the expected return type
// TODO: make it so argument_node_ptr can be null if we explicitly want call with no arguments or params
    virtual varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                cast_enum cast_behavior=cast_enum::implicit_casts);
    virtual C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
        std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                cast_enum cast_behavior=cast_enum::implicit_casts);
        // note: return of write_call is a bit sophisticated. If return is unknown C-type, then the return is most likely a string representing the call itself
            // in which case is unreferencable, and thus also claims to not own its memory, which is technically true...
        // otherwise this will write the call to output, and the return will represent a temporary variable if possible
        //    or an empty void if not
        // hopefully this all behaves as it should.


  /// BINARY OPERATORS ///
    // default is null (no)
    // MATH
      // power
    virtual varType_ptr get_has_LHSpower(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSpower(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
      // multiplication
    virtual varType_ptr get_has_LHSmultiplication(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSmultiplication(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
      // division
    virtual varType_ptr get_has_LHSdivision(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSdivision(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
      // modulus
    virtual varType_ptr get_has_LHSmodulus(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSmodulus(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
      // addition
    virtual varType_ptr get_has_LHSaddition(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSaddition(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
      // subtraction
    virtual varType_ptr get_has_LHSsubtraction(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSsubtraction(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);


// COMPARISON
    // lessThan
    virtual varType_ptr get_has_LHSlessThan(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSlessThan(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false);

    // greatThan

    virtual varType_ptr get_has_LHSgreatThan(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSgreatThan(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);

    // equalTo
    virtual varType_ptr get_has_LHSequalTo(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSequalTo(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);

    // notEqual
    virtual varType_ptr get_has_LHSnotEqual(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSnotEqual(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);

    // lessEqual
    virtual varType_ptr get_has_LHSlessEqual(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSlessEqual(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);

    // greatEqual
    virtual varType_ptr get_has_LHSgreatEqual(varType_ptr RHS_type){ return nullptr; };
    virtual C_expression_ptr write_LHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);
    virtual varType_ptr get_has_RHSgreatEqual(varType_ptr LHS_type){ return nullptr; };
    virtual C_expression_ptr write_RHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false);




    /// ASSIGNMENT /// What types can assign TO this type?
    // default is false
    virtual bool get_has_assignment(varType_ptr RHS_type){ return false; }
    // DOES write end ';'
    virtual void write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false);

    // assignTo is when assignment is defined by RHS type, and not LHS type. normal assignment is prefered
    // default is false
    virtual bool get_has_assignTo(varType_ptr LHS_type) { return false; }
    virtual void write_assignTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false);




    /// REFERENCING ///
    // sets is_ref to true or false accordingly. Returns null if can reference any type, otherwise returns specific type
    virtual varType_ptr is_reference_type(bool &is_ref);
    virtual bool can_take_reference(varType_ptr RHS_type){ return false; } // must return true at least for the type returned by is_reference_type
    virtual void take_reference(C_expression_ptr LHS_exp, C_expression_ptr referencable_RHS_exp, Csource_out_ptr output);
// NOTE: can_take_reference does NOT call can_get_pointer (below) (so casting is controlled!) Thus can_take_reference doesn't account for polymorphism
//   but take_reference will ALWAYS call get_pointer. Assumes always can get pointer to own type

// the way this works is that if ref-type can_take_reference than call  can_get_pointer on RHS (with ref-type is_reference_type as argument)
// need to consider rules when is_reference_type is null... not fully considered yet

// some (simple) ref-types need external memory given (via take reference).
// If this is false, then var can get dynamic memory when needed.
// If this returns true, is_reference_type MUST return a specific type!
    virtual bool needs_external_memory(){ return false; }


// these are defined for all types, necessary for taking references
    // by default, will allow getting own pointer
    // this also, in general, allows implicit pointer casting to parent types
    virtual bool can_get_pointer(varType_ptr output_type); // in general, should return true for equivlent type, at least
    // return C expression should be pointer to output_type. exp needs to be referancable c-expression to this type
    // MUST work for own type. This is essentually the lowest-level interface for polymorphism
    virtual csu::utf8_string get_pointer(varType_ptr output_type, csu::utf8_string& exp, Csource_out_ptr output);


// we we even use this??
  //  bool is_static_type(){ return true; } // IE, is this just as it appears, or can there be wierd indirection. (IE, can you call a non-virtual destructor?)
// note that non-static types should automatically call virtual methods.
//     Thus, calling code should generally assume type is static, call non-virtual methods, and not worry about it.
//     The non-static type will intercept this and realize the call should be virtual




    /// CLASS THINGS ///
    /// members
          // RHS
          // use when accses membor, a referance is allowed but not needed (could be a copy)
    // only returns null if problem, not print error
    virtual varType_ptr member_has_getter(csu::utf8_string& member_name){ return nullptr; } // default is null
    // note this wraps new exp in parenthesis. "expression" parameter must have this type
    // could write C statements. Returns C expression
    virtual C_expression_ptr write_member_getter(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual=false);

        //LHS
    // same, but getter MUST be an actual reference. Used in LHS expressions.
    virtual varType_ptr member_has_getref(csu::utf8_string& member_name){ return nullptr; } // default is null
    // note this wraps new exp in parenthesis. "expression" parameter must have this type
    // could write C statements. Returns C expression
    virtual C_expression_ptr write_member_getref(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual=false);

    // LHS setter (for top level of LHS reference)
     // checks if a member can be set. RHS_type is for checking validity, if null, do not check
     // only returns null if problem, not print error
     // if RHS_type is null and member exists, return member type
     // if RHS_type is non-null, return member type if RHS type can be assinged to this member, else null
     // return null if member does not exist, or RHS is not null and cannot be assigned to this member
    virtual varType_ptr set_member(csu::utf8_string& member_name, varType_ptr RHS_type=nullptr){ return nullptr; }
    // set a member to a RHS. note that LHS_expression should return THIS object, not the member
    virtual void write_member_setter(C_expression_ptr LHS_expression, csu::utf8_string& member_name,
             C_expression_ptr RHS_expression, Csource_out_ptr output, bool call_virtual=false){}



    /// CONSTRUCTORS ///
    // does nothing if no default constructor
    virtual void write_default_constructor(csu::utf8_string& var_name, Csource_out_ptr output){}

// for explicit construction. Will do implicit construction if explicit is not available
    virtual bool has_explicit_constructor(function_argument_types_ptr argument_types, csu::utf8_string& error_msg);
    virtual void write_explicit_constructor(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
                        std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output);

    virtual bool has_explicit_copy_constructor(varType_ptr RHS_type, csu::utf8_string& error_msg); // convience for when only one argument.
    virtual void write_explicit_copy_constructor(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                                             Csource_out_ptr output);


// cast_behavior is needed to avoided infinite do_loops
    virtual bool has_implicit_copy_constructor(varType_ptr RHS_type, cast_enum cast_behavior=cast_enum::implicit_casts){ return false; }
    virtual void write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior=cast_enum::implicit_casts);



    /// DESTRUCTOR ///
    virtual void write_destructor(csu::utf8_string& var_name, Csource_out_ptr output, bool call_virtual=false){}



    /// CASTING ///
// these act like copy constructors on the LHS. Therefore the LHS should not be constructed
// can NOT be used in place of assignment
    virtual bool can_implicit_castTo(varType_ptr LHS_type){return false; }
    virtual void write_implicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual=false){}

// this should try to call implicit casting first.
// by default this will just call implicit castTo
    virtual bool can_explicit_castTo(varType_ptr LHS_type){return can_implicit_castTo(LHS_type); }
    // does write ';'.
    virtual void write_explicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual=false)
                {  write_implicit_castTo( LHS_exp, RHS_exp, output, call_virtual); }



    /// interactions with C ///
    // return type when "cast" to c-type. return null if not possible (default)
    // can return self if already a useful c-type
    virtual varType_ptr can_toC(){ return nullptr; }
    virtual C_expression_ptr toC(C_expression_ptr exp, Csource_out_ptr output, bool call_virtual=false);
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

    bool is_equivalent(varType_ptr RHS) override
        { return RHS->definition_name=="void"; }

    bool can_be_defined() override { return true; } // ... don't ask ....
};

// Ctype is weird. Will record whenever a get_has* function is called, and always return true.
// the AST node will accses which functions are needed, and write them.
// NOTE: this is NOT presently well implemented!!
class varType_fromC : public varType
{

public:
    std::weak_ptr<varType> unnamed_C_type;// this MUST be set!


    varType_fromC();
    void set_pointers( std::weak_ptr<varType> _unnamed_C_type);

    bool is_equivalent(varType_ptr RHS) override;

    /// CALLING ///
    varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;
    C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;



    /// BINARY OPERATORS ///
    // MATH
    // we use the same rules for all binary operators
    varType_ptr has_binOperator(varType_ptr RHS_type);
    C_expression_ptr write_LHSoperator(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, const char* op);
    C_expression_ptr write_RHSoperator(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, const char* op);

          // power
//    varType_ptr get_has_LHSpower(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
//    C_expression_ptr write_LHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
//                              Csource_out_ptr output, bool call_virtual=false) override
//                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "**"); }
//
//    varType_ptr get_has_RHSpower(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
//    C_expression_ptr write_RHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
//                              Csource_out_ptr output, bool call_virtual=false) override
//                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "**"); }
      // multiplication
    varType_ptr get_has_LHSmultiplication(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "*"); }

    varType_ptr get_has_RHSmultiplication(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "*"); }
      // division
    varType_ptr get_has_LHSdivision(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "/"); }

    varType_ptr get_has_RHSdivision(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "/"); }
      // modulus
    varType_ptr get_has_LHSmodulus(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "%"); }

    varType_ptr get_has_RHSmodulus(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "%"); }
      // addition
    varType_ptr get_has_LHSaddition(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "+"); }

    varType_ptr get_has_RHSaddition(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "+"); }
      // subtraction
    varType_ptr get_has_LHSsubtraction(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "-"); }

    varType_ptr get_has_RHSsubtraction(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "-"); }
    // COMPARISON
      // lessThan
    varType_ptr get_has_LHSlessThan(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "<"); }

    varType_ptr get_has_RHSlessThan(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "<"); }
      // greatThan
    varType_ptr get_has_LHSgreatThan(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, ">"); }

    varType_ptr get_has_RHSgreatThan(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, ">"); }
      // equalTo
    varType_ptr get_has_LHSequalTo(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "=="); }

    varType_ptr get_has_RHSequalTo(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "=="); }
      // notEqual
    varType_ptr get_has_LHSnotEqual(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "!="); }

    varType_ptr get_has_RHSnotEqual(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "!="); }
      // lessEqual
    varType_ptr get_has_LHSlessEqual(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, "<="); }

    varType_ptr get_has_RHSlessEqual(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, "<="); }
      // greatEqual
    varType_ptr get_has_LHSgreatEqual(varType_ptr RHS_type) override { return has_binOperator(RHS_type); }
    C_expression_ptr write_LHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_LHSoperator(LHS_exp, RHS_exp, output, ">="); }

    varType_ptr get_has_RHSgreatEqual(varType_ptr LHS_type) override { return has_binOperator(LHS_type); }
    C_expression_ptr write_RHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false)override
                              { return write_RHSoperator(LHS_exp, RHS_exp, output, ">="); }





    /// ASSIGNMENT /// we can assign from all C types
    bool get_has_assignment(varType_ptr RHS_type) override;
    void write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false) override;

    varType_ptr get_auto_type() override { return shared_from_this(); }

    /// CONSTRUCTOR ///
    bool has_explicit_constructor(function_argument_types_ptr argument_types, csu::utf8_string& error_msg) override;
    void write_explicit_constructor(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
                        std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output) override;

    bool has_explicit_copy_constructor(varType_ptr RHS_type, csu::utf8_string& error_msg) override;
    void write_explicit_copy_constructor(C_expression_ptr LHS_exp,
                        C_expression_ptr RHS_exp, Csource_out_ptr output) override;

    // can implicit copy only same type
    bool has_implicit_copy_constructor(varType_ptr RHS_type, cast_enum cast_behavior=cast_enum::implicit_casts) override;
    void write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior=cast_enum::implicit_casts) override;



    /// interactions with C ///
    varType_ptr can_toC() override { return shared_from_this(); }
    C_expression_ptr toC(C_expression_ptr exp, Csource_out_ptr output, bool call_virtual=false) override
    { return  std::make_shared<simple_C_expression>(exp->get_C_expression(), shared_from_this(), false, false); }
};

class raw_C_pointer_reference : public varType
// provides a reference to anouther variable of some type. Used for cyth internals, not in the symbol table
// one use is the "self" variable in methods
{
private:
    // given C_expression_ptr (exp) of this type (which is a pointer), return a C_expression_ptr of the referenced type
    C_expression_ptr deref(C_expression_ptr exp);

public:
    enum assignment_mode_t
    {
        empty,
        no_assignment, // cannot assign to this pointer, default
        only_reference_assignment, // can assign if RHS is also a pointer-like thing ; can do pointer casting to parent type
        take_reference_assignment, // takes reference of the RHS. Errors if RHS cannot be referenced ; can do pointer casting to parent type
        full_assign_assignment, // call underlying assignment operator
    };

    varType_ptr referenced_type;
    assignment_mode_t assignment_mode; // this is very wierd, and I wish I didn't need it


    raw_C_pointer_reference(varType_ptr _ref_type); // default assignment
    raw_C_pointer_reference(varType_ptr _ref_type, assignment_mode_t _assignment_node);

    bool is_equivalent(varType_ptr RHS) override;

    /// DEFINITION /// hope this works!
    void C_definition_name(csu::utf8_string& var_name, Csource_out_ptr output) override;

    /// CALLING /// just call underlying functions with no modification. Hope it works!
    varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

    C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;


    /// Reference stuff ///
    varType_ptr is_reference_type(bool &is_ref) override; // can return nullptr if can reference general types
    bool can_take_reference(varType_ptr RHS_type) override; // must return true at least for the type returned by is_reference_type
    void take_reference(C_expression_ptr LHS_exp, C_expression_ptr referencable_RHS_exp, Csource_out_ptr output) override;
    bool needs_external_memory() override { return true; }

    bool can_get_pointer(varType_ptr output_type) override; // in general, should return true for equivlent type, at least
    // return C expression should be pointer to output_type
    csu::utf8_string get_pointer(varType_ptr output_type, csu::utf8_string& exp, Csource_out_ptr output) override;



    /// BINARY OPERATORS /// calls underlying functions, and just de-references LHS name
      // power
    varType_ptr get_has_LHSpower(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSpower(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // multiplication
    varType_ptr get_has_LHSmultiplication(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSmultiplication(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // division
    varType_ptr get_has_LHSdivision(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSdivision(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // modulus
    varType_ptr get_has_LHSmodulus(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSmodulus(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // addition
    varType_ptr get_has_LHSaddition(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSaddition(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // subtraction
    varType_ptr get_has_LHSsubtraction(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSsubtraction(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;

      // lessThan
    varType_ptr get_has_LHSlessThan(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSlessThan(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // greatThan
    varType_ptr get_has_LHSgreatThan(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSgreatThan(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // equalTo
    varType_ptr get_has_LHSequalTo(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSequalTo(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // notEqual
    varType_ptr get_has_LHSnotEqual(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSnotEqual(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // lessEqual
    varType_ptr get_has_LHSlessEqual(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSlessEqual(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
      // greatEqual
    varType_ptr get_has_LHSgreatEqual(varType_ptr RHS_type) override;
    C_expression_ptr write_LHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_has_RHSgreatEqual(varType_ptr LHS_type) override;
    C_expression_ptr write_RHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override;




    /// ASSIGNMENT /// again just wraps over reference type, and dereferences LHS when appropriate.
    bool get_has_assignment(varType_ptr RHS_type) override;
    void write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false) override;

    bool get_has_assignTo(varType_ptr LHS_type) override;
    void write_assignTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false) override;

// TODO: auto!

    /// CLASS THINGS /// again, mimics underlying type with dereferencing
    /// members
    varType_ptr member_has_getter(csu::utf8_string& member_name) override;
    C_expression_ptr write_member_getter(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual=false) override ;

    varType_ptr member_has_getref(csu::utf8_string& member_name) override;
    C_expression_ptr write_member_getref(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual=false) override;

    varType_ptr set_member(csu::utf8_string& member_name, varType_ptr RHS_type=nullptr) override;

    void write_member_setter(C_expression_ptr LHS_expression, csu::utf8_string& member_name,
             C_expression_ptr RHS_expression, Csource_out_ptr output, bool call_virtual=false) override;

    /// CONSTRUCTORS ///
// this constructs in place. Assumes this already has memory!
    bool has_implicit_copy_constructor(varType_ptr RHS_type, cast_enum cast_behavior=cast_enum::implicit_casts) override;
    void write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior=cast_enum::implicit_casts) override;


    /// CASTING ///
// TODO: this is weird. Need actual casting!
    bool can_implicit_castTo(varType_ptr LHS_type) override;
    void write_implicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual=false) override;

    bool can_explicit_castTo(varType_ptr LHS_type) override;
    void write_explicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual=false) override;


    /// interactions with C ///
    varType_ptr can_toC();
    C_expression_ptr toC(C_expression_ptr exp, Csource_out_ptr output, bool call_virtual=false);
};
typedef std::shared_ptr<raw_C_pointer_reference> CPntrRef_ptr;




class function_argument_types
// defined where a function is called
{
public:
    std::vector< varType_ptr > unnamed_argument_types;
    std::vector< csu::utf8_string > named_argument_names;
    std::vector< varType_ptr > named_argument_types;

    varType_ptr type_from_index( int index );

    void print(std::ostream& output);
}; //pntr typ: function_argument_types_ptr

class func_parameter_info
// defined where a function is defined
{
public:
    std::vector<varName_ptr> required_parameters;
    std::vector<varName_ptr> optional_parameters;

    int total_size();
    varName_ptr parameter_from_index(int i);
    int index_from_name(csu::utf8_string& name);
    // returns -1 if name not in parameters.

    // if two functions with same names names in same scope have these parameters, could they be distinguished
    bool is_equivalent(func_param_ptr RHS);
    bool is_distinguishable(func_param_ptr other_parameters); // simply not is_equivalent. Hoepfully stays that simple, but don't assume


    void write_to_C(Csource_out_ptr output, bool write_void_for_zero_params=false);// does NOT include parenthesis.
    void print(std::ostream& output);
}; //pntr typ: func_param_ptr



class parameter_to_arguments_mapper
{

    void initialize();
public:
// remember: parameter is defined in function, argument defined by call

// set by constructor
    func_param_ptr parameters;
    function_argument_types_ptr arguments;
    cast_enum cast_mode;

// set by initialize() (private above. assumes constructor is set)

    //each value is the index of argument to use. -1 means use default parameters.
    // index is the index of the parameter
    // -2 means index is un-mapped. Should not have if is_good is true
    std::vector<int> param_to_arg_map;


    bool is_good;
    int num_defaults_used; // smaller is better
    int num_casts;// smaller is better. num_defaults_used is more important
    int num_pointer_casts; // smaller is better. num_casts is more important

    csu::utf8_string error; // set if is_good is false;

// set by write_arguments:
    //std::vector<C_expression_ptr> cleanups;


// now the public interface:
    // NOTE: explicit casting not allowed, will error!
    parameter_to_arguments_mapper(func_param_ptr _parameters, function_argument_types_ptr _arguments,
                                  cast_enum cast_behavior=cast_enum::implicit_casts);

    // returns 1 if this map is preferred. 0 if they are equal, -1 if other is prefered.
    // does not check is_good
    int comparison(parm_arg_map_ptr RHS);

    // this does all the conversions and whatnot to convert given cyth expressions to what is really needed to call function
    void write_arguments( std::vector<C_expression_ptr> &argument_Cexpressions,
                          std::vector<C_expression_ptr> &out_expressions, Csource_out_ptr output);

//void write_cleanup( Csource_out_ptr output ); // need to call this to call destructors
}; // smart pntr type: parm_arg_map_ptr


// TODO: replace this with fancy implicit concepts
//// JUST HERE FOR TESTS. WILL BE REMOVED ///
class staticFuncPntr : public varType
{
public:
    // assume, loc, and is_ordered, will be set
    func_param_ptr parameters; // assume set externally as well
    varType_ptr return_type; // set externally. assume always known from definition.

    staticFuncPntr();

    bool is_equivalent(varType_ptr RHS) override;
    //varType_ptr copy(csu::utf8_string _definition_name="" ) override;

    /// CALLING ///
    varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

    C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;


    /// ASSIGNMENT /// assign from other func-pointers with same signiture
    bool get_has_assignment(varType_ptr RHS_type) override;
    void write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false) override;

    varType_ptr get_auto_type() override;

    /// DEFINITION ///
    bool can_be_defined() override { return true; }
    void C_definition_name(csu::utf8_string& var_name, Csource_out_ptr output) override;

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

        void write_C_prototype(Csource_out_ptr output);
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
    bool is_equivalent(varType_ptr RHS){return false;}
    bool type_is_fully_defined() override { return all_overloads_defined; }

    /// CALLING ///
    resolved_pair get_resolution(function_argument_types_ptr argument_types, csu::utf8_string& error_msg, cast_enum cast_behavior);

    varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

    C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;



    /// DEFINITION ///
    bool can_be_defined() override {return false;}

    /// ADDITION /// cannot add functions ........ unfortunately...... would be cool though

    /// ASSIGNMENT /// cannot assign to defined functions
    varType_ptr get_auto_type() override; // but we MIGHT have an auto-type
    csu::utf8_string single_overload_Cname(); // used for writing to C
};
typedef std::shared_ptr<DefFuncType> DefFuncType_ptr;



//some rules:
//    constructors are never virtual or inhereted
//    assignment operators can be virtual, but not inheretited (this is no longer true I think..)
//    conversion operators are normal (can be inhereted and virtual).

class MethodType : public varType
{
private:

    class method_access_writer : public C_expression
    // this is needed, because methods are weird.
    // made by get_C_exp below
       // not a clue how referencing works here. Will assume not referencable.
    {
    public:
        C_expression_ptr parent_exp; // doesn't  own this ( unless added as cleanup child)
        bool virtual_call_allowed; // if false, use static call, it true, use virtual call if possible

        csu::utf8_string get_C_expression() {  return ""; }
    };


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
        assignTo,
        inform_moved,
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

        void write_C_prototype(Csource_out_ptr output);
    };
    typedef std::shared_ptr<resolved_method> ResolvedMethod_ptr;
    typedef std::pair< ResolvedMethod_ptr, std::shared_ptr<parameter_to_arguments_mapper> > resolved_pair;



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
    bool is_equivalent(varType_ptr RHS) override {return false;} // is this correct??
    bool type_is_fully_defined() override { return num_undefined_overloads==0; }

    /// CALLING ///
//    resolved_pair get_resolution(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg);
//    resolved_pair get_resolution(std::vector<expression_AST_node*> &argument_expressions, sym_table_base* _symbol_table, csu::utf8_string& error_msg);
//
//    varType_ptr is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg) override;
//
//    exp_writer_ptr write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
//       std::vector<csu::utf8_string>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false) override;
//
//     exp_writer_ptr write_call(ResolvedMethod_ptr resolved_method, std::vector<expression_AST_node*> &argument_expressions,
//               sym_table_base* _symbol_table, exp_writer_ptr LHS_Cexp, std::vector<csu::utf8_string>& argument_Cexpressions, Csource_out_ptr output);
//
//     exp_writer_ptr write_call(std::vector<expression_AST_node*> &argument_expressions, sym_table_base* _symbol_table,
//                    exp_writer_ptr LHS_Cexp, std::vector<csu::utf8_string>& argument_Cexpressions, Csource_out_ptr output);

    resolved_pair get_resolution(function_argument_types_ptr argument_types, csu::utf8_string& error_msg, cast_enum cast_behavior);

    varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

    // here LHS_exp should have this type (made by get_C_exp below)
    // note call_virtual is not used (uses info from LHS_Cexp instead)
    C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

    // shortcut if parent exp is known. Here call_virtual IS used!
    C_expression_ptr parental_write_call(function_argument_types_ptr argument_types, C_expression_ptr parent_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts);

    /// DEFINITION ///
    bool can_be_defined() override {return false;}


    /// helper functions
    ResolvedMethod_ptr get_indestinguishable_overload(func_param_ptr parameters);
    bool overload_was_inhereted(ResolvedMethod_ptr ovrld);

    // if own_exp is true, then C_expression_ptr will take charge of cleaning parent_exp
    C_expression_ptr get_C_exp(C_expression_ptr parent_exp, bool _virtual_call_allowed, bool own_exp=false); // note, return does NOT own parent_exp
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

    bool is_equivalent(varType_ptr RHS) override;



    /// DEFINITION ///
    bool can_be_defined() override { return true; }
    void C_definition_name(csu::utf8_string& var_name, Csource_out_ptr output) override;
    void initialize(csu::utf8_string& var_exp, Csource_out_ptr output) override;


    void inform_moved(csu::utf8_string& var_name, Csource_out_ptr output, bool call_virtual=false) override;
    varType_ptr get_auto_type() override;


    /// REFERENCING ///
    // by default, will allow getting own pointer
    bool can_get_pointer(varType_ptr output_type) override;// in general, should return true for equivlent type, at least
    // return C expression should be pointer to output_type. exp needs to be referancable c-expression to this type
    csu::utf8_string get_pointer(varType_ptr output_type, csu::utf8_string& exp, Csource_out_ptr output) override;


    /// CLASS THINGS ///

    //given a member name, find parent with that member. Doesn't worry about vtables.
    // parent_index set to -1 for this class, -2 if variable not found
    varName_ptr get_member_full_inheritance(int& parent_index, csu::utf8_string& member_name);


// note getter and getref are mostly the same
// in case of known method, just returns expression to self. Hopefully such expression is sent to method type BEFORE writing C.
// self is cast to the type of the class the defines the method (even if it isn't the one with the vtable entry).

    varType_ptr member_has_getref(csu::utf8_string& member_name) override;
    C_expression_ptr write_member_getref(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual=false) override;

    varType_ptr member_has_getter(csu::utf8_string& member_name) override;
    C_expression_ptr write_member_getter(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual=false) override;


    varType_ptr set_member(csu::utf8_string& member_name, varType_ptr RHS_type=nullptr) override;
    // virtual bit doesn't really work, as we don't have setters yet,
    void write_member_setter(C_expression_ptr LHS_expression, csu::utf8_string& member_name,
             C_expression_ptr RHS_expression, Csource_out_ptr output, bool call_virtual=false) override;



    /// CONSTRUCTORS ///
    void write_default_constructor(csu::utf8_string& var_name, Csource_out_ptr output) override;

    bool has_explicit_constructor(function_argument_types_ptr argument_types, csu::utf8_string& error_msg) override;
    void write_explicit_constructor(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
                        std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output) override;

    bool has_explicit_copy_constructor(varType_ptr RHS_type, csu::utf8_string& error_msg) override;
    void write_explicit_copy_constructor(C_expression_ptr LHS_exp,
                        C_expression_ptr RHS_exp, Csource_out_ptr output) override;

    bool has_implicit_copy_constructor(varType_ptr RHS_type, cast_enum cast_behavior=cast_enum::implicit_casts) override;
    void write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior=cast_enum::implicit_casts) override;


    /// DESTRUCTORS ///
    void write_destructor(csu::utf8_string& var_name, Csource_out_ptr output, bool call_virtual=false) override;


    /// ASSIGNMENT ///
    bool get_has_assignment(varType_ptr RHS_type) override;
    void write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false) override;

    bool get_has_assignTo(varType_ptr LHS_type) override;
    void write_assignTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual=false) override;



    /// CASTING ///
    bool can_implicit_castTo(varType_ptr LHS_type) override;
    void write_implicit_castTo( C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual=false) override;

    bool can_explicit_castTo(varType_ptr LHS_type) override;
    void write_explicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual=false) override;



    /// CALLING ///
    varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

    C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;



    /// BINARY OPERATORS ///
    varType_ptr get_has_binOperator(varType_ptr type, const char* op_func_name);
    C_expression_ptr write_binOperator(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                          Csource_out_ptr output, const char* op_func_name, bool call_virtual, bool is_LHS);

      // power
    varType_ptr get_has_LHSpower(varType_ptr RHS_type) override
        { return get_has_binOperator(RHS_type, "__lPow__"); }
    C_expression_ptr write_LHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__lPow__", call_virtual, true); }

    varType_ptr get_has_RHSpower(varType_ptr LHS_type) override
        { return get_has_binOperator(LHS_type, "__rPow__"); }
    C_expression_ptr write_RHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__rPow__", call_virtual, false); }


      // multiplication
    varType_ptr get_has_LHSmultiplication(varType_ptr RHS_type) override
        { return get_has_binOperator(RHS_type, "__lMul__"); }
    C_expression_ptr write_LHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__lMul__", call_virtual, true); }

    varType_ptr get_has_RHSmultiplication(varType_ptr LHS_type) override
        { return get_has_binOperator(LHS_type, "__rMul__"); }
    C_expression_ptr write_RHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__rMul__", call_virtual, false); }


      // division
    varType_ptr get_has_LHSdivision(varType_ptr RHS_type) override
        { return get_has_binOperator(RHS_type, "__lDiv__"); }
    C_expression_ptr write_LHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__lDiv__", call_virtual, true); }

    varType_ptr get_has_RHSdivision(varType_ptr LHS_type) override
        { return get_has_binOperator(LHS_type, "__rDiv__"); }
    C_expression_ptr write_RHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__rDiv__", call_virtual, false); }


      // modulus
    varType_ptr get_has_LHSmodulus(varType_ptr RHS_type) override
        { return get_has_binOperator(RHS_type, "__lMod__"); }
    C_expression_ptr write_LHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__lMod__", call_virtual, true); }

    varType_ptr get_has_RHSmodulus(varType_ptr LHS_type) override
        { return get_has_binOperator(LHS_type, "__rMod__"); }
    C_expression_ptr write_RHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__rMod__", call_virtual, false); }


      // addition
    varType_ptr get_has_LHSaddition(varType_ptr RHS_type) override
        { return get_has_binOperator(RHS_type, "__lAdd__"); }
    C_expression_ptr write_LHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__lAdd__", call_virtual, true); }

    varType_ptr get_has_RHSaddition(varType_ptr LHS_type) override
        { return get_has_binOperator(LHS_type, "__rAdd__"); }
    C_expression_ptr write_RHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__rAdd__", call_virtual, false); }


      // subtraction
    varType_ptr get_has_LHSsubtraction(varType_ptr RHS_type) override
        { return get_has_binOperator(RHS_type, "__lSub__"); }
    C_expression_ptr write_LHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__lSub__", call_virtual, true); }

    varType_ptr get_has_RHSsubtraction(varType_ptr LHS_type) override
        { return get_has_binOperator(LHS_type, "__rSub__"); }

    C_expression_ptr write_RHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual=false) override
        { return write_binOperator(LHS_exp, RHS_exp, output, "__rSub__", call_virtual, false); }

    /// COMPARISON OPERATORS ///
    varType_ptr get_has_cmp(varType_ptr type);
    C_expression_ptr write_compOperator(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                          Csource_out_ptr output, char operation, bool call_virtual, bool is_LHS); // operations: < , >, !, =, L, G respectively: lessthan, greaterthan, not equal, equal to,  <=, and >=

        // lessThan
    varType_ptr get_has_LHSlessThan(varType_ptr RHS_type) override
        { return get_has_cmp(RHS_type); }
    C_expression_ptr write_LHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '<', call_virtual, true);
        }

    varType_ptr get_has_RHSlessThan(varType_ptr LHS_type) override
        { return get_has_cmp(LHS_type); }
    C_expression_ptr write_RHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '<', call_virtual, false);
        }


            // greatThan
    virtual varType_ptr get_has_LHSgreatThan(varType_ptr RHS_type) override
        { return get_has_cmp(RHS_type); }
    virtual C_expression_ptr write_LHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '>', call_virtual, true);
        }

    virtual varType_ptr get_has_RHSgreatThan(varType_ptr LHS_type) override
        { return get_has_cmp(LHS_type); }
    virtual C_expression_ptr write_RHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '>', call_virtual, false);
        }

            // equalTo
    virtual varType_ptr get_has_LHSequalTo(varType_ptr RHS_type) override
        { return get_has_cmp(RHS_type); }
    virtual C_expression_ptr write_LHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '=', call_virtual, true);
        }

    virtual varType_ptr get_has_RHSequalTo(varType_ptr LHS_type) override
        { return get_has_cmp(LHS_type); }
    virtual C_expression_ptr write_RHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '=', call_virtual, false);
        }

            // notEqual
    virtual varType_ptr get_has_LHSnotEqual(varType_ptr RHS_type) override
        { return get_has_cmp(RHS_type); }
    virtual C_expression_ptr write_LHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '!', call_virtual, true);
        }

    virtual varType_ptr get_has_RHSnotEqual(varType_ptr LHS_type) override
        { return get_has_cmp(LHS_type); }
    virtual C_expression_ptr write_RHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, '!', call_virtual, false);
        }

            // lessEqual
    virtual varType_ptr get_has_LHSlessEqual(varType_ptr RHS_type) override
        { return get_has_cmp(RHS_type); }
    virtual C_expression_ptr write_LHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, 'L', call_virtual, true);
        }

    virtual varType_ptr get_has_RHSlessEqual(varType_ptr LHS_type) override
        { return get_has_cmp(LHS_type); }
    virtual C_expression_ptr write_RHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, 'L', call_virtual, false);
        }

            // greatEqual
    virtual varType_ptr get_has_LHSgreatEqual(varType_ptr RHS_type) override
        { return get_has_cmp(RHS_type); }
    virtual C_expression_ptr write_LHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, 'G', call_virtual, true);
        }

    virtual varType_ptr get_has_RHSgreatEqual(varType_ptr LHS_type) override
        { return get_has_cmp(LHS_type); }
    virtual C_expression_ptr write_RHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual=false) override
        {
            return write_compOperator(LHS_exp, RHS_exp, output, 'G', call_virtual, false);
        }




/// C-interface
    varType_ptr can_toC() override;
    C_expression_ptr toC(C_expression_ptr exp, Csource_out_ptr output, bool call_virtual=false) override;




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
    C_expression_ptr write_parent_access(int parent_index, C_expression_ptr exp, Csource_out_ptr output);   // note, return isn't cleaned up in DefClassType::get_pointer !!
};


class MetaType : public varType
// a variable with the name of a type, has this type
// eventually this should be a global object. This is where static functions and variables should be defined
{
public:
    varType_ptr type;

    MetaType(varType_ptr _type);

    bool is_equivalent(varType_ptr RHS) override; // if is metatype and "type" is the same

    // note that type cannot be defined
    // .... or do much at all at this point .....

    // BUT!! it IS callable!!
    // this explicitly constructs new variable of type and "returns" it
    varType_ptr is_callable(function_argument_types_ptr argument_types, csu::utf8_string& error_msg,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

    // for now, LHS_Cexp is not used
    C_expression_ptr write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual=false,
                                    cast_enum cast_behavior=cast_enum::implicit_casts) override;

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

    virtual void add_variable(varName_ptr new_var, bool& is_exclusive, csu::utf8_string& alias);
    virtual void add_type(varType_ptr new_type, bool& is_exclusive, csu::utf8_string& alias);

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
