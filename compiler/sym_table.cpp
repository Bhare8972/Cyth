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
#include <sstream>
#include "sym_table.hpp"
#include "cyth_AST.hpp"
#include "writeAST_to_C.hpp"

using namespace csu;
using namespace std;



/////// types /////////

// varType (BASE)

/// some basics ///
ClassType_ptr varType::as_class()
{
    if( not (type_of_type == varType::defined_class_t) )
    {
        return nullptr;
    }
    else
    {
        return dynamic_pointer_cast<DefClassType>( shared_from_this() );
    }
}
MethodType_ptr varType::as_method()
{
    if( not (type_of_type == varType::method_function_t) )
    {
        return nullptr;
    }
    else
    {
        return dynamic_pointer_cast<MethodType>( shared_from_this() );
    }
}
MetaType_ptr varType::as_metatype()
{
    if( not (type_of_type == varType::metatype_t) )
    {
        return nullptr;
    }
    else
    {
        return dynamic_pointer_cast<MetaType>( shared_from_this() );
    }
}

/// DEFINITION ///
bool varType::can_be_defined() // Returns false if Ctype is unknown
{
    return not (C_name == "");
}

bool varType::is_equivalent(varType_ptr RHS)
{
    return shared_from_this() == RHS;
}

// define new variable of this type.
void varType::C_definition_name(utf8_string& var_name, Csource_out_ptr output)
{
    if( can_be_defined() )
    {
        output->out_strm() << C_name << " " << var_name ;
    }
}

/// CALLING ///
varType_ptr varType::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg, cast_enum cast_behavior)
{
    error_msg = "type " + definition_name + " is not callable";
    return nullptr;
}

C_expression_ptr varType::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                    cast_enum cast_behavior)
{
    throw gen_exception("ERROR: TYPE IS NOT CALLABLE THIS SHOULD NOT BE REACHED. varType::write_call");
    return nullptr;
}

/// BINARY OPERATORS ///
// power
C_expression_ptr varType::write_LHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have power ");
}
C_expression_ptr varType::write_RHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have power ");
}
// multiplication
C_expression_ptr varType::write_LHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have multiplication ");
}
C_expression_ptr varType::write_RHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have multiplication ");
}
// division
C_expression_ptr varType::write_LHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have division ");
}
C_expression_ptr varType::write_RHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have division ");
}
// modulus
C_expression_ptr varType::write_LHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have modulus ");
}
C_expression_ptr varType::write_RHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have modulus ");
}
// addition
C_expression_ptr varType::write_LHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have addition ");
}
C_expression_ptr varType::write_RHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have addition ");
}
// subtraction
C_expression_ptr varType::write_LHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have subtraction ");
}
C_expression_ptr varType::write_RHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have subtraction ");
}
// lessThan
C_expression_ptr varType::write_LHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have lessThan ");
}
C_expression_ptr varType::write_RHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have lessThan ");
}
// greatThan
C_expression_ptr varType::write_LHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have greatThan ");
}
C_expression_ptr varType::write_RHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have greatThan ");
}
// equalTo
C_expression_ptr varType::write_LHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have equalTo ");
}
C_expression_ptr varType::write_RHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have equalTo ");
}
// notEqual
C_expression_ptr varType::write_LHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have notEqual ");
}
C_expression_ptr varType::write_RHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have notEqual ");
}
// lessEqual
C_expression_ptr varType::write_LHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have lessEqual ");
}
C_expression_ptr varType::write_RHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have lessEqual ");
}
// greatEqual
C_expression_ptr varType::write_LHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have greatEqual ");
}
C_expression_ptr varType::write_RHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have greatEqual ");
}


/// ASSIGNMENT ///
void varType::write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have assignment ");
}


void varType::write_assignTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have assign-to ");
}



/// REFERENCE ///
varType_ptr varType::is_reference_type(bool &is_ref)
{
    is_ref = false;
    return nullptr;
}

void varType::take_reference(C_expression_ptr LHS_exp, C_expression_ptr referencable_RHS_exp, Csource_out_ptr output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " is not a reference type. In varType::take_reference, should not be reached. ");
}

bool varType::can_get_pointer(varType_ptr output_type)
{
    return is_equivalent( output_type );
}

utf8_string varType::get_pointer(varType_ptr output_type, csu::utf8_string& exp, Csource_out_ptr output)
{
    if(is_equivalent( output_type ) )
    {
        return "&(" + exp + ")";
    }
    else
    {
        throw gen_exception("ERROR: TYPE ", definition_name, " cannot be referenced as ", output_type->definition_name,
                            ". In varType::get_pointer, should not be reached." );
    }
}


/// CLASS THINGS ///

C_expression_ptr varType::write_member_getter(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have member access ");
    return nullptr;
}

C_expression_ptr varType::write_member_getref(C_expression_ptr LHS_exp, csu::utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have member access ");
    return nullptr;
}

/// CONSTRUCTORS ///

bool varType::has_explicit_constructor(function_argument_types_ptr argument_types, csu::utf8_string& error_msg)
{
    error_msg = "type "+definition_name+" cannot be explicitly constructed";
    return false;
}

void varType::write_explicit_constructor(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
                        std::vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " cannot be explicitly constructed");
}

bool varType::has_explicit_copy_constructor(varType_ptr RHS_type, csu::utf8_string& error_msg)
{
    error_msg = "type "+definition_name+" cannot be explicitly constructed";
    return false;
}

void varType::write_explicit_copy_constructor( C_expression_ptr LHS_exp,
                        C_expression_ptr RHS_exp, Csource_out_ptr output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " cannot be explicitly constructed");
}

void varType::write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " is not copyable");
}

C_expression_ptr varType::toC(C_expression_ptr exp, Csource_out_ptr output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " has no C-type. This should not be reached");
}




// C import

varType_fromC::varType_fromC()
{
    type_of_type = varType::c_import_t;
}
void varType_fromC::set_pointers(std::weak_ptr<varType> _unnamed_C_type)
{
    unnamed_C_type = _unnamed_C_type;
}

bool varType_fromC::is_equivalent(varType_ptr RHS)
{
    if( RHS->type_of_type == varType::c_import_t )
    {
        if( C_name=="" or RHS->C_name=="")
        {
            return false;
        }
        else if (C_name == RHS->C_name)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

varType_ptr varType_fromC::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg, cast_enum cast_behavior)
{
    if( argument_types->named_argument_names.size() != 0 )
    {
        stringstream TMP;
        TMP << "C-function cannot take named arguments." << endl;
        error_msg = TMP.str();
        return nullptr;
    }

    int num_unnamed_args = argument_types->unnamed_argument_types.size();
    for( int i=0; i<num_unnamed_args; i++ )
    {
        auto &typ = argument_types->unnamed_argument_types[ i ];
        if( not typ->can_toC() )
        {
            stringstream TMP;
            TMP << "type " << typ->definition_name << " cannot be passed to C-function. Cannot be converted to a c-type" << endl;
            error_msg = TMP.str();
            return nullptr;
        }
    }

    return unnamed_C_type.lock();
}

C_expression_ptr varType_fromC::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                    cast_enum cast_behavior)
{

    stringstream out;
    out <<  '(' << ( LHS_Cexp->get_C_expression() ) << '(';

    bool do_comma = false;
    int num_args = argument_Cexpressions.size();
    list< std::shared_ptr<C_expression> > child_expressions;
    for(int arg_i=0; arg_i<num_args; arg_i++)
    {
        if( do_comma )
        {
            out << ',';
        }
        else
        {
            do_comma = true;
        }

        auto &arg_type = argument_types->unnamed_argument_types[ arg_i ];
        auto &arg_exp = argument_Cexpressions[ arg_i ];

        auto new_writer = arg_type->toC( arg_exp, output ); // call virtual defaults to false??
        out << new_writer->get_C_expression();
        //ret_exp_writer->add_cleanup_child(new_writer);
        child_expressions.push_back( new_writer );
    }
    out << "))";



    auto ret_exp_writer = make_shared<simple_C_expression>(out.str(), unnamed_C_type.lock(),
                                                           false, false);
    for( auto& W : child_expressions )
    {
        ret_exp_writer->add_cleanup_child( W );
    }


    return ret_exp_writer;
}


/// BINARY OPERATORS ///
varType_ptr varType_fromC::has_binOperator(varType_ptr RHS_type)
{
    if( RHS_type->type_of_type == varType::c_import_t )
        return unnamed_C_type.lock();
    else
    {
        bool is_ref;
        auto ref_type = RHS_type->is_reference_type( is_ref );
        if( is_ref and ref_type and ref_type->type_of_type == varType::c_import_t)
        {
            return unnamed_C_type.lock();
        }
        return nullptr;
    }
}

C_expression_ptr varType_fromC::write_LHSoperator(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, const char* op)
{
    stringstream ret;
    auto &RHS_type = RHS_exp->cyth_type;

    if( RHS_type->type_of_type == varType::c_import_t )
    {
        ret << '(' << LHS_exp->get_C_expression() << op << RHS_exp->get_C_expression() << ')';
    }
    else
    { // assume is ref to str type

        bool is_ref;
        auto ref_type = RHS_type->is_reference_type( is_ref );

        auto tmp = RHS_exp->get_C_expression();
        auto pntr_str = RHS_type->get_pointer(ref_type, tmp, output);
        ret << '(' << LHS_exp->get_C_expression()  << op <<" (*" << pntr_str << "))";
    }

    return make_shared<simple_C_expression>( ret.str(), unnamed_C_type.lock(), false, true );
}

C_expression_ptr varType_fromC::write_RHSoperator(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, const char* op)
{
    stringstream ret;
    auto &LHS_type = LHS_exp->cyth_type;

    if( LHS_type->type_of_type == varType::c_import_t )
    {
        ret << '(' << LHS_exp->get_C_expression() << op << RHS_exp->get_C_expression() << ')';
    }
    else
    { // assume is ref to str type

        bool is_ref;
        auto ref_type = LHS_type->is_reference_type( is_ref );

        auto tmp = LHS_exp->get_C_expression();
        auto pntr_str = LHS_type->get_pointer(ref_type, tmp, output);
        ret << '(' <<  " (*" << pntr_str << "))" << op << RHS_exp->get_C_expression();
    }

    return make_shared<simple_C_expression>( ret.str(), unnamed_C_type.lock(), false, true );
}



/// ASSIGNMENT ///
bool varType_fromC::get_has_assignment(varType_ptr RHS_type)
{

    if( RHS_type->type_of_type == varType::c_import_t )
        return true;
    else
    {
        bool is_ref;
        auto ref_type = RHS_type->is_reference_type( is_ref );
        if( is_ref and ref_type and ref_type->type_of_type == varType::c_import_t)
        {
            return true;
        }
        return false;
    }
}

void varType_fromC::write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
               Csource_out_ptr output, bool call_virtual)
{
    //output << LHS << '=' << RHS_exp <<';';

    auto RHS_type = RHS_exp->cyth_type ;

    if( RHS_type->type_of_type == varType::c_import_t )
    {
        output->out_strm() << output->ln_strt << LHS_exp->get_C_expression() << '=' << RHS_exp->get_C_expression() <<';';
    }
    else
    { // assume is ref to str type

        bool is_ref;
        auto ref_type = RHS_type->is_reference_type( is_ref );

        auto tmp = RHS_exp->get_C_expression();
        auto pntr_str = RHS_type->get_pointer(ref_type, tmp, output);
        output->out_strm() << output->ln_strt << LHS_exp->get_C_expression() << '=' << " (*" << pntr_str << ");";
    }

}


/// CONSTRUCTION ///
bool varType_fromC::has_explicit_constructor(function_argument_types_ptr argument_types, utf8_string& error_msg)
{
    if( argument_types->unnamed_argument_types.size()!=1  or  argument_types->named_argument_names.size() != 0 )
    {
        stringstream TMP;
        TMP << "C-type ( " <<  definition_name << ":" << C_name << " ) can only be constructed with one un-named argument";
        error_msg = TMP.str();
        return false;
    }

    auto type = argument_types->unnamed_argument_types[0];
    if( type->type_of_type != varType::c_import_t )
    {
        stringstream TMP;
        TMP << "can only assign to a C-type from another C-type" << endl;
        error_msg = TMP.str();
        return false;
    }
    return true;
}


void varType_fromC::write_explicit_constructor(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
                        vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output)
{
    output->out_strm() << output->ln_strt << LHS_Cexp->get_C_expression() << '=' << argument_Cexpressions[0]->get_C_expression() << ';' << endl;
}


bool varType_fromC::has_explicit_copy_constructor(varType_ptr RHS_type, utf8_string& error_msg)
{
    if( RHS_type->type_of_type != varType::c_import_t )
    {
        stringstream TMP;
        TMP << "can only assign to a C-type from another C-type" << endl;
        error_msg = TMP.str();
        return false;
    }
    return true;
}


void varType_fromC::write_explicit_copy_constructor(C_expression_ptr LHS_exp,
                        C_expression_ptr RHS_exp, Csource_out_ptr output)
{
    output->out_strm() << output->ln_strt << LHS_exp->get_C_expression() << '=' << RHS_exp->get_C_expression() << ';' << endl;
}


bool varType_fromC::has_implicit_copy_constructor(varType_ptr RHS_type, cast_enum cast_behavior)
{
    bool is_ref;
    auto ref_type = RHS_type->is_reference_type( is_ref );
    if( is_ref and ref_type )
    {
        RHS_type = ref_type;
    }

    return (RHS_type->type_of_type==varType::c_import_t) and (C_name==RHS_type->C_name) ;

}

void varType_fromC::write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior)
{
    auto RHS_type = RHS_exp->cyth_type;

    if( RHS_type->type_of_type == varType::c_import_t )
    {
        output->out_strm() << output->ln_strt << LHS->get_C_expression() << '=' << RHS_exp->get_C_expression() <<';';
    }
    else
    { // assume is ref to same type

        bool is_ref;
        auto ref_type = RHS_type->is_reference_type( is_ref );

        auto TMP = RHS_exp->get_C_expression();
        auto pntr_str = RHS_type->get_pointer(ref_type, TMP, output);
        output->out_strm() << output->ln_strt << LHS->get_C_expression() << '=' << " (*" << pntr_str << ");";
    }
}


// raw c-pointer references
raw_C_pointer_reference::raw_C_pointer_reference(varType_ptr _ref_type)
{
    referenced_type = _ref_type;
    type_of_type = varType::c_pointer_reference;
    definition_name = "*" + _ref_type->definition_name;
    C_name = _ref_type->C_name;
    loc = _ref_type->loc;
    is_ordered = false;

    assignment_mode = raw_C_pointer_reference::no_assignment;
    //construct_mode = raw_C_pointer_reference::construct_only_reference;
}

raw_C_pointer_reference::raw_C_pointer_reference(varType_ptr _ref_type, assignment_mode_t _assignment_node)
{
    referenced_type = _ref_type;
    type_of_type = varType::c_pointer_reference;
    definition_name = "*" + _ref_type->definition_name;
    C_name = _ref_type->C_name;
    loc = _ref_type->loc;
    is_ordered = false;

    assignment_mode = _assignment_node;
    //construct_mode = raw_C_pointer_reference::construct_only_reference;
}

//raw_C_pointer_reference::raw_C_pointer_reference(varType_ptr _ref_type, assignment_mode_t _assignment_node, construct_mode_t _construct_mode)
//{
//    referenced_type = _ref_type;
//    type_of_type = varType::c_pointer_reference;
//    definition_name = "*" + _ref_type->definition_name;
//    C_name = _ref_type->C_name;
//    loc = _ref_type->loc;
//    is_ordered = false;
//
//    assignment_mode = _assignment_node;
//    construct_mode = _construct_mode;
//}

C_expression_ptr raw_C_pointer_reference::deref(C_expression_ptr exp)
{
    utf8_string new_C_exp = "(*" + exp->get_C_expression() + ")";
    return make_shared<simple_C_expression>( new_C_exp, shared_from_this(), true, false );
}

bool raw_C_pointer_reference::is_equivalent(varType_ptr RHS)
{
    if( RHS->type_of_type == varType::c_pointer_reference )
    {
        auto RHS_ptr = dynamic_pointer_cast<raw_C_pointer_reference>( RHS );
        return referenced_type->is_equivalent( RHS_ptr->referenced_type );
    }
    else
    {
        return false;
    }
}

/// DEFINITION

void raw_C_pointer_reference::C_definition_name(utf8_string& var_name, Csource_out_ptr output)
{
    utf8_string new_var_name = "*" + var_name;
    referenced_type->C_definition_name(new_var_name, output);
}

/// CALLING
varType_ptr raw_C_pointer_reference::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg,
                                cast_enum cast_behavior)
{
    return referenced_type->is_callable(argument_types, error_msg, cast_behavior);
}


C_expression_ptr raw_C_pointer_reference::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
        vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                cast_enum cast_behavior)
{
    return referenced_type->write_call( argument_types, deref(LHS_Cexp), argument_Cexpressions, output, true, cast_behavior );
}






/// BINARY OPERATORS

// power
varType_ptr raw_C_pointer_reference::get_has_LHSpower(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSpower( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSpower(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSpower(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSpower( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSpower(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSpower(LHS_exp, deref(RHS_exp), output, true); }

// multiplication
varType_ptr raw_C_pointer_reference::get_has_LHSmultiplication(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSmultiplication( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSmultiplication(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSmultiplication(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSmultiplication( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSmultiplication(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSmultiplication(LHS_exp, deref(RHS_exp), output, true); }

// division
varType_ptr raw_C_pointer_reference::get_has_LHSdivision(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSdivision( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSdivision(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSdivision(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSdivision( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSdivision(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSdivision(LHS_exp, deref(RHS_exp), output, true); }

// modulus
varType_ptr raw_C_pointer_reference::get_has_LHSmodulus(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSmodulus( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSmodulus(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSmodulus(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSmodulus( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSmodulus(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSmodulus(LHS_exp, deref(RHS_exp), output, true); }

// addition
varType_ptr raw_C_pointer_reference::get_has_LHSaddition(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSaddition( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSaddition(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSaddition(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSaddition( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSaddition(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSaddition(LHS_exp, deref(RHS_exp), output, true); }

// subtraction
varType_ptr raw_C_pointer_reference::get_has_LHSsubtraction(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSsubtraction( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSsubtraction(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSsubtraction(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSsubtraction( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSsubtraction(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSsubtraction(LHS_exp, deref(RHS_exp), output, true); }

// lessThan
varType_ptr raw_C_pointer_reference::get_has_LHSlessThan(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSlessThan( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSlessThan(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSlessThan(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSlessThan( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSlessThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSlessThan(LHS_exp, deref(RHS_exp), output, true); }

// greatThan
varType_ptr raw_C_pointer_reference::get_has_LHSgreatThan(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSgreatThan( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSgreatThan(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSgreatThan(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSgreatThan( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSgreatThan(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSgreatThan(LHS_exp, deref(RHS_exp), output, true); }

// equalTo
varType_ptr raw_C_pointer_reference::get_has_LHSequalTo(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSequalTo( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSequalTo(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSequalTo(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSequalTo( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSequalTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSequalTo(LHS_exp, deref(RHS_exp), output, true); }

// notEqual
varType_ptr raw_C_pointer_reference::get_has_LHSnotEqual(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSnotEqual( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSnotEqual(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSnotEqual(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSnotEqual( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSnotEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSnotEqual(LHS_exp, deref(RHS_exp), output, true); }

// lessEqual
varType_ptr raw_C_pointer_reference::get_has_LHSlessEqual(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSlessEqual( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSlessEqual(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSlessEqual(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSlessEqual( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSlessEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSlessEqual(LHS_exp, deref(RHS_exp), output, true); }

// greatEqual
varType_ptr raw_C_pointer_reference::get_has_LHSgreatEqual(varType_ptr RHS_type)
{ return referenced_type->get_has_LHSgreatEqual( RHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_LHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_LHSgreatEqual(deref(LHS_exp), RHS_exp, output, true); }

varType_ptr raw_C_pointer_reference::get_has_RHSgreatEqual(varType_ptr LHS_type)
{ return referenced_type->get_has_RHSgreatEqual( LHS_type ); }
C_expression_ptr raw_C_pointer_reference::write_RHSgreatEqual(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                              Csource_out_ptr output, bool call_virtual)
{ return referenced_type->write_RHSgreatEqual(LHS_exp, deref(RHS_exp), output, true); }




/// ASSIGNMENT
bool raw_C_pointer_reference::get_has_assignment(varType_ptr RHS_type)
{
    if( assignment_mode == raw_C_pointer_reference::no_assignment )
    {
        return false;
    }
    else if( assignment_mode == raw_C_pointer_reference::only_reference_assignment ) // include polymorphism!
    {
        //return is_equivalent( RHS_type );
        bool is_ref = false;
        RHS_type->is_reference_type( is_ref );
        return is_ref and RHS_type->can_get_pointer( referenced_type );
    }
    else if(  assignment_mode == raw_C_pointer_reference::take_reference_assignment ) // include polymorphism!
    {
        //return is_equivalent( RHS_type ) or referenced_type->is_equivalent( RHS_type );
        return RHS_type->can_get_pointer( referenced_type );
    }
    else if(  assignment_mode == raw_C_pointer_reference::full_assign_assignment )
    {
        return referenced_type->get_has_assignment(RHS_type);

    }
    else
    {
        throw gen_exception("Error in raw_C_pointer_reference::get_has_assignment. Should not be reached." );
    }
}

//void raw_C_pointer_reference::write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node, utf8_string& LHS,
 //                                   utf8_string& RHS_exp, ostream& output, bool call_virtual)

void raw_C_pointer_reference::write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual)
{
    if( assignment_mode == raw_C_pointer_reference::no_assignment )
    {
        throw gen_exception("Error 1 in raw_C_pointer_reference::write_assignment. Should not be reached." );
    }
//    else if( assignment_mode == raw_C_pointer_reference::only_reference_assignment )
//    {
//        output << LHS << '=' << RHS_exp <<';';
//    }
//    else if(  assignment_mode == raw_C_pointer_reference::take_reference_assignment )
//    {
//        if( is_equivalent( RHS_type ) )
//        {
//            output << LHS << '=' << RHS_exp <<';';
//        }
//        else if( referenced_type->is_equivalent( RHS_type ) )
//        {
////
//            if( not RHS_AST_node->c_exp_can_be_referenced )
//            {
//                throw gen_exception("Expression cannot be referenced at ", RHS_AST_node->loc );
//            }
//
//            output << LHS << '=' << "&(" << RHS_exp << ");";
//        }
//        else
//        {
//            throw gen_exception("Error 2 in raw_C_pointer_reference::write_assignment. Should not be reached." );
//        }
//    }

    else if( assignment_mode == raw_C_pointer_reference::only_reference_assignment or assignment_mode == raw_C_pointer_reference::take_reference_assignment )
    {
        auto RHS_type = RHS_exp->cyth_type;
        bool RHS_is_ref = false;
        RHS_type->is_reference_type( RHS_is_ref );

        if( RHS_is_ref or RHS_exp->can_be_referenced)
        {
            auto RHS_C_exp = RHS_exp->get_C_expression();
            utf8_string pntr_expression = RHS_type->get_pointer( referenced_type, RHS_C_exp, output );
            (output->out_strm()) << (output->ln_strt) << ( LHS_exp->get_C_expression()) << '=' << "(" << pntr_expression << ");";
        }
        else
        {
            throw gen_exception("Expression cannot be referenced");
        }
    }

    else if( assignment_mode == raw_C_pointer_reference::full_assign_assignment )
    {
        referenced_type->write_assignment(deref(LHS_exp), RHS_exp, output, true);
    }
}

bool raw_C_pointer_reference::get_has_assignTo(varType_ptr LHS_type)
{
    return referenced_type->get_has_assignTo( LHS_type );
}


void raw_C_pointer_reference::write_assignTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual)
{
    referenced_type->write_assignTo(LHS_exp, deref(RHS_exp), output, true);
}


/// CLASS members

varType_ptr raw_C_pointer_reference::member_has_getter(utf8_string& member_name)
{
    return referenced_type->member_has_getter( member_name );
}

C_expression_ptr raw_C_pointer_reference::write_member_getter(C_expression_ptr LHS_exp, utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual)
{
    return referenced_type->write_member_getter( deref(LHS_exp), member_name, output, true);
}

varType_ptr raw_C_pointer_reference::member_has_getref(utf8_string& member_name)
{
    return referenced_type->member_has_getref( member_name );
}

C_expression_ptr raw_C_pointer_reference::write_member_getref(C_expression_ptr LHS_exp, utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual)
{
    return referenced_type->write_member_getref( deref(LHS_exp), member_name, output, true);
}

varType_ptr raw_C_pointer_reference::set_member(csu::utf8_string& member_name, varType_ptr RHS_type)
{
    return referenced_type->set_member(member_name, RHS_type);
}

void raw_C_pointer_reference::write_member_setter(C_expression_ptr LHS_expression, csu::utf8_string& member_name,
             C_expression_ptr RHS_expression, Csource_out_ptr output, bool call_virtual)
{
    referenced_type->write_member_setter( deref(LHS_expression), member_name, RHS_expression, output, true);
}


/// CASTING ///

bool raw_C_pointer_reference::can_implicit_castTo(varType_ptr LHS_type)
{
    return referenced_type->can_implicit_castTo( LHS_type );

    // this duplicity is because construction won't work as it should for references, thus will default here
    // so we want to try copc_construction first
    //return LHS_type->has_implicit_copy_constructor( referenced_type.get() ) or referenced_type->can_implicit_castTo( LHS_type );
}

void raw_C_pointer_reference::write_implicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output,
                                            bool call_virtual)
{

    referenced_type->write_implicit_castTo(LHS_exp, deref(RHS_exp), output, true);


//    if( LHS_type->has_implicit_copy_constructor( referenced_type.get() ) )
//    {
//        LHS_type->write_implicit_copy_constructor(referenced_type.get(), RHS_AST_node, LHS, newRHS, output);
//    }
//    else
//    {
//        referenced_type->write_implicit_castTo(LHS_type, RHS_AST_node, LHS, newRHS, output, true);
//    }
}


bool raw_C_pointer_reference::can_explicit_castTo(varType_ptr LHS_type)
{
// and so it comes to this
    if( can_implicit_castTo(LHS_type) )
    {
        return true;
    }
    else
    {
        // explcit constructor??
        return referenced_type->can_explicit_castTo( LHS_type );
    }
}

void raw_C_pointer_reference::write_explicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual)

{
    if( can_implicit_castTo(LHS_exp->cyth_type)  )
    {
        write_implicit_castTo(LHS_exp, RHS_exp, output, true);
    }

    referenced_type->write_explicit_castTo(LHS_exp, deref(RHS_exp), output, true);
}

 /// Reference stuff ///
varType_ptr raw_C_pointer_reference::is_reference_type(bool &is_ref)
{
    is_ref = true;
    return referenced_type;
}

bool raw_C_pointer_reference::can_take_reference(varType_ptr RHS_type)
{
    //return RHS_type->can_get_pointer( referenced_type.get() );
    return referenced_type->is_equivalent( RHS_type );
}

void raw_C_pointer_reference::take_reference(C_expression_ptr LHS_exp, C_expression_ptr referencable_RHS_exp, Csource_out_ptr output)
{
    auto RHS_type = referencable_RHS_exp->cyth_type;
    auto RHS_Cexp = referencable_RHS_exp->get_C_expression();

    auto C_ptr_str = RHS_type->get_pointer(referenced_type, RHS_Cexp, output);

    auto LHS_Cexp = LHS_exp->get_C_expression();
    (output->out_strm()) << (output->ln_strt) << LHS_Cexp << "=" << C_ptr_str << ";" << endl;
}

bool raw_C_pointer_reference::can_get_pointer(varType_ptr output_type)
{
    if( is_equivalent( output_type ) or referenced_type->can_get_pointer(output_type))
    {
        return true;
    }
    else
    {
        return false;
    }
}

utf8_string raw_C_pointer_reference::get_pointer(varType_ptr output_type, utf8_string& exp, Csource_out_ptr output)
{
    if( is_equivalent( output_type ) )
    {
        return "&(" + exp +")";
    }
    else if( referenced_type->is_equivalent( output_type ) )
    {
        return exp;
    }
    else if( referenced_type->can_get_pointer(output_type) )
    {
        utf8_string __exp = "*(" + exp +")";
        return referenced_type->get_pointer(output_type, __exp, output) ;
    }
    else
    {
        throw gen_exception("cannot take reference of type ", output_type->definition_name, " to type ", definition_name,
            ". This should not be reached in raw_C_pointer_reference::get_pointer" );
    }
}

/// CONSTRUCTORS ///
bool raw_C_pointer_reference::has_implicit_copy_constructor(varType_ptr RHS_type, cast_enum cast_behavior)
{
    //return is_equivalent( RHS_type ) or referenced_type->is_equivalent( RHS_type );
    return referenced_type->has_implicit_copy_constructor(RHS_type, cast_behavior);
}

void raw_C_pointer_reference::write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior)
{
    referenced_type->write_implicit_copy_constructor(deref(LHS), RHS_exp, output, cast_behavior);
}

varType_ptr raw_C_pointer_reference::can_toC()
{
    return referenced_type->can_toC();
}

C_expression_ptr raw_C_pointer_reference::toC(C_expression_ptr exp, Csource_out_ptr output, bool call_virtual)
{
    return referenced_type->toC( deref(exp),  output,  true);
}



/// function arguments ////
varType_ptr function_argument_types::type_from_index( int index )
{
    if( index >= unnamed_argument_types.size() )
    {
        index -= unnamed_argument_types.size();
        if( index >= named_argument_names.size() )
        {
            throw gen_exception("index in function_argument_types::type_from_index is too large" );
        }
        else
        {
            return named_argument_types[ index ];
        }
    }
    else
    {
        return unnamed_argument_types[index];
    }
}

void function_argument_types::print(ostream& output)
{
    bool do_comma = false;

    output <<"( ";

    for( auto typ : unnamed_argument_types )
    {
        if(do_comma)
        {
            output << ", " ;
        }
        else
        {
            do_comma = true;
        }

        output << typ->definition_name ;
    }


    int num = named_argument_names.size();
    for( int i=0; i<num; ++i )
    {

        if(do_comma)
        {
            output << ", " ;
        }
        else
        {
            do_comma = true;
        }


        output << named_argument_names[i] << '=' << named_argument_types[i]->definition_name;
    }

    output <<")";
}


/// function parameters ///
int func_parameter_info::total_size()
{
    return required_parameters.size() + optional_parameters.size();
}

varName_ptr func_parameter_info::parameter_from_index(int i)
{
    if( i<required_parameters.size() )
    {
        return required_parameters[i];
    }
    else if( i< total_size() )
    {
        return optional_parameters[ i-required_parameters.size() ];
    }
    else
    {
        throw gen_exception("index is too large in func_parameter_info::parameter_from_index");
    }
}

bool func_parameter_info::is_equivalent(func_param_ptr RHS)
{
    if( (required_parameters.size()!=RHS->required_parameters.size()) or (optional_parameters.size()!=RHS->optional_parameters.size())  )
    {
        return false;
    }
    else
    {
        for(int param_i=0; param_i<total_size(); param_i++)
        {
            auto thisT =       parameter_from_index(param_i)->var_type;
            auto otherT = RHS->parameter_from_index(param_i)->var_type;
            if( not thisT->is_equivalent( otherT ) )
            {
                return false;
            }
        }
        return true;
    }
}

bool func_parameter_info::is_distinguishable(func_param_ptr other_parameters)
{
    return not is_equivalent( other_parameters );

}

int func_parameter_info::index_from_name(csu::utf8_string& name)
{
    for(int param_i=0; param_i<total_size(); param_i++)
    {
        auto var = parameter_from_index( param_i );
        if( var->definition_name == name )
        {
            return param_i;
        }
    }
    return -1;
}

void func_parameter_info::write_to_C(Csource_out_ptr output, bool write_void_for_zero_params)
{
   // output << '(';



    if( total_size()==0  and write_void_for_zero_params)
    {
        (output->out_strm()) << (output->ln_strt) << "void";
    }
    else
    {
        // do required parameters
        int req_param_i=0;
        for(; req_param_i<required_parameters.size(); req_param_i++ )
        {
            if( req_param_i !=0 )
            {
                (output->out_strm()) << (output->ln_strt) << ',';
            }

            auto paramname = required_parameters[ req_param_i ];
            paramname->var_type->C_definition_name( paramname->C_name, output );
        }

        for(int opt_param_i=0; opt_param_i<optional_parameters.size(); opt_param_i++ )
        {
            if( (opt_param_i !=0) or (req_param_i!=0) )
            {
                (output->out_strm()) << (output->ln_strt) << ',';
            }

            auto paramname = optional_parameters[ opt_param_i ];

            // put default bool bit
            // bools in c are just int
            (output->out_strm()) << "int " << paramname->definition_name << "__use_default__,";
//            (output->out_strm()) << (output->ln_strt) << "int "; // bools in c are just int
//            (output->out_strm()) << (output->ln_strt) << paramname->definition_name ;
//            (output->out_strm()) << (output->ln_strt) << "__use_default__,";

            paramname->var_type->C_definition_name( paramname->C_name, output );
        }

    }
    //output<<")";
}

void func_parameter_info::print(ostream& output)
{
    output<<"( ";
    for( auto name : required_parameters)
    {
        output<< name->var_type->definition_name <<" "<< name->definition_name << ", ";
    }
    for( auto name : optional_parameters)
    {
        output<< "[ " << name->var_type->definition_name <<" "<< name->definition_name << " ], ";
    }
    output<<")";
}



/// how to map arguments to parameters ///
     // wish it worked in real-life.

parameter_to_arguments_mapper::parameter_to_arguments_mapper(func_param_ptr _parameters, function_argument_types_ptr _arguments, cast_enum cast_behavior)
{
    parameters = _parameters;
    arguments = _arguments;
    cast_mode = cast_behavior; // must be implicit_casts or pntr_casts

    if( cast_mode == cast_enum::explicit_casts )
    {
        throw gen_exception("cannot have explicit casts in parameter_to_arguments_mapper::parameter_to_arguments_mapper. This should not be reached" );
    }

    initialize();
}


void parameter_to_arguments_mapper::initialize()
{
    is_good = true; // change if not true

    // increment when needed
    num_defaults_used = 0;
    num_casts = 0;
    num_pointer_casts = 0;

    int num_unnamed_args = arguments->unnamed_argument_types.size() ;
    int num_named_args = arguments->named_argument_names.size() ;
    int num_arguments = num_unnamed_args + num_named_args;

    int num_params = parameters->total_size();
    int num_req_params = parameters->required_parameters.size();

    /// check we have a reasonable number of arguments ///
    if( num_arguments > num_params  )
    {
        error = "more arguments then parameters";
        is_good = false;
        return;
    }

    if( num_arguments < num_req_params)
    {
        error = "less arguments then required parameters";
        is_good = false;
        return;
    }

    ///now try to map the parameters and arguments ///
    // set the unnamed arguments in order
    param_to_arg_map.resize( num_params, -2 ); // -2 means "not set"
    for( int i=0; i<num_unnamed_args; i++ )
    {
        param_to_arg_map[i] = i;
    }

    // now the named arguments
    for( int named_arg_i=0; named_arg_i<num_named_args; named_arg_i++ )
    {
        auto param_name = arguments->named_argument_names[named_arg_i];
        int parameter_i = parameters->index_from_name( param_name );
        if( parameter_i==-1 )
        {
            error = "no parameter has the name ";
            error += param_name;
            is_good = false; // no parameter of that name
            return;
        }

        if( param_to_arg_map[parameter_i] != -2 )
        {
            error = "parameter: \"";
            error += parameters->parameter_from_index(parameter_i)->definition_name;
            error += utf8_string( "\" is set twice" );
            is_good = false; // parameter has been set twice
            return;
        }

        param_to_arg_map[parameter_i] = num_unnamed_args + named_arg_i;
    }

    /// check that all params have value, and that the types are compatable ///
    for( int param_i=0; param_i<num_params; param_i++ )
    {
        int arg_index = param_to_arg_map[param_i];

        if( arg_index >= 0 )
        {
            auto param_type = parameters->parameter_from_index(param_i)->var_type;
            auto arg_type = arguments->type_from_index(arg_index);
            bool param_is_reference = false;
            auto param_ref_type = param_type->is_reference_type( param_is_reference );

            if( param_is_reference )
            {
                if( not param_type->can_take_reference( arg_type ) )
                {
                    if( arg_type->can_get_pointer( param_type ) )
                    {
                        num_pointer_casts += 1;
                    }
                    else if(cast_mode==cast_enum::implicit_casts and (param_ref_type->has_implicit_copy_constructor(arg_type, cast_enum::pntr_casts)
                                                                      or arg_type->can_implicit_castTo(param_ref_type)) )
                    {
                        //we're good
                        num_casts += 1;
                    }
                    else
                    {
                        // cannot cast
                        stringstream err;
                        err << "type \"" << param_type->definition_name <<  "\" cannot reference type \"" << arg_type->definition_name << "\"\n";
                        err << "  type \"" << param_type->definition_name << "\" defined at " <<  param_type->loc << endl;
                        err << "  type \"" << arg_type->definition_name << "\" defined at " <<  arg_type->loc << endl;

                        error = err.str();

//                        error = "type \""
//                        error += param_type->definition_name;
//                        error += utf8_string( "\" cannot reference type \"");
//                        error += arg_type->definition_name;
//                        error += "\""

                        is_good = false;
                    }
                } // else we are good

            }
            else if( not arg_type->is_equivalent(param_type) ) // types not exactly the same. need to cast!
            {
                if(cast_mode==cast_enum::implicit_casts and (param_type->has_implicit_copy_constructor(arg_type, cast_enum::pntr_casts)
                                                             or arg_type->can_implicit_castTo(param_type)) )
                {
                    //we're good
                    num_casts += 1;
                }
                else
                {
                    // cannot cast
                    error = "cannot cast type \"";
                    error += arg_type->definition_name;
                    error += utf8_string( "\" to \"");
                    error += param_type->definition_name;

                    is_good = false;
                    return;
                }
            }
            else if( not param_type->has_implicit_copy_constructor(arg_type, cast_enum::pntr_casts) ) // we still have to call the copy constructor!
            {
                error = "cannot cast type \"";
                error += arg_type->definition_name;
                error += utf8_string( "\" to itself.");

                is_good = false;
                return;
            }

        }
        else if( arg_index == -2 )
        {
            // check we have a default assume casting is checked elsewhere
            if( param_i>=num_req_params )
            {
                param_to_arg_map[param_i] = -1;
                num_defaults_used += 1;
            }
            else
            {
                // not assigned, and no default

                error = "parameter \"";
                error += parameters->parameter_from_index(param_i)->definition_name;
                error += utf8_string( "\" is not set and has no default");

                is_good = false;
                return;
            }
        }
    }
}

int parameter_to_arguments_mapper::comparison(parm_arg_map_ptr RHS)
//returns 1 if this map is preferred. 0 if they are equal, -1 if other is preferred
{
    if( num_defaults_used < RHS->num_defaults_used )
    {
        return 1;
    }
    else if ( num_defaults_used > RHS->num_defaults_used )
    {
        return -1;
    }
    else
    {
        if( num_casts < RHS->num_casts )
        {
            return 1;
        }
        else if ( num_casts > RHS->num_casts )
        {
            return -1;
        }
        else
        {
            if( num_casts < RHS->num_pointer_casts )
            {
                return 1;
            }
            else if ( num_casts > RHS->num_pointer_casts )
            {
                return -1;
            }
            else
            {
                return 0;
            }
        }
    }
}

void parameter_to_arguments_mapper::write_arguments( vector<C_expression_ptr> &argument_Cexpressions,
                          vector<C_expression_ptr> &out_expressions, Csource_out_ptr output)
{
    /// useful variables
    int total_C_arguments = parameters->required_parameters.size() + 2*parameters->optional_parameters.size();
    int total_num_parameters = parameters->total_size();


    /// first we write the argument expressions
    out_expressions.reserve( total_C_arguments );
    for(int param_i = 0; param_i < total_num_parameters; param_i++ )
    {
        varName_ptr param_name = parameters->parameter_from_index( param_i );
        auto prm_typ = param_name->var_type;
        int argument_index = param_to_arg_map[param_i];// amazing bit of naming here... solid job.

        bool param_is_reference = false;
        auto param_ref_type = prm_typ->is_reference_type( param_is_reference );



        bool param_has_default  =  param_i>=(parameters->required_parameters.size());
        utf8_string use_default_vname;

        if( param_has_default )
        {
            use_default_vname = "__cy__need_default_";
            use_default_vname += output->get_unique_string();
            out_expressions.push_back( make_shared<simple_C_expression>( use_default_vname, nullptr, true, false ) );

            if( argument_index == -1 ) // use default value
            {
                (output->out_strm()) << (output->ln_strt) << "int " << use_default_vname << " = 1;" << endl;
            }
            else // do not use default value
            {
                (output->out_strm()) << (output->ln_strt) << "int " << use_default_vname << " = 0;" << endl;
            }
        }



        utf8_string argument_vname = "__cy__arg_";
        argument_vname += output->get_unique_string();

        (output->out_strm()) << (output->ln_strt);
        prm_typ->C_definition_name(argument_vname, output); // note this is destructed inside of, at the end of , the function.
        (output->out_strm()) << ';'<< endl;
        prm_typ->initialize(argument_vname, output);

        auto outArg_Cexp = make_shared<simple_C_expression>( argument_vname, prm_typ, true, false ); // NOT owned!! is destructed inside function!
        out_expressions.push_back( outArg_Cexp );



        if( argument_index == -1 )// make a default value.
        {
            if( param_is_reference and prm_typ->needs_external_memory()) // some ref-params need external memory
            {
                utf8_string memTmp_vname = "__cy__arg_MemTmp_";
                memTmp_vname +=  output->get_unique_string();

                (output->out_strm()) << (output->ln_strt);
                param_ref_type->C_definition_name(memTmp_vname, output);
                (output->out_strm()) << ';'<<endl;
                param_ref_type->initialize(memTmp_vname, output);

                auto var_name_Cexp = make_shared<owned_name>(param_ref_type, memTmp_vname);
                outArg_Cexp->add_cleanup_child( var_name_Cexp );

                prm_typ->take_reference(outArg_Cexp,  var_name_Cexp,  output);
            }
        }
        else
        {
            auto Cexpr = argument_Cexpressions[ argument_index ];
            auto Cexpr_type = Cexpr->cyth_type;

            if( param_is_reference )
            {
                if( prm_typ->can_take_reference( Cexpr_type )  or  Cexpr_type->can_get_pointer( param_ref_type ) ) // no major casting! (can pointer cast)
                {
                    C_expression_ptr Cexpr_to_reference = Cexpr;

                    if( not Cexpr->can_be_referenced )
                    { // need to copy the argument
                        utf8_string memTmp_vname = "__cy__arg_MemTmp_";
                        memTmp_vname +=  output->get_unique_string();

                        (output->out_strm()) << (output->ln_strt);
                        Cexpr_type->C_definition_name(memTmp_vname, output);
                        (output->out_strm()) << ';'<<endl;
                        Cexpr_type->initialize(memTmp_vname, output);


                        Cexpr_to_reference = make_shared<owned_name>(Cexpr_type, memTmp_vname);
                        outArg_Cexp->add_cleanup_child( Cexpr_to_reference );


                        Cexpr->has_output_ownership = false;
                        (output->out_strm()) << (output->ln_strt) << memTmp_vname << " = " << (Cexpr->get_C_expression()) << ";" << endl;
                        Cexpr_type->inform_moved(memTmp_vname, output);
                    }

                    // this can do a pointer cast if necessary.
                    prm_typ->take_reference(outArg_Cexp, Cexpr_to_reference, output);
                }
                else if( cast_mode==cast_enum::implicit_casts ) // implicit casts if allowed
                {
                    utf8_string memTmp_vname = "__cy__arg_MemTmp_";
                    memTmp_vname +=  output->get_unique_string();

                    (output->out_strm()) << (output->ln_strt);
                    param_ref_type->C_definition_name(memTmp_vname, output);
                    (output->out_strm()) << ';'<<endl;
                    param_ref_type->initialize(memTmp_vname, output);

                    auto Cexpr_to_reference = make_shared<owned_name>(param_ref_type, memTmp_vname);
                    outArg_Cexp->add_cleanup_child( Cexpr_to_reference );


                    if( param_ref_type->has_implicit_copy_constructor( Cexpr_type, cast_enum::pntr_casts )  )
                    {
                        param_ref_type->write_implicit_copy_constructor( Cexpr_to_reference, Cexpr, output, cast_enum::pntr_casts);
                    }
                    else if( Cexpr_type->can_implicit_castTo( param_ref_type ) )
                    {
                        Cexpr_type->write_implicit_castTo( Cexpr_to_reference, Cexpr, output);
                    }
                    else
                    {
                        throw gen_exception("cannot convert ", Cexpr_type->definition_name, " to ",  param_ref_type->definition_name,
                                            " in parameter_to_arguments_mapper::write_arguments. This should not be reached" );
                    }

                    prm_typ->take_reference(outArg_Cexp, Cexpr_to_reference, output);
                }
                else
                {
                    throw gen_exception("cannot convert ", Cexpr_type->definition_name, " to ",  param_ref_type->definition_name,
                                            " in parameter_to_arguments_mapper::write_arguments  :: 2 . This should not be reached" );
                }
            }
            else if( (cast_mode==cast_enum::implicit_casts or prm_typ->is_equivalent(Cexpr_type) ) and
                                prm_typ->has_implicit_copy_constructor( Cexpr_type, cast_enum::pntr_casts )  )
            {
                prm_typ->write_implicit_copy_constructor( outArg_Cexp, Cexpr, output, cast_enum::pntr_casts);
            }
            else if(cast_mode==cast_enum::implicit_casts and Cexpr_type->can_implicit_castTo( prm_typ ) )
            {
                Cexpr_type->write_implicit_castTo(outArg_Cexp, Cexpr, output);
            }
            (output->out_strm()) << endl;
        }
    }
}


// function pointer
//// JUST HERE FOR TESTS. WILL BE REMOVED ///
staticFuncPntr::staticFuncPntr()
{
    type_of_type = varType::function_pntr_t;
    definition_name = "(void)funcPntr(void)";
    C_name = "";
}

bool staticFuncPntr::is_equivalent(varType_ptr RHS)
{
    if( RHS->type_of_type ==  varType::function_pntr_t )
    {
        auto RHS_funcP = dynamic_pointer_cast<staticFuncPntr>( RHS );

        bool param_equiv =  parameters->is_equivalent( RHS_funcP->parameters );
        bool return_equiv = return_type->is_equivalent( RHS_funcP->return_type );

        return param_equiv and return_equiv;
    }
    else
    {
        return false;
    }
}


//varType_ptr staticFuncPntr::copy(utf8_string _definition_name )
//{
//    if( _definition_name == "" )
//    {
//        _definition_name = definition_name;
//    }
//
//    auto new_var = make_shared<staticFuncPntr>( );
//    new_var->definition_name = _definition_name;
//
//    new_var->self = new_var;
//    new_var->loc = loc;
//    new_var->is_ordered = is_ordered;
//    new_var->parameters = parameters;
//    new_var->return_type = return_type;
//    new_var->C_name = C_name;
//
//    return new_var;
//}

/// CALLING ///
varType_ptr staticFuncPntr::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg,
                                cast_enum cast_behavior)
{
    parameter_to_arguments_mapper param_to_arg_map( parameters, argument_types, cast_behavior );

    if( param_to_arg_map.is_good )
    {
        return return_type;
    }
    else
    {
        stringstream MSG;
        MSG << "function defined at " << loc << endl;
        MSG << "  with parameters: ";
        parameters->print( MSG );
        MSG <<endl;
        MSG << " called with arguments: ";
        argument_types->print(MSG);
        //MSG << " at " << argument_node_ptr->loc;
        MSG << endl;
        MSG << " cannot be called because " << param_to_arg_map.error << endl;
        error_msg = MSG.str();

        return nullptr;
    }
}



C_expression_ptr staticFuncPntr::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
        vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                cast_enum cast_behavior)
{
    parameter_to_arguments_mapper param_to_arg_map( parameters, argument_types, cast_behavior );

    vector<C_expression_ptr> true_arguments;
    param_to_arg_map.write_arguments( argument_Cexpressions, true_arguments, output );


    list< shared_ptr<C_expression> > child_expressions;
    stringstream out;
    out << '(' << ( LHS_Cexp->get_C_expression() ) << '(';

    bool write_comma = false;

    for(auto &arg : true_arguments)
    {
        if( write_comma )
        {
            out << ", ";
        }
        else
        {
            write_comma = true;
        }

        out << arg->get_C_expression();

        child_expressions.push_back( arg );
    }
    out << "))" ;




    C_expression_ptr RET;
    if( return_type->can_be_defined() and return_type->type_of_type!=varType::empty ) //empty is because of the void type
    {
        utf8_string new_tmp_vname = "__cy__tmp_";
        new_tmp_vname += output->get_unique_string();

        (output->out_strm()) << (output->ln_strt);
        return_type->C_definition_name(new_tmp_vname, output); // note this is destructed inside of, at the end of , the function.
        (output->out_strm()) << ';'<< endl;
        return_type->initialize(new_tmp_vname, output);

        (output->out_strm()) << (output->ln_strt) << new_tmp_vname << " = " << out.str() << ';' << endl;

        return_type->inform_moved(new_tmp_vname, output);

        RET = make_shared<owned_name>( return_type,  new_tmp_vname);
    }
    else
    {
        output->out_strm() << output->ln_strt << out.str() << ';' << endl;
        RET = make_shared<simple_C_expression>("", make_shared<void_type>(), false, false);
    }


    for(auto& W : child_expressions)
    {
        RET->add_cleanup_child(W);
    }

    return RET;
}

/// ASSIGNMENT ///
bool staticFuncPntr::get_has_assignment(varType_ptr RHS_type)
{
    if( RHS_type->type_of_type == varType::function_pntr_t )
    {
        return is_equivalent( RHS_type );
    }
    else if( RHS_type->type_of_type == varType::defined_function_t )
    {
        auto RHS_def_func = dynamic_pointer_cast<DefFuncType>(RHS_type);
        if( RHS_def_func->overloads.size()==1 )
        {
            DefFuncType::ResolvedFunction_ptr single_overload = RHS_def_func->overloads.front();

            bool param_eq = parameters->is_equivalent( single_overload->parameters );
            bool return_eq = return_type->is_equivalent( single_overload->return_type );

            return param_eq and return_eq;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

void staticFuncPntr::write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp, Csource_out_ptr output, bool call_virtual)
{
    auto RHS_type = RHS_exp->cyth_type;

    if( RHS_type->type_of_type == varType::function_pntr_t )
    {
        output->out_strm()  <<  output->ln_strt  << LHS_exp->get_C_expression() << '=' << RHS_exp->get_C_expression() <<';';
    }
    else if( RHS_type->type_of_type == varType::defined_function_t )
    { // we can't trust the RHS name here.
        auto RHS_def_func = dynamic_pointer_cast<DefFuncType>(RHS_type);
        output->out_strm()  <<  output->ln_strt  <<  LHS_exp->get_C_expression() << '=' << RHS_def_func->single_overload_Cname() <<';';// probably only place single_overload_Cname is used?
    }
}

shared_ptr<varType> staticFuncPntr::get_auto_type()
{
    return shared_from_this();
}

/// DEFINITION ///

void staticFuncPntr::C_definition_name(utf8_string& var_name, Csource_out_ptr output)
{
    utf8_string tmp("");
    return_type->C_definition_name(tmp, output);

    output->out_strm() << " (*" << var_name << ")";
    output->out_strm() << "(";
    parameters->write_to_C(output);
    output->out_strm() << ")";
}



/// defined function type
DefFuncType::DefFuncType(utf8_string name, location_span _loc)
{
    loc = _loc; // what??? this doesn't HAVE a location!!
    definition_name = name;
    C_name = "";
    is_ordered = false;
    type_of_type = varType::defined_function_t;
    all_overloads_defined = true; // no overloads yet, thus all defined!
}

//varType_ptr DefFuncType::copy(utf8_string _definition_name )
//{
//    if( _definition_name == "" )
//    {
//        _definition_name = definition_name;
//    }
//
//    auto new_var = make_shared<DefFuncType>( _definition_name, loc);
//    new_var->is_ordered = is_ordered;
//    new_var->C_name = C_name;
//    new_var->overloads = overloads;
//
//    return new_var;
//}

DefFuncType::ResolvedFunction_ptr DefFuncType::define_overload(location_span defining_loc, sym_table_base* defining_scope, func_param_ptr parameters, bool is_ordered)
{
    // first check that new overload does not conflict with current overloads
    /// assume all functions are in the same scope!
    for(auto OL : overloads)
    {
        // first check if distinguish by signiture
        if( not OL->parameters->is_distinguishable( parameters ) )
        {
            cout << "function defined twice in same scope " << OL->define_loc << " and " << defining_loc << endl;
            return nullptr; // cannot distinguish between functions!
        }
    }

    ResolvedFunction_ptr new_overload = make_shared<resolved_func>( );
    //new_overload->defining_scope = defining_scope;
    new_overload->define_loc = defining_loc;
    new_overload->is_ordered = is_ordered;
    new_overload->c_reference = defining_scope->namespace_unique_name + "__";
    new_overload->c_reference += definition_name;
    new_overload->c_reference += utf8_string("__");
    new_overload->c_reference += defining_scope->get_unique_string();

    new_overload->return_type = nullptr;
    new_overload->parameters = parameters;

    all_overloads_defined = false; // now need to define its return type.
    overloads.push_back( new_overload );
    return new_overload;
}

void DefFuncType::define_overload_return(ResolvedFunction_ptr overload,  varType_ptr _return_type)
{
    overload->return_type = _return_type;

    /// check to see if all overloads defined!
    all_overloads_defined = true;// assume
    for(  auto OL : overloads)
    {
        if( not OL->return_type )
        {
            all_overloads_defined = false;
            return;
        }
    }
}


 /// CALLING ///
DefFuncType::resolved_pair DefFuncType::get_resolution(function_argument_types_ptr argument_types, utf8_string& error_msg, cast_enum cast_behavior)
{
    ResolvedFunction_ptr current_overload = nullptr;
    shared_ptr<parameter_to_arguments_mapper> current_argument_map = nullptr;

    list< pair<ResolvedFunction_ptr, utf8_string> > attempted_resolutions;

    for(auto OL : overloads)
    {
        // first check this overload is reasonable
//        if( OL->is_ordered and not argument_node_ptr->loc.strictly_GT( OL->define_loc ) )
//        {
//            attempted_resolutions.push_back( make_pair( OL, "overload defined after call" ) );
//            continue;
//        }

        shared_ptr<parameter_to_arguments_mapper> new_argument_map =
                                make_shared<parameter_to_arguments_mapper>(OL->parameters, argument_types, cast_behavior);
        if( not new_argument_map->is_good )// cannot map these arguments to these parameters
        {
            attempted_resolutions.push_back( make_pair( OL, new_argument_map->error ) );
            continue;
        }

        if( not current_overload ) // can map, and no competitors
        {
            current_overload = OL;
            current_argument_map = new_argument_map;
        }
        else
        // OOPS! these two conflict, need to find which one is better
        {
            // check signiture
            int cmp_sig = new_argument_map->comparison( current_argument_map );
            if( cmp_sig==1 )
            { // prefer the new one
                current_overload = OL;
                current_argument_map = new_argument_map;
            }
            else if( cmp_sig == -1)
            {
                // do nothing!
                // present one is prefered!
            }
            // I guess earlier if sig can't seperate them
            // hope this doesn't cause issues!

            // signitures are the same, so now we check which one has the most inner scope
//             else if( current_overload->defining_scope->is_or_inScope( OL->defining_scope ) )
//             {
//                 // OL is the most inner (note they cannot be the same scope)
//                 current_overload = OL;
//                 current_argument_map = new_argument_map;
//             }
        }
    }

    if( not current_overload )
    {// no resolution found
        stringstream TMP;
        TMP << "cannot resolve overload with arguments: " ;
        argument_types->print( TMP );
        TMP << endl;
        for( auto P : attempted_resolutions )
        {
            TMP << "overload defined " << P.first->define_loc << " with parameters ";
            P.first->parameters->print( TMP );
            TMP <<endl;
            TMP << "  is invalid because "<<P.second<<endl;
        }

        error_msg = TMP.str();

        return resolved_pair();
    }
    else
    {
        return make_pair( current_overload, current_argument_map);
    }
}

varType_ptr DefFuncType::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg,
                                cast_enum cast_behavior)
{
    auto OV = get_resolution( argument_types, error_msg, cast_behavior );
    if(OV.first)
        return OV.first->return_type;
    return nullptr;
}

C_expression_ptr DefFuncType::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
        vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                cast_enum cast_behavior)
{
    utf8_string TMP;
    auto OV = get_resolution( argument_types, TMP, cast_behavior );
    if( not OV.first )
        throw gen_exception("ERROR: DefFuncType::write_call error this should never be seen");

    auto func_overload = OV.first;

    auto args_mapper = OV.second;
    vector<C_expression_ptr> out_expressions;
    OV.second->write_arguments( argument_Cexpressions, out_expressions, output);


    stringstream out;
    out << '(' << func_overload->c_reference << '(';

    list< shared_ptr<C_expression> > child_expressions;
    bool write_comma = false;
    for(auto &out_arg : out_expressions)
    {
        if( write_comma )
        {
            out << ", ";
        }
        else
        {
            write_comma = true;
        }

        out << out_arg->get_C_expression();
        child_expressions.push_back( out_arg );
    }
    out << "))" ;




    C_expression_ptr RET;
    if( func_overload->return_type->can_be_defined() and func_overload->return_type->type_of_type!=varType::empty ) //empty is because of the void type
    {
        utf8_string new_tmp_vname = "__cy__tmp_";
        new_tmp_vname += output->get_unique_string();

        (output->out_strm()) << (output->ln_strt);
        func_overload->return_type->C_definition_name(new_tmp_vname, output); // note this is destructed inside of, at the end of , the function.
        (output->out_strm()) << ';'<< endl;
        func_overload->return_type->initialize(new_tmp_vname, output);

        (output->out_strm()) << (output->ln_strt) << new_tmp_vname << " = " << out.str() << ';' << endl;

        func_overload->return_type->inform_moved(new_tmp_vname, output);

        RET = make_shared<owned_name>( func_overload->return_type,  new_tmp_vname);
    }
    else
    {
        output->out_strm() << output->ln_strt << out.str() << ';' << endl;
        RET = make_shared<simple_C_expression>("", make_shared<void_type>(), false, false);
    }


    for(auto& W : child_expressions)
    {
        RET->add_cleanup_child(W);
    }

    return RET;
}


shared_ptr<varType> DefFuncType::get_auto_type()
{
    if( overloads.size() == 1 )
    {
        ResolvedFunction_ptr single_overload = overloads.front();

        auto funcPntr = make_shared<staticFuncPntr>();
        funcPntr->loc = single_overload->define_loc; // probably not right? probably doesn't matter, becouse these arn't proper symbols
        funcPntr->is_ordered = single_overload->is_ordered;
        funcPntr->return_type = single_overload->return_type;
        funcPntr->parameters = single_overload->parameters;

        return funcPntr;
    }
    else
    {
        cout << "overload of function " << definition_name <<" is ambiguous." << endl;
        return nullptr;
    }
}

utf8_string DefFuncType::single_overload_Cname()
{
    if( overloads.size() == 1 )
    {
        ResolvedFunction_ptr single_overload = overloads.front();

        return single_overload->c_reference;
    }
    else
    {
        throw gen_exception("function with multiple overloads does not have one c name");
        return "";
    }
}

void DefFuncType::resolved_func::write_C_prototype(Csource_out_ptr output)
{
    utf8_string TMP("");
    return_type->C_definition_name(TMP, output);

    output->out_strm() << "(";
    output->out_strm() << c_reference;
    output->out_strm() << "(";
    parameters->write_to_C(output, true);
    output->out_strm() << "))";
}






/// CLASSES ///

DefClassType::DefClassType(utf8_string _name, utf8_string _c_name, bool _is_ordered, sym_table_ptr _class_symbol_table, location_span _loc)
{
    type_of_type = varType::defined_class_t;
    loc = _loc;
    definition_name = _name;
    C_name = _c_name;
    is_ordered = _is_ordered;
    class_symbol_table = _class_symbol_table;

    vtableType_cname = definition_name + "_" + class_symbol_table->get_unique_string();
    global_vtableVar_cname = definition_name + "_" + class_symbol_table->get_unique_string();
}

bool DefClassType::is_equivalent(varType_ptr RHS)
{
    //return ((void*)this) == ((void*)RHS);
    return (RHS->type_of_type==varType::defined_class_t) and (C_name == RHS->C_name); // I hope this works.
}


/// DEFINITION ///
void DefClassType::C_definition_name(utf8_string& var_name, Csource_out_ptr output)
{
    output->out_strm() << "struct " << C_name << " " << var_name;
}

void DefClassType::initialize(utf8_string& var_exp, Csource_out_ptr output)
{
    // initilize the vtable
    output->out_strm()  << "(" << var_exp << ").__cy_vtable = &" << global_vtableVar_cname << ";" << endl;

    // now initialize children
    // note that this initializes the immediate parents!!    however, all the vtables are wrong!
    // this is wasteful, as vtables are set for every parent for every level. Need check to see if is parent, initilize differently

    auto var_Cexp = make_shared<simple_C_expression>(var_exp, shared_from_this(), true, false ); // merely a referencing thing. Does not own memory!

    for( auto &X : class_symbol_table->variable_table )
    {
        auto def_name = X.first;
        auto type = X.second->var_type;

        auto get_exp = write_member_getref( var_Cexp, def_name, output );
        auto exp_str = get_exp->get_C_expression();
        type->initialize( exp_str, output );
        get_exp->write_cleanup( output );
    }

    // now we initialize the vtables of all parents
    for( int parent_index=0; parent_index!=full_inheritance_tree.size(); ++parent_index )
    {
        auto parent_reference = write_parent_access(parent_index, var_Cexp, output);
        output->out_strm() << "(" << parent_reference->get_C_expression() << ").__cy_vtable = &" << global_parentVtable_cnames[parent_index] << ";" << endl;
        parent_reference->write_cleanup( output );
    }


    var_Cexp->write_cleanup( output ); // probably not really needed, but technically required
}

void DefClassType::inform_moved( utf8_string& var_name, Csource_out_ptr output, bool call_virtual)
{
    utf8_string moveSTR("__moved__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto moved_method_var = class_symbol_table->get_variable_local(moveSTR, tmp_loc, check_order);
    if( not moved_method_var )
    {
        return;
    }

    // call!
    auto self_accses = make_shared<simple_C_expression>(var_name, shared_from_this(), true, false); // does NOT own its memory!! merely for reference

    function_argument_types_ptr argument_types = make_shared<function_argument_types>();
    vector<C_expression_ptr> argument_Cexpressions;


    auto method_type = moved_method_var->var_type->as_method();
    auto method_Cexp = method_type->parental_write_call(argument_types, self_accses, argument_Cexpressions,
                                    output, call_virtual);

    output->out_strm() << output->ln_strt << method_Cexp->get_C_expression() << ';' << endl;

    method_Cexp->write_cleanup( output );
    self_accses->write_cleanup( output ); // probably not really needed, but technically required
}

varType_ptr DefClassType::get_auto_type()
{
    return shared_from_this();
}

/// REFERENCING ///
bool DefClassType::can_get_pointer(varType_ptr output_type)
{
    if( is_equivalent( output_type )  )
    {
        return true;
    }
    else
    {
        for(auto &par_class : full_inheritance_tree )
        {
            if( par_class->is_equivalent( output_type ) )
            {
                return true;
            }
        }
    }
    return false;
}

utf8_string DefClassType::get_pointer(varType_ptr output_type, csu::utf8_string& exp, Csource_out_ptr output)
{
    auto Cexp = make_shared<simple_C_expression>(exp, shared_from_this(), true, false ); // merely refernce. Does not own  memory

    if( is_equivalent( output_type ) )
    {
        return "&(" + exp + ")";
    }
    else
    {
        auto clss_pntr = output_type->as_class();
        if( clss_pntr )
        {
            int parent_pointer = get_parent_index( clss_pntr );
            if( parent_pointer >= 0)
            {
                auto new_exp = write_parent_access(parent_pointer, Cexp, output);
                return "&(" + new_exp->get_C_expression() + ")";;
            }
        }

        throw gen_exception("ERROR: TYPE ", definition_name, " cannot be referenced as ", output_type->definition_name,
                            ". In DefClassType::get_pointer, should not be reached." );
    }


    Cexp->write_cleanup( output ); // probably not really needed, but technically required
}

/// CLASS THINGS ///
varName_ptr DefClassType::get_member_full_inheritance(int& parent_index, utf8_string& member_name)
{
    location_span tmploc;
    bool check_order = false;

    auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    if( varname )
    {
        parent_index = -1;
        return varname;
    }
    else
    {
        for(int parent_index_=full_inheritance_tree.size()-1; parent_index_>-1; --parent_index_)
        {
            check_order = false;
            auto parent_class = full_inheritance_tree[parent_index_];
            varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);
            if( varname )
            {
                if( varname->var_type->type_of_type == varType::method_function_t )
                {
                    break; // only return methods from local table, not from parents
                }

                parent_index = parent_index_;
                return varname;
            }
        }

        parent_index = -2;
        return nullptr;
    }
}

varType_ptr DefClassType::member_has_getref(utf8_string& member_name)
{
 //   location_span tmploc;
  //  bool check_order = false;
   // auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    int parent_index;
    auto varname = get_member_full_inheritance(parent_index, member_name);

    if(varname)
    {
        return varname->var_type;
    }
    else
    {
        return nullptr;
    }
}

C_expression_ptr DefClassType::write_member_getref(C_expression_ptr LHS_exp, utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual)
{
    int parent_index;
    auto varname = get_member_full_inheritance(parent_index, member_name);

    if( not varname)
    {
        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
    }

    auto expression_to_use = LHS_exp;
    bool own_exp = false;
    if( parent_index != -1 )
    {
        own_exp = true;
        expression_to_use = write_parent_access( parent_index, LHS_exp, output );
    }

    if( varname->var_type->type_of_type == varType::method_function_t)
    {
        auto the_method = varname->var_type->as_method();
        return the_method->get_C_exp(expression_to_use, call_virtual, own_exp);
    }
    else
    {
        stringstream out_str;
        out_str << "(" << expression_to_use->get_C_expression() << "." << varname->C_name << ")";
        auto out_cexp = make_shared< simple_C_expression >( out_str.str(), varname->var_type, true, false );
        if( own_exp )
        {
            out_cexp->add_cleanup_child( expression_to_use );
        }
        return out_cexp;
    }
}

varType_ptr DefClassType::member_has_getter(utf8_string& member_name)
{
    return member_has_getref(member_name);
}

C_expression_ptr DefClassType::write_member_getter(C_expression_ptr LHS_exp, utf8_string& member_name,
            Csource_out_ptr output, bool call_virtual)
{
    return write_member_getref( LHS_exp, member_name, output, call_virtual);
}

varType_ptr DefClassType::set_member(csu::utf8_string& member_name, varType_ptr RHS_type)
{
    // first get the member


    int parent_index;
    auto varname = get_member_full_inheritance(parent_index, member_name);

    if( not varname)
    {
        // don't give error, just return
        return nullptr;
    }

    if( RHS_type )
    {
        bool has_assign = varname->var_type->get_has_assignment( RHS_type )  or  RHS_type->get_has_assignTo( varname->var_type );
        if( not has_assign )
        {
            return nullptr;
        }
    }

    // if here, then we are good
    return varname->var_type;
}

void DefClassType::write_member_setter(C_expression_ptr LHS_expression, utf8_string& member_name,
         C_expression_ptr RHS_expression, Csource_out_ptr output, bool call_virtual)
{
    // WELL! this is going to be nice and complicated!
    // NOTE: that call_virtual only applies to method calls on self, NOT on the LHS member or RHS exp

    // first get the member

    int parent_index;
    auto varname = get_member_full_inheritance(parent_index, member_name);

    if( not varname)
    {
        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
    }

    // now write full LHS code
    utf8_string LHS = "(";
    C_expression_ptr parent_accses = nullptr;
    if( parent_index==-1 )
    {
        LHS += LHS_expression->get_C_expression();
    }
    else
    {
        parent_accses = write_parent_access(parent_index,  LHS_expression, output );
        LHS += parent_accses->get_C_expression();
    }
    LHS += utf8_string(".");
    LHS += varname->C_name;
    LHS += utf8_string(")");

    auto LHS_exp = make_shared< simple_C_expression >( LHS, varname->var_type, true, false );

    // now assign to it!
    auto RHS_type = RHS_expression->cyth_type;
    if( varname->var_type->get_has_assignment( RHS_type ) )
    {
        varname->var_type->write_assignment(LHS_exp, RHS_expression, output);
        LHS_exp->write_cleanup( output ); // technically unneeded, but is good form

        if(parent_accses)
        {
            parent_accses->write_cleanup( output );
        }

    }
    else if( RHS_type->get_has_assignTo( varname->var_type ))
    {
        RHS_type->write_assignTo(LHS_exp, RHS_expression, output);
        LHS_exp->write_cleanup( output ); // technically unneeded, but is good form

        if(parent_accses)
        {
            parent_accses->write_cleanup( output );
        }
    }
    else
    {
        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
    }

//    else if( RHS_type->can_implicit_castTo( varname->var_type.get() )  )
//    {
//// THIS IS WRONG!!
//        RHS_type->write_implicit_castTo(varname->var_type.get(), exp_AST_node, LHS, RHS_expression, output);
//    }
}

/// CONSTRUCTORS ///
// never inhereted, never virtual!!!
void DefClassType::write_default_constructor(utf8_string& var_name, Csource_out_ptr output)
{
    // default constructor??
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( not init_method_var )
    {// no init method!
        return;
    }

    auto init_method_type = init_method_var->var_type->as_method() ;
    bool found_default_constuctor = false;
    for( auto &instance : init_method_type->overloads )
    {
        if( instance->parameters->total_size() == 0 )
        {
            found_default_constuctor = true;
            break;
        }
    }

    if( not found_default_constuctor)
    {
        return;
    }


    // setup the function call
    auto self_name_writer = make_shared<simple_C_expression>( var_name,  shared_from_this(), true, false);

    auto empty_args = make_shared<function_argument_types>();
    vector<C_expression_ptr> argument_Cexpressions;

    auto func_writer = init_method_type->parental_write_call( empty_args, self_name_writer, argument_Cexpressions, output, false);

    // make the call
    output->out_strm() << output->ln_strt << func_writer->get_C_expression() << ';' << endl;
    func_writer->write_cleanup( output );
    self_name_writer->write_cleanup( output );
}

bool DefClassType::has_explicit_constructor(function_argument_types_ptr argument_types, csu::utf8_string& error_msg)
{
    stringstream running_err;

    // first check implicit constructors
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( init_method_var )
    {
        utf8_string TMP;
        auto ret = init_method_var->var_type->is_callable( argument_types, TMP );
        if( ret )
        {
            return true;
        }
        running_err << TMP << endl;
    }

    // now try explicit constructors
    utf8_string exInitSTR("__exInit__");
    check_order = false;
    auto exInit_method_var = class_symbol_table->get_variable_local(exInitSTR, tmp_loc, check_order);
    if( exInit_method_var )
    {
        utf8_string TMP;
        auto ret = exInit_method_var->var_type->is_callable( argument_types, TMP );
        if( ret )
        {
            return true;
        }
        running_err << TMP << endl;
    }


    // BAD
    error_msg = running_err.str();
    return false;
}

void DefClassType::write_explicit_constructor(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
                        vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output)
{

    // try implicit
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( init_method_var )
    {
        utf8_string tmp;
        auto ret = init_method_var->var_type->is_callable( argument_types, tmp );
        if( ret )
        {
            auto method_type = init_method_var->var_type->as_method();
            auto writer = method_type->parental_write_call(argument_types, LHS_Cexp,
                                                        argument_Cexpressions, output );
            output->out_strm() <<  output->ln_strt  << writer->get_C_expression() << ';' <<endl;
            writer->write_cleanup( output );
            return;
        }
    }

    // now try explicit!
    utf8_string exInitSTR("__exInit__");
    check_order = false;
    auto exInit_method_var = class_symbol_table->get_variable_local(exInitSTR, tmp_loc, check_order);
    if( exInit_method_var )
    {
        auto method_type = exInit_method_var->var_type->as_method();
        auto writer = method_type->parental_write_call(argument_types, LHS_Cexp,
                                                        argument_Cexpressions, output  );
        output->out_strm() <<  output->ln_strt  << writer->get_C_expression() << ';' <<endl;
        writer->write_cleanup( output );
    }
}

bool DefClassType::has_explicit_copy_constructor(varType_ptr RHS_type, utf8_string& error_msg)
{
    auto func_arg_types = make_shared<function_argument_types>();
    func_arg_types->unnamed_argument_types.push_back( RHS_type );



    stringstream running_err;

    // default constructor??
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    utf8_string TMP1;
    if( init_method_var and init_method_var->var_type->is_callable(func_arg_types, TMP1) )
    {
        return true;
    }
    running_err << TMP1 << endl;


    // now try explicit constructors
    utf8_string exInitSTR("__exInit__");
    check_order = false;
    auto exInit_method_var = class_symbol_table->get_variable_local(exInitSTR, tmp_loc, check_order);
    utf8_string TMP2;
    if( exInit_method_var and exInit_method_var->var_type->is_callable(func_arg_types, TMP2) )
    {
        return true;
    }
    running_err << TMP2 << endl;


    // BAD
    error_msg = running_err.str();
    return false;
}


void DefClassType::write_explicit_copy_constructor(C_expression_ptr LHS_exp,
                        C_expression_ptr RHS_exp, Csource_out_ptr output)

{
    auto RHS_type = RHS_exp->cyth_type;

    auto func_arg_types = make_shared<function_argument_types>();
    func_arg_types->unnamed_argument_types.push_back( RHS_type );

    vector<C_expression_ptr> argument_Cexpressions({ RHS_exp });


    stringstream running_err;

    // default constructor??
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    utf8_string TMP1;
    if( init_method_var and init_method_var->var_type->is_callable(func_arg_types, TMP1) )
    {
        auto method_type = init_method_var->var_type->as_method();
        auto writer = method_type->parental_write_call(func_arg_types, LHS_exp, argument_Cexpressions, output);

        output->out_strm() <<  output->ln_strt  << writer->get_C_expression() << ';' <<endl;
        writer->write_cleanup( output );
        return;

    }
    running_err << TMP1 << endl;


    // now try explicit constructors
    utf8_string exInitSTR("__exInit__");
    check_order = false;
    auto exInit_method_var = class_symbol_table->get_variable_local(exInitSTR, tmp_loc, check_order);
    utf8_string TMP2;
    if( exInit_method_var and exInit_method_var->var_type->is_callable(func_arg_types, TMP2) )
    {
        auto method_type = exInit_method_var->var_type->as_method();
        auto writer = method_type->parental_write_call(func_arg_types, LHS_exp, argument_Cexpressions, output);

        output->out_strm() <<  output->ln_strt  << writer->get_C_expression() << ';' <<endl;
        writer->write_cleanup( output );
        return;
    }
    running_err << TMP2 << endl;


    // BAD
    //auto error_msg = running_err.str();
    throw gen_exception("problem in DefClassType::write_explicit_copy_constructor. This should not be reached!");
}


bool DefClassType::has_implicit_copy_constructor(varType_ptr RHS_type, cast_enum cast_behavior)
{
    // default constructor??
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( not init_method_var or init_method_var->var_type->type_of_type!=method_function_t)
    {// no init method!
        return false;
    }


    utf8_string error_tmp;
    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( RHS_type );

    auto init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
    auto resolved_pair = init_method_type->get_resolution(args_typs, error_tmp, cast_behavior);
    auto method_resolution = resolved_pair.first;

    if( method_resolution and (method_resolution->parameters->required_parameters.size() == 1) and (method_resolution->parameters->optional_parameters.size() == 0) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void DefClassType::write_implicit_copy_constructor( C_expression_ptr LHS, C_expression_ptr RHS_exp,
                                            Csource_out_ptr output, cast_enum cast_behavior)
{

    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( not init_method_var )
    {// no init method! eventually this should never happen?
        throw gen_exception("cannot find overload::1 in DefClassType::write_implicit_copy_constructor. This should not be reached!");
    }

    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( RHS_exp->cyth_type );

    vector<C_expression_ptr> argument_Cexpressions({RHS_exp});

    auto method_type = init_method_var->var_type->as_method();
    auto c_exp = method_type->parental_write_call(args_typs, LHS, argument_Cexpressions, output, false, cast_behavior);

    output->out_strm()  <<  output->ln_strt  << c_exp->get_C_expression() << ';' << endl;
    c_exp->write_cleanup( output );
}
//    auto init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
//    MethodType::ResolvedMethod_ptr found_method = nullptr;
//    int pref = -1; // track preference higher is better
//    // -1 is not set
//    // 0 is parameter is generic reference
//    // 1  RHS can implicit cast_to param-ref-type       (only allowed if cast_behavior==0)
//    // 2   param-ref-type can implict copy-construct from RHS   (only allowed if cast_behavior==0)
//    // 3 is RHS can be pointer to param-ref-type
//    // 4 is parameter is ref to LHS type
//    for( auto &instance : init_method_type->overloads )
//    {
//        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
//        {
//            auto TYP = instance->parameters->required_parameters.front()->var_type;
//
//            bool is_ref;
//            auto ref_typ = TYP->is_reference_type( is_ref );
//
//            if( not is_ref ) { continue; } // what else to do?
//
//            if( ref_typ->is_equivalent(RHS_type) ) // most prefered
//            {
//
//                found_method = instance;
//                break;
//            }
//            else if( ref_typ and RHS_type->can_get_pointer(ref_typ.get()) )
//            {
//                pref = 3;
//                found_method = instance;
//            }
//            else if( pref == -1 and not ref_typ )
//            {
//                found_method = instance;
//                pref = 0;
//            }
//            else if( cast_behavior==0 and pref<2 and (not is_equivalent(ref_typ.get())) and ref_typ->has_implicit_copy_constructor( RHS_type, 1 ) )
//            {
//                found_method = instance;
//                pref = 2;
//            }
//            else if( cast_behavior==0 and pref<1 and RHS_type->can_implicit_castTo( ref_typ.get() ) )
//            {
//                found_method = instance;
//                pref = 1;
//            }
//        }
//    }







/// destructors ///
void DefClassType::write_destructor(utf8_string& var_name, Csource_out_ptr output, bool call_virtual)
{
    utf8_string delSTR("__del__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto del_method_var = class_symbol_table->get_variable_local(delSTR, tmp_loc, check_order);
    if( not del_method_var )
    {
        return;
    }

    // call!

    auto self_name_writer = make_shared<simple_C_expression>( var_name,  shared_from_this(), true, false);
    auto argument_types = make_shared<function_argument_types>();
    vector<C_expression_ptr> argument_Cexpressions;

    // make sure it's callable
    auto method_type = del_method_var->var_type->as_method();
    auto writer = method_type->parental_write_call(argument_types, self_name_writer, argument_Cexpressions, output, call_virtual );

    output->out_strm()  <<  output->ln_strt  <<  writer->get_C_expression()  << ';' <<endl;
    writer->write_cleanup( output );
    self_name_writer->write_cleanup( output );
}



/// ASSIGNMENT ///
bool DefClassType::get_has_assignment(varType_ptr RHS_type)
{
    utf8_string assignSTR("__assign__");
    int parent_index;
    auto assign_method_var = get_member_full_inheritance(parent_index, assignSTR);
    if( not assign_method_var or assign_method_var->var_type->type_of_type!=method_function_t)
    {
        return false;
    }

    utf8_string error_tmp;
    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( RHS_type );

    auto assign_method_type = dynamic_pointer_cast<MethodType>( assign_method_var->var_type );
    auto resolved_pair = assign_method_type->get_resolution(args_typs, error_tmp, cast_enum::implicit_casts);

    auto method_resolution = resolved_pair.first;

    if( method_resolution and (method_resolution->parameters->required_parameters.size() == 1) and (method_resolution->parameters->optional_parameters.size() == 0) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
//    utf8_string assignSTR("__assign__");
//
//    int parent_index;
//    auto assign_method_var = get_member_full_inheritance(parent_index, assignSTR);
//
//    if( not assign_method_var )
//    {
//        return false;
//    }
//
//    auto assign_method_type = dynamic_pointer_cast<MethodType>( assign_method_var->var_type );
//    for( auto &instance : assign_method_type->overloads )
//    {
//        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
//        {
//            auto TYP = instance->parameters->required_parameters.front()->var_type;
//
//            bool is_ref;
//            auto ref_typ = TYP->is_reference_type( is_ref );
//            if( not is_ref){ continue; } // what else? this should never happen
//
//            if( (not ref_typ) or RHS_type->can_get_pointer(ref_typ.get()) ) // paremater can be ANYTHING, or argument can pointer cast to param
//            {
//                return true;
//            }
//            else if(  (not is_equivalent(ref_typ.get())) and ref_typ->has_implicit_copy_constructor( RHS_type, 1 ) )
//            {
//                return true;
//            }
//            else if( RHS_type->can_implicit_castTo( ref_typ.get() ) )
//            {
//                return true;
//            }
//        }
//    }
//    return false;


void DefClassType::write_assignment(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual)
{
    utf8_string assignSTR("__assign__");

    int parent_index;
    auto assign_method_var = get_member_full_inheritance(parent_index, assignSTR);

    if( not assign_method_var )
    {
        throw gen_exception("cannot find method in DefClassType::write_assignment. This should not be reached!");
    }


//    utf8_string expression_to_use = LHS_exp->get_C_expression();
//    if( parent_index != -1 )
//    {
//        expression_to_use = write_parent_access( parent_index, expression_to_use, output );
//    }
//    auto var_name_writer = make_shared< MethodType::method_access_writer >(expression_to_use, shared_from_this(), call_virtual);

    C_expression_ptr exp_to_use;
    bool clean_exp_to_use = false;
    if( parent_index != -1 )
    {
        exp_to_use = write_parent_access( parent_index, LHS_exp, output );
        clean_exp_to_use = true;
    }
    else
    {
        exp_to_use = LHS_exp;
        clean_exp_to_use = false;
    }


    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( RHS_exp->cyth_type );

    vector<C_expression_ptr> argument_Cexpressions({ RHS_exp });

    auto method_type = assign_method_var->var_type->as_method();
    auto c_exp = method_type->parental_write_call(args_typs, exp_to_use, argument_Cexpressions, output, call_virtual);

    output->out_strm()  <<  output->ln_strt  << c_exp->get_C_expression() << ';' << endl;
    c_exp->write_cleanup( output );

    if( clean_exp_to_use )
    {
        exp_to_use->write_cleanup( output );
    }
}
//    auto assign_method_type = dynamic_pointer_cast<MethodType>( assign_method_var->var_type );
//    MethodType::ResolvedMethod_ptr found_method = nullptr;
//    int pref = -1; // track preference higher is better
//    // -1 is not set
//    // 0 is parameter is generic reference
//    // 1  RHS can implicit cast_to param-ref-type
//    // 2   param-ref-type can implict copy-construct from RHS
//    // 3 is RHS can be pointer to param-ref-type
//    // 4 is parameter is ref to LHS type
//    for( auto &instance : assign_method_type->overloads )
//    {
//        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
//        {
//            auto TYP = instance->parameters->required_parameters.front()->var_type;
//
//            bool is_ref;
//            auto ref_typ = TYP->is_reference_type( is_ref );
//
//            if( not is_ref ) { continue; } // what else to do?
//
//            if( ref_typ->is_equivalent(RHS_type) ) // most prefered
//            {
//                found_method = instance;
//                break;
//            }
//            else if( ref_typ and RHS_type->can_get_pointer(ref_typ.get()) )
//            {
//                pref = 3;
//                found_method = instance;
//            }
//            else if( pref == -1 and not ref_typ )
//            {
//                found_method = instance;
//                pref = 0;
//            }
//            else if( pref<2 and (not is_equivalent(ref_typ.get())) and ref_typ->has_implicit_copy_constructor( RHS_type, 1 ) )
//            {
//                found_method = instance;
//                pref = 2;
//            }
//            else if( pref<1 and RHS_type->can_implicit_castTo( ref_typ.get() ) )
//            {
//                found_method = instance;
//                pref = 1;
//            }
//        }
//    }


bool DefClassType::get_has_assignTo(varType_ptr LHS_type)
{
    utf8_string assignSTR("__assignTo__");

    int parent_index;
    auto assign_method_var = get_member_full_inheritance(parent_index, assignSTR);
    if( not assign_method_var or assign_method_var->var_type->type_of_type!=method_function_t)
    {
        return false;
    }

    utf8_string error_tmp;
    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( LHS_type );

    auto assign_method_type = dynamic_pointer_cast<MethodType>( assign_method_var->var_type );
    auto resolved_pair = assign_method_type->get_resolution(args_typs, error_tmp, cast_enum::pntr_casts);
    auto method_resolution = resolved_pair.first;

    if( method_resolution and (method_resolution->parameters->required_parameters.size() == 1) and (method_resolution->parameters->optional_parameters.size() == 0) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
//    for( auto &instance : assign_method_type->overloads )
//    {
//        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
//        {
//            auto TYP = instance->parameters->required_parameters.front()->var_type;
//
//            bool is_ref;
//            auto ref_typ = TYP->is_reference_type( is_ref );
//            if(is_ref)
//            {
//                if( (not ref_typ) or LHS_type->can_get_pointer(ref_typ.get()) ) // paremater can be ANYTHING, or argument can cast to param
//                {
//                    return true;
//                }
//            }
//            // else? should always be a REF
//        }
//    }
//    return false;


void DefClassType::write_assignTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                            Csource_out_ptr output, bool call_virtual)
{
    utf8_string convertSTR("__assignTo__");

    int parent_index;
    auto convert_method_var = get_member_full_inheritance(parent_index, convertSTR);

    if( not convert_method_var )
    {
        throw gen_exception("cannot find overload in DefClassType::write_assignTo. This should not be reached!");
    }



    C_expression_ptr exp_to_use;
    bool clean_exp_to_use = false;
    if( parent_index != -1 )
    {
        exp_to_use = write_parent_access( parent_index, RHS_exp, output );
        clean_exp_to_use = true;
    }
    else
    {
        exp_to_use = RHS_exp;
        clean_exp_to_use = false;
    }


    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( LHS_exp->cyth_type );

    vector<C_expression_ptr> argument_Cexpressions({ LHS_exp });

    auto as_method = convert_method_var->var_type->as_method();
    auto c_exp = as_method->parental_write_call(args_typs, exp_to_use, argument_Cexpressions, output, call_virtual, cast_enum::pntr_casts);


    output->out_strm()  <<  output->ln_strt  << c_exp->get_C_expression() << ';' << endl;
    c_exp->write_cleanup( output );
    if( clean_exp_to_use )
    {
        exp_to_use->write_cleanup( output );
    }
}
//    auto convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
//    MethodType::ResolvedMethod_ptr found_method = nullptr;
//    int pref = -1; // track preference higher is better
//    // -1 is not set
//    // 0 is parameter is generic reference
//    // 1 is LHStype can be pointer to ref type
//    // 2 is parameter is ref to LHS type
//    for( auto &instance : convert_method_type->overloads )
//    {
//        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
//        {
//            auto TYP = instance->parameters->required_parameters.front()->var_type;
//
//            bool is_ref;
//            auto ref_typ = TYP->is_reference_type( is_ref );
//
//            if( not is_ref ) { continue; } // what else to do?
//
//            if( ref_typ->is_equivalent(LHS_type) ) // most prefered
//            {
//
//                found_method = instance;
//                break;
//            }
//            else if( ref_typ and LHS_type->can_get_pointer(ref_typ.get()) )
//            {
//                pref = 1;
//                found_method = instance;
//            }
//            else if( pref == -1 and not ref_typ )
//            {
//                found_method = instance;
//                pref = 0;
//            }
//
//
//
////            if( TYP->type_of_type == c_pointer_reference )
////            {
////                auto TYP_casted = dynamic_pointer_cast<raw_C_pointer_reference>(TYP);
////                if( TYP_casted->referenced_type->is_equivalent(LHS_type) )
////                {
////                    found_method = instance;
////                    break;
////                }
////            }
//        }
//    }
//
//    if( not found_method)
//    {
//        throw gen_exception("cannot find overload in DefClassType::write_assignTo. This should not be reached!");
//    }
//
//
//
//    utf8_string expression_to_use = RHS_exp;
//    if( parent_index != -1 )
//    {
//        expression_to_use = write_parent_access( parent_index, RHS_exp, output );
//    }
//
//    auto var_name_writer = make_shared< MethodType::method_access_writer >(expression_to_use, call_virtual);
//
//
//
//
//    // now assignment!
//    vector<utf8_string> argument_Cexpressions;
//    argument_Cexpressions.push_back(LHS);
//
//
//// I HOPE THIS WORKS!! SOOOO hacky!!
//    auto LHS_exp = make_shared<varReferance_expression_AST_node>( LHS, RHS_AST_node->loc );
//    LHS_exp->has_output_ownership = false;
//    LHS_exp->c_exp_can_be_referenced = true;
//    LHS_exp->variable_symbol = make_shared<varName>();
//    LHS_exp->variable_symbol->var_type = LHS_type->shared_from_this();
//    LHS_exp->variable_symbol->C_name = LHS;
//    LHS_exp->variable_symbol->loc = RHS_AST_node->loc ;
//    LHS_exp->variable_symbol->is_ordered = false;
//    LHS_exp->expression_return_type = LHS_exp->variable_symbol->var_type;
//
//
//    vector<expression_AST_node*> argument_expressions;
//    argument_expressions.push_back( LHS_exp.get() );
//
//
//    //auto var_name_writer = make_shared<MethodType::method_access_writer>( RHS_exp, call_virtual );
//    auto writer = convert_method_type->write_call(found_method, argument_expressions,
//               RHS_AST_node->symbol_table, var_name_writer, argument_Cexpressions, output);
//
//    output << writer->get_C_expression() << ';' <<endl;
//    writer->write_cleanup();




/// CASTING TO OTHER TYPES ///
// can be inherited and virtual
bool DefClassType::can_implicit_castTo(varType_ptr LHS_type)
{
    utf8_string convertSTR("__convert__");

    int parent_index;
    auto convert_method_var = get_member_full_inheritance(parent_index, convertSTR);

    if( not convert_method_var )
    {
        return false;
    }


    utf8_string error_tmp;
    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( LHS_type );

    auto convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
    auto resolved_pair = convert_method_type->get_resolution(args_typs, error_tmp, cast_enum::pntr_casts);
    auto method_resolution = resolved_pair.first;

    if( method_resolution and (method_resolution->parameters->required_parameters.size() == 1) and (method_resolution->parameters->optional_parameters.size() == 0) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
//    auto convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
//    for( auto &instance : convert_method_type->overloads )
//    {
//        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
//        {
//            auto TYP = instance->parameters->required_parameters.front()->var_type;
//
//            bool is_ref;
//            auto ref_typ = TYP->is_reference_type( is_ref );
//            if(is_ref)
//            {
//                if( (not ref_typ) or LHS_type->can_get_pointer(ref_typ.get()) ) // paremater can be ANYTHING, or argument can cast to param
//                {
//                    return true;
//                }
//            }
//            // else? should always be a REF
//        }
//    }
//    return false;


void DefClassType::write_implicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual)
{
    utf8_string convertSTR("__convert__");

    int parent_index;
    auto convert_method_var = get_member_full_inheritance(parent_index, convertSTR);

    if( not convert_method_var )
    {
        throw gen_exception("cannot find overload in DefClassType::write_implicit_castTo. This should not be reached!");
    }

    C_expression_ptr exp_to_use;
    bool clean_exp_to_use = false;
    if( parent_index != -1 )
    {
        exp_to_use = write_parent_access( parent_index, RHS_exp, output );
        clean_exp_to_use = true;
    }
    else
    {
        exp_to_use = RHS_exp;
        clean_exp_to_use = false;
    }


    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( LHS_exp->cyth_type );

    vector<C_expression_ptr> argument_Cexpressions({ LHS_exp });

    auto as_method = convert_method_var->var_type->as_method();
    auto c_exp = as_method->parental_write_call(args_typs, exp_to_use, argument_Cexpressions, output, call_virtual, cast_enum::pntr_casts);

    output->out_strm()  <<  output->ln_strt  << c_exp->get_C_expression() << ';' << endl;
    c_exp->write_cleanup( output );
    if( clean_exp_to_use )
    {
        exp_to_use->write_cleanup( output );
    }



//    C_expression_ptr exp_to_use;
//    bool clean_exp_to_use = false;
//    if( parent_index != -1 )
//    {
//        exp_to_use = write_parent_access( parent_index, LHS_exp, output );
//        clean_exp_to_use = true;
//    }
//    else
//    {
//        exp_to_use = LHS_exp;
//        clean_exp_to_use = false;
//    }
//
//
//    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
//    args_typs->unnamed_argument_types.push_back( LHS_exp->cyth_type );
//
//    vector<C_expression_ptr> argument_Cexpressions({ LHS_exp });
//
//    auto as_method = convert_method_var->var_type->as_method();
//    auto c_exp = as_method->parental_write_call(args_typs, exp_to_use, argument_Cexpressions, output, call_virtual, cast_enum::pntr_casts);
//
//    output->out_strm()  <<  output->ln_strt  << c_exp->get_C_expression() << ';' << endl;
//    c_exp->write_cleanup( output );
//    if( clean_exp_to_use )
//    {
//        exp_to_use->write_cleanup( output );
//    }
}



bool DefClassType::can_explicit_castTo(varType_ptr LHS_type)
{
    if( can_implicit_castTo(LHS_type) )
    {
        return true;
    }

    utf8_string convertSTR("__exConvert__");

    int parent_index;
    auto convert_method_var = get_member_full_inheritance(parent_index, convertSTR);

    if( not convert_method_var )
    {
        return false;
    }

    utf8_string error_tmp;
    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( LHS_type );

    auto convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
    auto resolved_pair = convert_method_type->get_resolution(args_typs, error_tmp, cast_enum::pntr_casts);
    auto method_resolution = resolved_pair.first;

    if( method_resolution and (method_resolution->parameters->required_parameters.size() == 1) and (method_resolution->parameters->optional_parameters.size() == 0) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
//    auto convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
//    for( auto &instance : convert_method_type->overloads )
//    {
//        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
//        {
//            auto TYP = instance->parameters->required_parameters.front()->var_type;
//
//            bool is_ref;
//            auto ref_typ = TYP->is_reference_type( is_ref );
//            if(is_ref)
//            {
//                if( (not ref_typ) or LHS_type->can_get_pointer(ref_typ.get()) ) // paremater can be ANYTHING, or argument can cast to param
//                {
//                    return true;
//                }
//            }
//            // else? should always be a REF
//        }
//    }
//    return false;





void DefClassType::write_explicit_castTo(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                Csource_out_ptr output, bool call_virtual)
{
    if( can_implicit_castTo(LHS_exp->cyth_type) )
    {
        write_implicit_castTo(LHS_exp, RHS_exp, output, call_virtual);
        return;
    }


    utf8_string convertSTR("__exConvert__");

    int parent_index;
    auto convert_method_var = get_member_full_inheritance(parent_index, convertSTR);
    if( not convert_method_var )
    {
        throw gen_exception("cannot find overload in DefClassType::write_explicit_castTo. This should not be reached!");
    }

//    utf8_string expression_to_use = RHS_exp->get_C_expression();
//    if( parent_index != -1 )
//    {
//        expression_to_use = write_parent_access( parent_index, expression_to_use, output );
//    }
//
//    auto var_name_writer = make_shared< MethodType::method_access_writer >(expression_to_use, shared_from_this(), call_virtual);

    C_expression_ptr exp_to_use;
    bool clean_exp_to_use = false;
    if( parent_index != -1 )
    {
        exp_to_use = write_parent_access( parent_index, RHS_exp, output );
        clean_exp_to_use = true;
    }
    else
    {
        exp_to_use = RHS_exp;
        clean_exp_to_use = false;
    }

    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( LHS_exp->cyth_type );

    vector<C_expression_ptr> argument_Cexpressions({ LHS_exp });

    auto as_method = convert_method_var->var_type->as_method();
    auto c_exp = as_method->parental_write_call(args_typs, exp_to_use, argument_Cexpressions, output, call_virtual, cast_enum::pntr_casts);

    output->out_strm()  <<  output->ln_strt  << c_exp->get_C_expression() << ';' << endl;
    c_exp->write_cleanup( output );
    if(  clean_exp_to_use )
    {
        exp_to_use->write_cleanup( output );
    }

}



/// CALLING ///
varType_ptr DefClassType::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg,
                                    cast_enum cast_behavior)
{
    utf8_string callSTR("__call__");

    int parent_index;
    auto call_method_var = get_member_full_inheritance(parent_index, callSTR);

    if( not call_method_var )
    {
        error_msg = "type " + definition_name + " cannot be called";
        return nullptr;
    }
    else
    {
        return call_method_var->var_type->is_callable( argument_types, error_msg, cast_behavior );
    }
}

C_expression_ptr DefClassType::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                    cast_enum cast_behavior)
{
    utf8_string callSTR("__call__");

    int parent_index;
    auto call_method_var = get_member_full_inheritance(parent_index, callSTR);

    if( not call_method_var )
    {
        throw gen_exception("cannot find overload in DefClassType::write_call. This should not be reached!");
    }
    else
    {
//        utf8_string expression_to_use = LHS_Cexp->get_C_expression();
//        if( parent_index != -1 )
//        {
//            expression_to_use = write_parent_access( parent_index, expression_to_use, output );
//        }
//        auto var_name_writer = make_shared< MethodType::method_access_writer >(expression_to_use, shared_from_this(), call_virtual);

        C_expression_ptr exp_to_use;
        bool clean_exp_to_use = false;
        if( parent_index != -1 )
        {
            exp_to_use = write_parent_access( parent_index, LHS_Cexp, output );
            clean_exp_to_use = true;
        }
        else
        {
            exp_to_use = LHS_Cexp;
            clean_exp_to_use = false;
        }


        auto as_method = call_method_var->var_type->as_method();
        auto writer = as_method->parental_write_call(argument_types, exp_to_use, argument_Cexpressions, output, call_virtual, cast_behavior );

        if( clean_exp_to_use )
        {
            writer->add_cleanup_child( exp_to_use );
        }
        return writer;
    }
}


/// BINARY OPERATORS ///
varType_ptr DefClassType::get_has_binOperator(varType_ptr type, const char* op_func_name)
{
    utf8_string opSTR(op_func_name);

    int parent_index;
    auto method_var = get_member_full_inheritance(parent_index, opSTR);

    if( not method_var )
    {
        return nullptr;
    }

    utf8_string error_tmp;
    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( type );

    auto method_type = dynamic_pointer_cast<MethodType>( method_var->var_type );
    auto resolved_pair = method_type->get_resolution(args_typs, error_tmp, cast_enum::implicit_casts);
    auto method_resolution = resolved_pair.first;

    if( method_resolution and (method_resolution->parameters->required_parameters.size() == 1) and (method_resolution->parameters->optional_parameters.size() == 0) )
    {
        return method_resolution->return_type;
    }
    else
    {
        return nullptr;
    }

}

C_expression_ptr DefClassType::write_binOperator(C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                          Csource_out_ptr output, const char* op_func_name, bool call_virtual, bool is_LHS)
{
    utf8_string opSTR(op_func_name);

    C_expression_ptr self_exp;
    C_expression_ptr other_exp;
    if(is_LHS)
    {
        self_exp = LHS_exp;
        other_exp = RHS_exp;
    }
    else
    {
        self_exp = RHS_exp;
        other_exp = LHS_exp;
    }



    int parent_index;
    auto method_var = get_member_full_inheritance(parent_index, opSTR);

    if( not method_var )
    {
        throw gen_exception("cannot find method ", opSTR, " in DefClassType. This should not be reached!");
    }

    C_expression_ptr exp_to_use;
    bool clean_exp_to_use = false;
    if( parent_index != -1 )
    {
        exp_to_use = write_parent_access( parent_index, self_exp, output );
        clean_exp_to_use = true;
    }
    else
    {
        exp_to_use = self_exp;
        clean_exp_to_use = false;
    }


    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( other_exp->cyth_type );

    vector<C_expression_ptr> argument_Cexpressions({ other_exp });

    auto method_type = method_var->var_type->as_method();
    auto c_exp = method_type->parental_write_call(args_typs, exp_to_use, argument_Cexpressions, output, call_virtual);


    if( clean_exp_to_use )
    {
        c_exp->add_cleanup_child( exp_to_use );
    }
    return c_exp;
}


varType_ptr DefClassType::get_has_cmp(varType_ptr type)
{
    utf8_string opSTR("__cmp__");

    int parent_index;
    auto method_var = get_member_full_inheritance(parent_index, opSTR);

    if( not method_var )
    {
        return nullptr;
    }

    utf8_string error_tmp;
    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( type );

    auto method_type = dynamic_pointer_cast<MethodType>( method_var->var_type );
    auto resolved_pair = method_type->get_resolution(args_typs, error_tmp, cast_enum::implicit_casts);
    auto method_resolution = resolved_pair.first;

    // check we have method with right respolution
    if( method_resolution and (method_resolution->parameters->required_parameters.size() == 1) and (method_resolution->parameters->optional_parameters.size() == 0) )
    {
        varType_ptr toCtyp = method_resolution->return_type->can_toC(); // check result can convert to C. Should check is c-int, but this is very hard
        if(not toCtyp)
        {
            return nullptr;
        }
        else
        {
        // now we check if bool is availble, or use c-type

            utf8_string bool_name("bool");
            location_span loc;
            bool check_order = false;
            auto bool_type = this->class_symbol_table->get_type_global(bool_name, loc, check_order);

            C_expression_ptr return_exp;
            if(bool_type)
            {
                return bool_type;
            }
            else
            {
                utf8_string C_name("UNNAMED_C_TYPE");
                check_order = false;
                auto C_type = this->class_symbol_table->get_type_global(C_name, loc, check_order);
                return C_type;
            }
        }
    }
    else
    {
        return nullptr;
    }
}

// opperation must be int, becouse  binOperator_expression_AST_node::expression_type  not defined in header
C_expression_ptr DefClassType::write_compOperator( C_expression_ptr LHS_exp, C_expression_ptr RHS_exp,
                          Csource_out_ptr output, char operation, bool call_virtual, bool is_LHS)
{  // operations: < , >, !, =, L, G respectively: lessthan, greaterthan, not equal, equal to,  <=, and >=

    utf8_string opSTR("__cmp__");

    C_expression_ptr self_exp;
    C_expression_ptr other_exp;
    if(is_LHS)
    {
        self_exp = LHS_exp;
        other_exp = RHS_exp;
    }
    else
    {
        self_exp = RHS_exp;
        other_exp = LHS_exp;
    }

    int parent_index;
    auto method_var = this->get_member_full_inheritance(parent_index, opSTR);

    if( not method_var )
    {
        throw gen_exception("cannot find method __cmp__ in DefClassType. This should not be reached!");
    }

    C_expression_ptr exp_to_use;
    bool clean_exp_to_use = false;
    if( parent_index != -1 )
    {
        exp_to_use = this->write_parent_access( parent_index, self_exp, output );
        clean_exp_to_use = true;
    }
    else
    {
        exp_to_use = self_exp;
        clean_exp_to_use = false;
    }


    function_argument_types_ptr args_typs = make_shared<function_argument_types>();
    args_typs->unnamed_argument_types.push_back( other_exp->cyth_type );

    vector<C_expression_ptr> argument_Cexpressions({ other_exp });

    // get __cmp__
    auto method_type = method_var->var_type->as_method();
    auto cmp_exp = method_type->parental_write_call(args_typs, exp_to_use, argument_Cexpressions, output, call_virtual);

    // convert to c int
    auto cmpToC_exp = cmp_exp->cyth_type->toC(cmp_exp, output);
    utf8_string var_name( "_cyTMP__cmp__" );
    var_name += output->get_unique_string();
    (output->out_strm()) << (output->ln_strt) << "int " << var_name << " = " <<  cmpToC_exp->get_C_expression() << ';' << endl;



    // flip op if necisary
    char op_to_use = operation;
    if( not is_LHS )
    {
        switch( operation )
        {
        case '<':
            op_to_use = '>';
            break;
        case '>':
            op_to_use = '<';
            break;
        case '=':
            break;
        case '!':
            break;
        case 'L':
            op_to_use = 'G';
            break;
        case 'G':
            op_to_use = 'L';
            break;
        default:
            throw gen_exception("bad operation(A) in  DefClassType::write_compOperator. op:", operation);
        }
    }

    // apply op
    utf8_string C_operation;
    switch( op_to_use )
    {
    case '<':
        C_operation = var_name + "== -1";
        break;
    case '>':
        C_operation = var_name + "== 1";
        break;
    case '=':
        C_operation = var_name + "== 0";
        break;
    case '!':
        C_operation = var_name + "!= 0";
        break;
    case 'L':
        C_operation = (var_name + "== -1) || (") + (var_name + "== 0") ;
        break;
    case 'G':
        C_operation = (var_name + "== 1) || (") + (var_name + "== 0") ;
        break;
    default:
        throw gen_exception("bad operation(B) in  DefClassType::write_compOperator op:", operation);
    }

    // convert c_int to bool if possible
    utf8_string bool_name("bool");
    location_span loc;
    bool check_order = false;
    auto bool_type = this->class_symbol_table->get_type_global(bool_name, loc, check_order);

    C_expression_ptr return_exp;
    if(bool_type)
    {
        auto output_name = "__cy__tmp_" + output->get_unique_string();

        bool_type->C_definition_name( output_name, output );
        output->out_strm() << ';' << endl;
        bool_type->initialize( output_name, output );

        return_exp = make_shared<owned_name>( bool_type, output_name );


        bool_type->write_default_constructor(output_name, output);

        utf8_string member("__val__");
        auto val_setter = bool_type->write_member_getref( return_exp, member, output );
        output->out_strm() <<  output->ln_strt << val_setter->get_C_expression() << "= (" << C_operation << ");" << endl;
        val_setter->write_cleanup( output );

    }
    else
    {
        utf8_string C_name("UNNAMED_C_TYPE");
        check_order = false;
        auto C_type = this->class_symbol_table->get_type_global(C_name, loc, check_order);

        C_operation = "(" + C_operation + ")";

        return_exp = make_shared<simple_C_expression>( C_operation, C_type, false, false );

    }

    // cleanup and return
    if( clean_exp_to_use )
    {
        exp_to_use->write_cleanup( output );
    }
    cmp_exp->write_cleanup( output );
    cmpToC_exp->write_cleanup( output );

    return return_exp;
}







/// C-interface
varType_ptr DefClassType::can_toC()
{
    utf8_string callSTR("__toC__");

    int parent_index;
    auto method_var = get_member_full_inheritance(parent_index, callSTR);

    utf8_string error_msg;
    if( not method_var )
    {
        //error_msg = "type " + definition_name + " cannot be converted to a C-type (does not have a __toC__ method.";
        return nullptr;
    }
    else
    {
        auto argument_types = make_shared<function_argument_types>();
        return method_var->var_type->is_callable( argument_types, error_msg );
    }
}


C_expression_ptr DefClassType::toC(C_expression_ptr exp, Csource_out_ptr output, bool call_virtual)
{
    utf8_string callSTR("__toC__");

    int parent_index;
    auto method_var = get_member_full_inheritance(parent_index, callSTR);

    if( not method_var )
    {
        throw gen_exception("cannot find overload in DefClassType::toC. This should not be reached!");
    }
    else
    {

        C_expression_ptr exp_to_use;
        bool clean_exp_to_use = false;
        if( parent_index != -1 )
        {
            exp_to_use = write_parent_access( parent_index, exp, output );
            clean_exp_to_use = true;
        }
        else
        {
            exp_to_use = exp;
            clean_exp_to_use = false;
        }


        auto argument_types = make_shared<function_argument_types>();
        vector<C_expression_ptr> argument_Cexpressions;

        auto as_method = method_var->var_type->as_method();
        auto writer = as_method->parental_write_call(argument_types, exp_to_use, argument_Cexpressions, output, call_virtual );

        if( clean_exp_to_use )
        {
            writer->add_cleanup_child( exp_to_use );
        }
        return writer;
    }
}






/// GET METHODS ///
MethodType_ptr DefClassType::get_method(utf8_string &name)
{
    csu::location_span tmp_loc;
    bool ordered = false;
    auto variable = class_symbol_table->get_variable_local(name, tmp_loc, ordered);
    auto var_type = variable->var_type;
    if( not ordered )
    {
        return nullptr; // only not ordered if there is a problem..... other than ordering...
    }
    else if( var_type->type_of_type == varType::method_function_t )
    {
        return var_type->as_method();
    }
    else
    {
        return nullptr;
    }
}

/// METHOD ITERATOR ///
//MethodType_ptr DefClassType::methodIter::get()
//{
//    return internal_iter->second->var_type->as_method();
//}
//
//DefClassType::methodIter& DefClassType::methodIter::operator++()
//{
//    ++internal_iter;
//    while( internal_iter != end_iter )
//    {
//        if( internal_iter->second->var_type and (internal_iter->second->var_type->type_of_type == varType::method_function_t )) // this only works if method type is defined. Problem?
//        {
//            break;
//        }
//        ++internal_iter;
//    }
//
//    return *this;
//}
//
//bool DefClassType::methodIter::operator!=(const methodIter& RHS)
//{
//    return internal_iter != RHS.internal_iter;
//}
//
//bool DefClassType::methodIter::operator==(const methodIter& RHS)
//{
//    return internal_iter == RHS.internal_iter;
//}
//
//DefClassType::methodIter DefClassType::methodIter_begin()
//{
//    methodIter ret;
//    ret.internal_iter = class_symbol_table->variable_table.begin();
//    ret.end_iter = class_symbol_table->variable_table.end();
//
//    if( not (ret.internal_iter->second->var_type->type_of_type == varType::method_function_t) )
//    {
//        // we expect the iterator to always be valid, or at the end;
//        ++ret;
//    }
//
//    return ret;
//}
//
//DefClassType::methodIter DefClassType::methodIter_end()
//{
//    methodIter ret;
//    ret.internal_iter = class_symbol_table->variable_table.end();
//    ret.end_iter = class_symbol_table->variable_table.end();
//    return ret;
//}

/// METHOD OVERLOAD ITERATOR ///
MethodType_ptr DefClassType::methodOverloadIter::method_get()
{
    return method_iter->second->var_type->as_method();
}

MethodType::ResolvedMethod_ptr DefClassType::methodOverloadIter::overload_get()
{
    return (*overload_iter);
}

DefClassType::methodOverloadIter& DefClassType::methodOverloadIter::operator++()
{
    if( method_iter ==  method_end_iter)
    {
        return *this;
    }

    if( method_iter->second->var_type and (method_iter->second->var_type->type_of_type == varType::method_function_t) and (overload_iter != overload_end_iter ) )
    {
        auto method_self_type = method_get()->self_ptr_name->var_type;

        ++overload_iter;
        while( (overload_iter != overload_end_iter) and not ( do_inhereted_overloads or method_self_type->is_equivalent( (*overload_iter)->self_ptr_name->var_type ) ) )
        {
            ++overload_iter;
        }

        if( overload_iter != overload_end_iter )
        {
            return *this;
        }
    }

    // if am here, then increment method iterator and try again
    ++method_iter;
    if( (method_iter != method_end_iter) and method_iter->second->var_type and (method_iter->second->var_type->type_of_type == varType::method_function_t ) )
    {
        // yay!
        auto method = method_get();
        overload_iter = method->overloads.begin();
        overload_end_iter = method->overloads.end();

        auto method_self_type = method_get()->self_ptr_name->var_type;
        while( (overload_iter != overload_end_iter) and not ( do_inhereted_overloads or method_self_type->is_equivalent( (*overload_iter)->self_ptr_name->var_type ) ) )
        {
            ++overload_iter;
        }

        if( overload_iter != overload_end_iter )
        {
            return *this;
        }
        else
        {
            return (++(*this));
        }
    }
    else
    {
        return (++(*this));
    }
}

bool DefClassType::methodOverloadIter::operator!=(const methodOverloadIter& RHS)
{
    return not (*this == RHS);
}

bool DefClassType::methodOverloadIter::operator==(const methodOverloadIter& RHS)
{
    if( method_iter == RHS.method_iter )
    {
        if( (method_iter==method_end_iter) or (overload_iter==RHS.overload_iter) )
        {
            return true;
        }
    }
    return false;
}

DefClassType::methodOverloadIter DefClassType::methodOverloadIter_begin(bool do_inhereted_overloads)
{
    methodOverloadIter ret;
    ret.method_iter = class_symbol_table->variable_table.begin();
    ret.method_end_iter = class_symbol_table->variable_table.end();
    ret.do_inhereted_overloads = do_inhereted_overloads;

    if( ret.method_iter->second->var_type->type_of_type == varType::method_function_t )
    {
        auto method = ret.method_get();
        ret.overload_iter = method->overloads.begin();
        ret.overload_end_iter = method->overloads.end();
    }
    else
    {
        ++ret; // should find next method, and set the overload iterator
    }

    return ret;
}

DefClassType::methodOverloadIter DefClassType::methodOverloadIter_end()
{
    methodOverloadIter ret;
    ret.method_iter = class_symbol_table->variable_table.end();
    ret.method_end_iter = class_symbol_table->variable_table.end();
    return ret;
}


/// iterator over parents! ///
int DefClassType::get_parent_index( ClassType_ptr parent_class )
//// return the index in inheritance tree of the parent, -1 if this class doesn't inherit from parent
{
    int i = 0;
    for(auto &par_class : full_inheritance_tree)
    {
        if( par_class->is_equivalent( parent_class ) )
        {
            return i;
        }
        ++i;
    }
    return -1;
}

int DefClassType::get_immediate_parent_index( ClassType_ptr parent_class )
// only searches immediate parents, -1 if not immediate parent
{
    int i = 0;
    for(auto &par_class : top_level_inheritances)
    {
        if( par_class->is_equivalent( parent_class ) )
        {
            return i;
        }
        ++i;
    }
    return -1;
}

ClassType_ptr DefClassType::parentIter::get()
{
    if( current_parent_index==-2 )
    {
        return nullptr;
    }
    else if( current_parent_index==-1 )
    {
        return self_type;
    }
    else
    {
        return self_type->full_inheritance_tree[ current_parent_index ];
    }
}

int DefClassType::parentIter::get_parent_index()
{
    return current_parent_index;
}

DefClassType::parentIter& DefClassType::parentIter::operator++()
{
    if( current_parent_index==-2 )
    {
    }
    else if( current_parent_index==-1 )
    {
        current_parent_index = -2;
    }
    else
    {
        current_parent_index = self_type->inheritance_tree_level[ current_parent_index ];
    }

    return *this;
}

bool DefClassType::parentIter::operator!=(const parentIter& RHS)
{
    return current_parent_index != RHS.current_parent_index;
}

bool DefClassType::parentIter::operator==(const parentIter& RHS)
{
    return current_parent_index == RHS.current_parent_index;
}

DefClassType::parentIter DefClassType::parentIter_begin( int parent_index )
{
    parentIter ret;
    ret.current_parent_index = parent_index;
    ret.self_type = as_class();
    return ret;
}

DefClassType::parentIter DefClassType::parentIter_end()
{
    parentIter ret;
    ret.current_parent_index = -2;
    ret.self_type = as_class();
    return ret;
}

C_expression_ptr DefClassType::write_parent_access(int parent_index, C_expression_ptr exp, Csource_out_ptr output)
{
    auto parent_iter = parentIter_begin(parent_index);
    auto parent_iter_end = parentIter_end();

    utf8_string ret;
    auto previous_parent = parent_iter.get();
    ++ parent_iter;
    for( ; parent_iter!=parent_iter_end; ++parent_iter)
    {
        auto current_class = parent_iter.get();
        int immediate_parent_index = current_class->get_immediate_parent_index( previous_parent );
        auto varname = current_class->parent_class_names[ immediate_parent_index ];

        ret = "." + varname->C_name + ret;

        previous_parent = current_class;
    }

    ret = "(" + exp->get_C_expression() + ")" + ret;

    auto ret_exp = make_shared<simple_C_expression>( ret, previous_parent, true, false );
    return ret_exp;
}





// method function type
MethodType::MethodType(utf8_string name, location_span _loc, varName_ptr _self_ptr_name)
{
    loc = _loc; // what??? this doesn't HAVE a location!!
    definition_name = name;
    C_name = "";
    is_ordered = false;
    type_of_type = varType::method_function_t;
    num_undefined_overloads = 0; // no overloads at all!
    self_ptr_name = _self_ptr_name;
    type_of_method = MethodType::normal; // this can be altered externally
}

MethodType::ResolvedMethod_ptr MethodType::define_overload(location_span defining_loc, func_param_ptr parameters, sym_table_base* defining_scope)
{
    // first check that new overload does not conflict with current overloads
    for(auto OL : overloads)
    {
        // first check if distinguish by signiture
        if( OL->parameters->is_distinguishable( parameters ) )
        {
            continue;// can distinguish by signiture
        }
        else
        {
            cout << "method defined twice in same scope " << OL->define_loc << " and " << defining_loc << endl;
            return nullptr; // cannot distinguish between functions!
        }
    }

    ResolvedMethod_ptr new_overload = make_shared<resolved_method>( );
    new_overload->is_virtual = true;
    new_overload->define_loc = defining_loc;
    new_overload->c_reference = defining_scope->namespace_unique_name + "__";
    new_overload->c_reference += definition_name;
    new_overload->c_reference += utf8_string("__");
    new_overload->c_reference += defining_scope->get_unique_string();
    new_overload->return_type = nullptr;
    new_overload->parameters = parameters;
    new_overload->self_ptr_name = self_ptr_name;
    new_overload->overriden_method = nullptr;

    num_undefined_overloads += 1; // now need to define its return type.
    overloads.push_back( new_overload );
    return new_overload;
}

void MethodType::define_overload_return(ResolvedMethod_ptr overload,  varType_ptr _return_type)
{
    if( overload->return_type )
    {
        throw gen_exception("overload of method return type defined twice!!! in MethodType::define_overload_return");
    }

    overload->return_type = _return_type;
    num_undefined_overloads -= 1;
}

//varType_ptr MethodType::copy( utf8_string _definition_name )
//{
//    if( _definition_name == "" )
//    {
//        _definition_name = definition_name;
//    }
//
//    auto new_var = make_shared<MethodType>( _definition_name, loc, self_ptr_name );
//    new_var->num_undefined_overloads = num_undefined_overloads;
//
//    return new_var;
//}

 /// CALLING ///

MethodType::resolved_pair MethodType::get_resolution(function_argument_types_ptr argument_types, utf8_string& error_msg, cast_enum cast_behavior)
{

    ResolvedMethod_ptr current_overload = nullptr;
    shared_ptr<parameter_to_arguments_mapper> current_argument_map = nullptr;

    list< pair<ResolvedMethod_ptr, utf8_string> > attempted_resolutions;

    for(auto OL : overloads)
    {

        shared_ptr<parameter_to_arguments_mapper> new_argument_map = make_shared<parameter_to_arguments_mapper>(OL->parameters, argument_types, cast_behavior);

        if( not new_argument_map->is_good )// cannot map these arguments to these parameters
        {
            attempted_resolutions.push_back( make_pair( OL, new_argument_map->error ) );
            continue;
        }


        if( not current_overload ) // can map, and no competitors
        {
            current_overload = OL;
            current_argument_map = new_argument_map;
        }
        else
        // OOPS! these two conflict, need to find which one is better
        {
            // check signiture
            int cmp_sig = new_argument_map->comparison( current_argument_map );
            if( cmp_sig==1 )
            { // prefer the new one
                current_overload = OL;
                current_argument_map = new_argument_map;
            }
            else if( cmp_sig == -1)
            {
                // do nothing!
                // present one is prefered!
            }
            else
            {
                //???
            }
        }
    }

    if( not current_overload )
    {// no resolution found
        stringstream TMP;
        TMP << "cannot resolve overload with arguments: " ;
        argument_types->print( TMP );
        //TMP << " at " << argument_node_ptr->loc ;
        TMP << endl;
        for( auto P : attempted_resolutions )
        {
            TMP << "overload defined " << P.first->define_loc << " with parameters ";
            P.first->parameters->print( TMP );
            TMP << endl;
            TMP << "  is invalid because " << P.second << endl;
        }
        auto TMP2 = TMP.str();
        error_msg = TMP2;
        return resolved_pair();
    }
    else
    {
        return make_pair(current_overload, current_argument_map);
    }
}

//MethodType::resolved_pair MethodType::get_resolution(vector<expression_AST_node*> &argument_expressions, sym_table_base* _symbol_table, csu::utf8_string& error_msg)
//{
//
//    ResolvedMethod_ptr current_overload = nullptr;
//    shared_ptr<parameter_to_arguments_mapper> current_argument_map = nullptr;
//
//    list< pair<ResolvedMethod_ptr, utf8_string> > attempted_resolutions;
//
//    for(auto OL : overloads)
//    {
//
//        shared_ptr<parameter_to_arguments_mapper> new_argument_map = make_shared<parameter_to_arguments_mapper>(OL->parameters, argument_expressions, _symbol_table);
//
//        if( not new_argument_map->is_good )// cannot map these arguments to these parameters
//        {
//            attempted_resolutions.push_back( make_pair( OL, new_argument_map->error ) );
//            continue;
//        }
//
//
//        if( not current_overload ) // can map, and no competitors
//        {
//            current_overload = OL;
//            current_argument_map = new_argument_map;
//        }
//        else
//        // OOPS! these two conflict, need to find which one is better
//        {
//            // check signiture
//            int cmp_sig = new_argument_map->comparison( current_argument_map );
//            if( cmp_sig==1 )
//            { // prefer the new one
//                current_overload = OL;
//                current_argument_map = new_argument_map;
//            }
//            else if( cmp_sig == -1)
//            {
//                // do nothing!
//                // present one is prefered!
//            }
//            else
//            {
//                //???
//            }
//        }
//    }
//
//    if( not current_overload )
//    {// no resolution found
//        stringstream TMP;
//        TMP << "cannot resolve overload with arguments: (" ;
//        for(auto &exp : argument_expressions)
//        {
//            TMP << exp->expression_return_type << ", ";
//        }
//        TMP << ") " << endl;
//        for( auto P : attempted_resolutions )
//        {
//            TMP << "overload defined " << P.first->define_loc << " with parameters ";
//            P.first->parameters->print( TMP );
//            TMP << endl;
//            TMP << "  is invalid because " << P.second << endl;
//        }
//        auto TMP2 = TMP.str();
//        error_msg = TMP2;
//        return resolved_pair();
//    }
//    else
//    {
//        return make_pair(current_overload, current_argument_map);
//    }
//}

varType_ptr MethodType::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg,
                                    cast_enum cast_behavior)
{
    auto OL = get_resolution( argument_types, error_msg, cast_behavior );
    if(OL.first)
        return OL.first->return_type;
    else
        return nullptr;
}


C_expression_ptr MethodType::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
            vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                    cast_enum cast_behavior)
    // NOTE: call_virtual doesn't do anything, virtuality is controlled via LHS_Cexp
{
    auto method_access = dynamic_pointer_cast<MethodType::method_access_writer>( LHS_Cexp );

    auto parent_class_CExp = method_access->parent_exp;
    bool virtual_call_allowed = method_access->virtual_call_allowed;

    return parental_write_call( argument_types, parent_class_CExp, argument_Cexpressions, output, virtual_call_allowed, cast_behavior );
}


C_expression_ptr MethodType::parental_write_call(function_argument_types_ptr argument_types, C_expression_ptr parent_Cexp,
            vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                    cast_enum cast_behavior)
{

    // note that LHS_Cexp is reference to the class object..I think

    //// PREP ////
    utf8_string tmp;
    auto OV = get_resolution( argument_types, tmp, cast_behavior );
    auto func = OV.first;

    if( not func)
    {
        throw gen_exception("cannot find method resolution in MethodType::write_call This should not be reached!\n",
                            "    method: ", definition_name, '\n',
                            " err:", tmp);
        return nullptr;
    }

    stringstream out;

    out << '(' ;



    //// write initial method accsess and pass self class ////
    auto parent_class_strCexp = parent_Cexp->get_C_expression();

    //auto ret = make_shared<simple_C_expression>("", func->return_type, false, true);
    list< shared_ptr<C_expression> > child_expressions;

    if( func->is_virtual and call_virtual )
    {
        bool tmp;
        auto self_type = self_ptr_name->var_type->is_reference_type(tmp)->as_class(); // this is very clumsy


        if( func->overriden_method )
        {
            utf8_string method_name = func->overriden_method->c_reference;
            auto class_Cexp = self_type->write_parent_access(func->parental_vtable_location, parent_Cexp, output );
            //ret->add_cleanup_child( class_Cexp );
            child_expressions.push_back(class_Cexp);
            auto class_exp = class_Cexp->get_C_expression();

            utf8_string vtable_entry = "(" + class_exp + ").__cy_vtable->" + method_name ;
            utf8_string self_ptr_exp = "((void*)&("+class_exp+"))-" + vtable_entry+"_offset";

            out << "(" << vtable_entry << ")( " << self_ptr_exp;
        }
        else
        {
            utf8_string method_name = func->c_reference;
            utf8_string vtable_entry = "(" + parent_class_strCexp + ").__cy_vtable->" + method_name ;
            utf8_string self_ptr_exp = "((void*)&("+parent_class_strCexp+"))-" + vtable_entry+"_offset";

            out << "(" << vtable_entry << ")( " << self_ptr_exp;
        }
    }
    else
    {
        out << func->c_reference << endl;
        out << "( (void*)&(" << parent_class_strCexp << ") ";
    }



    //// write params ////
    vector<C_expression_ptr> out_expressions;
    OV.second->write_arguments( argument_Cexpressions, out_expressions, output);
    for(auto &param : out_expressions)
    {
        out << ", " << param->get_C_expression();
        //ret->add_cleanup_child( param );
        child_expressions.push_back(param);
    }

    out << "))" ;


    //// return ////
    //ret->exp = out.str();
    C_expression_ptr RET;
    if( func->return_type->can_be_defined() and func->return_type->type_of_type!=varType::empty ) //empty is because of the void type
    {
        utf8_string new_tmp_vname = "__cy__tmp_";
        new_tmp_vname += output->get_unique_string();

        (output->out_strm()) << (output->ln_strt);
        func->return_type->C_definition_name(new_tmp_vname, output); // note this is destructed inside of, at the end of , the function.
        (output->out_strm()) << ';'<< endl;
        func->return_type->initialize(new_tmp_vname, output);

        (output->out_strm()) << (output->ln_strt) << new_tmp_vname << " = " << out.str() << ';' << endl;

        func->return_type->inform_moved(new_tmp_vname, output);

        RET = make_shared<owned_name>( func->return_type,  new_tmp_vname);
    }
    else
    {
        output->out_strm() << output->ln_strt << out.str() << ';' << endl;
        RET = make_shared<simple_C_expression>("", make_shared<void_type>(), false, false);
    }


    for(auto& W : child_expressions)
    {
        RET->add_cleanup_child(W);
    }


    return RET;
}





// I don't know why this is here.
//exp_writer_ptr MethodType::write_call(ResolvedMethod_ptr resolved_method, vector<expression_AST_node*> &argument_expressions,
//               sym_table_base* _symbol_table, exp_writer_ptr LHS_Cexp, vector<utf8_string>& argument_Cexpressions, ostream& output)
//{
//// note that LHS_Cexp is reference to the class object..I think
//
//    auto writer = make_shared<basic_function_call_writer>(output, resolved_method->parameters, argument_expressions, _symbol_table);
//
//    if( not writer->mapper.is_good )
//    {// no resolution found
//        cout<< "cannot resolve overload with arguments: " ;
//        writer->mapper.arguments.print( cout );
//        cout << endl;
//        cout << "overload defined " << resolved_method->define_loc << " with parameters ";
//        resolved_method->parameters->print( cout );
//        cout<<endl;
//        cout << "  is invalid because "<< writer->mapper.error <<endl;
//        cout << endl;
//        return nullptr;
//    }
//
//    vector< utf8_string > params_to_write;
//
//    writer->mapper.write_arguments(argument_Cexpressions, params_to_write, output);
//
//    stringstream out;
//    out << '(' ;
//
//    // it's annoying how much this duplicates from previous call
//
//    auto method_access = dynamic_pointer_cast<MethodType::method_access_writer>( LHS_Cexp );
//    auto parent_class_CExp = method_access->get_C_expression();
//    bool virtual_call_allowed = method_access->virtual_call_allowed;
//
//    if( resolved_method->is_virtual and virtual_call_allowed  )
//    {
//        bool tmp;
//        auto self_type = self_ptr_name->var_type->is_reference_type(tmp)->as_class(); // this is very clumsy
//
//
//        if( resolved_method->overriden_method )
//        {
//            utf8_string method_name = resolved_method->overriden_method->c_reference;
//            utf8_string class_exp = self_type->write_parent_access(resolved_method->parental_vtable_location, parent_class_CExp, output );
//            utf8_string vtable_entry = "(" + class_exp + ").__cy_vtable->" + method_name ;
//            utf8_string self_ptr_exp = "((void*)&("+class_exp+"))-" + vtable_entry+"_offset";
//
//            out << "(" << vtable_entry << ")( " << self_ptr_exp;
//        }
//        else
//        {
//            utf8_string method_name = resolved_method->c_reference;
//            utf8_string vtable_entry = "(" + parent_class_CExp + ").__cy_vtable->" + method_name ;
//            utf8_string self_ptr_exp = "((void*)&("+parent_class_CExp+"))-" + vtable_entry+"_offset";
//
//            out << "(" << vtable_entry << ")( " << self_ptr_exp;
//        }
//    }
//    else
//    {
//        out << resolved_method->c_reference << endl;
//        out << "( (void*)&(" << parent_class_CExp << ") ";
//    }
//
//
//    for(auto &param : params_to_write)
//    {
//        out << ", " << param;
//    }
//
//    out << "))" ;
//
//    writer->exp = out.str();
//
//    return writer;
//}

//
//exp_writer_ptr MethodType::write_call(vector<expression_AST_node*> &argument_expressions, sym_table_base* _symbol_table,
//                                    exp_writer_ptr LHS_Cexp, vector<utf8_string>& argument_Cexpressions, ostream& output)
//{
//// note that LHS_Cexp is reference to the class object..I think
//
//    utf8_string tmp;
//    auto OV = get_resolution( argument_expressions, _symbol_table, tmp );
//    if( not OV.first)
//        return nullptr;
//
//    auto resolved_method = OV.first;
//
//    auto writer = make_shared<basic_function_call_writer>(output, *OV.second);
//
//    if( not writer->mapper.is_good )
//    {// no resolution found
//        cout<< "cannot resolve overload with arguments: " ;
//        writer->mapper.arguments.print( cout );
//        cout << endl;
//        cout << "overload defined " << resolved_method->define_loc << " with parameters ";
//        resolved_method->parameters->print( cout );
//        cout<<endl;
//        cout << "  is invalid because "<< writer->mapper.error <<endl;
//        cout << endl;
//        return nullptr;
//    }
//
//    vector< utf8_string > params_to_write;
//
//    writer->mapper.write_arguments(argument_Cexpressions, params_to_write, output);
//
//    stringstream out;
//    out << '(' ;
//
//    // it's annoying how much this duplicates from previous call
//
//    auto method_access = dynamic_pointer_cast<MethodType::method_access_writer>( LHS_Cexp );
//    auto parent_class_CExp = method_access->get_C_expression();
//    bool virtual_call_allowed = method_access->virtual_call_allowed;
//
//    if( resolved_method->is_virtual and virtual_call_allowed  )
//    {
//        bool tmp;
//        auto self_type = self_ptr_name->var_type->is_reference_type(tmp)->as_class(); // this is very clumsy
//
//
//        if( resolved_method->overriden_method )
//        {
//            utf8_string method_name = resolved_method->overriden_method->c_reference;
//            utf8_string class_exp = self_type->write_parent_access(resolved_method->parental_vtable_location, parent_class_CExp, output );
//            utf8_string vtable_entry = "(" + class_exp + ").__cy_vtable->" + method_name ;
//            utf8_string self_ptr_exp = "((void*)&("+class_exp+"))-" + vtable_entry+"_offset";
//
//            out << "(" << vtable_entry << ")( " << self_ptr_exp;
//        }
//        else
//        {
//            utf8_string method_name = resolved_method->c_reference;
//            utf8_string vtable_entry = "(" + parent_class_CExp + ").__cy_vtable->" + method_name ;
//            utf8_string self_ptr_exp = "((void*)&("+parent_class_CExp+"))-" + vtable_entry+"_offset";
//
//            out << "(" << vtable_entry << ")( " << self_ptr_exp;
//        }
//    }
//    else
//    {
//        out << resolved_method->c_reference << endl;
//        out << "( (void*)&(" << parent_class_CExp << ") ";
//    }
//
//
//    for(auto &param : params_to_write)
//    {
//        out << ", " << param;
//    }
//
//    out << "))" ;
//
//    writer->exp = out.str();
//
//    return writer;
//}



void MethodType::resolved_method::write_C_prototype(Csource_out_ptr output)
{
//    stringstream OUT;
//    OUT << "(";
//    OUT << c_reference;
//    OUT << "(";
//
//    //self_ptr_name->var_type->C_definition_name(self_ptr_name->C_name, OUT);
//    OUT << "void* " << self_ptr_name->C_name << "_";
//    if( parameters->total_size() > 0 )
//    {
//        OUT << ",";
//    }
//
//    parameters->write_to_C(OUT, false);
//    OUT << "))";
//
//    utf8_string name = OUT.str();
//    return_type->C_definition_name(name, output);

    utf8_string TMP("");
    return_type->C_definition_name(TMP, output);

    output->out_strm() << "(";
    output->out_strm() << c_reference;
    output->out_strm() << "(";

    output->out_strm() << "void* " << self_ptr_name->C_name << "_";

    if( parameters->total_size() > 0 )
    {
        output->out_strm() << ",";
    }

    parameters->write_to_C(output, false);
    output->out_strm()  << "))";
}

MethodType::ResolvedMethod_ptr MethodType::get_indestinguishable_overload(func_param_ptr parameters)
{
    for( auto& TST_overload : overloads)
    {
        if( not TST_overload->parameters->is_distinguishable( parameters ) )
        {
            return TST_overload;
        }
    }
    return nullptr;
}

bool MethodType::overload_was_inhereted(ResolvedMethod_ptr ovrld)
{
    return not self_ptr_name->var_type->is_equivalent( ovrld->self_ptr_name->var_type );
}

C_expression_ptr MethodType::get_C_exp(C_expression_ptr parent_exp, bool _virtual_call_allowed, bool own_exp)
{
    auto new_exp = make_shared<method_access_writer>();
    new_exp->can_be_referenced = false;
    new_exp->has_output_ownership = true;
    new_exp->cyth_type = shared_from_this();
    new_exp->virtual_call_allowed = _virtual_call_allowed;
    new_exp->parent_exp = parent_exp;

    if( own_exp )
    {
        new_exp->add_cleanup_child( parent_exp );
    }

    return new_exp;
}



/// metatype ///
// the type of type
MetaType::MetaType(varType_ptr _type)
{
    type = _type;
    type_of_type = varType::metatype_t;
    definition_name = "metatype";
    is_ordered = false;
}

bool MetaType::is_equivalent(varType_ptr _type)
{
    if( _type->type_of_type == varType::metatype_t )
    {
        auto RHS_metatype = _type->as_metatype();
        return type->is_equivalent( RHS_metatype->type );
    }
    else
    {
        return false;
    }
}

//varType_ptr MetaType::copy(csu::utf8_string _definition_name)
//{
//    return shared_from_this();
//}


varType_ptr MetaType::is_callable(function_argument_types_ptr argument_types, utf8_string& error_msg,
                                cast_enum cast_behavior)
{
    if( not type->can_be_defined() )
    {
        return nullptr;
    }

    if( not type->has_explicit_constructor( argument_types, error_msg))
    {
        error_msg += " \n";
        if( argument_types->unnamed_argument_types.size()==1 and argument_types->named_argument_types.size()==0 )
        {
            auto RHS_type = argument_types->unnamed_argument_types[0];
            if( RHS_type->can_explicit_castTo( type ) )
            {
                return type;
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        return type;
    }
}


C_expression_ptr MetaType::write_call(function_argument_types_ptr argument_types, C_expression_ptr LHS_Cexp,
        vector<C_expression_ptr>& argument_Cexpressions, Csource_out_ptr output, bool call_virtual,
                                cast_enum cast_behavior)
{
    // first, define and initiate new variable
    utf8_string new_var = "__TMP_VAR_" + output->get_unique_string();

    type->C_definition_name(new_var, output);
    output->out_strm() << ';' << endl;
    type->initialize(new_var, output);

    //auto var_C_exp = make_shared<simple_C_expression>(new_var, type);
    auto var_C_exp = make_shared<owned_name>(type, new_var);

    // then construct
    utf8_string TMP;
    if( type->has_explicit_constructor( argument_types, TMP) )
    {
        type->write_explicit_constructor(argument_types, var_C_exp,
                        argument_Cexpressions, output);

    }
    else  // assume only one unnamed RHS var
    {
        auto RHS_type = argument_types->unnamed_argument_types[0];
        auto RHS_exp = argument_Cexpressions[0];

        RHS_type->write_explicit_castTo(var_C_exp, RHS_exp, output, call_virtual);
    }

    return var_C_exp;
}



////// symbol tables //////

// base symbol table
sym_table_base::sym_table_base()
{
    //next_variable_ID = 0;
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
    variable->C_name += get_unique_string();
    variable->loc = _loc;
    variable->is_ordered = is_ordered;
    variable->var_type = var_type;

    variable_table[definition_name] = variable;
    return variable;
}

void sym_table_base::add_variable(varName_ptr new_var, bool& is_exclusive)
{
    if( is_exclusive )
    {
        bool check_order = false;
        varName_ptr otherV = get_variable_local( new_var->definition_name, new_var->loc, check_order);
        check_order = false;
        varType_ptr otherT = get_type_local( new_var->definition_name, new_var->loc, check_order);

        if( otherV )
        {
            //throw MultipleDefinition_exc(definition_name, _loc, otherV->loc);
            cout << "name '" << new_var->definition_name << "' has multiple definitions. Defined at: " << new_var->loc << " and " << otherV->loc << endl;
            is_exclusive = false;
            return;
        }
        else if( otherT )
        {
            //throw MultipleDefinition_exc(definition_name, new_type->loc, otherT->loc);
            cout << "name '" << new_var->definition_name << "' has multiple definitions. Defined at: " << new_var->loc << " and " << otherT->loc << endl;
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


    variable_table[new_var->definition_name] = new_var;
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

void sym_table_base::add_variable(varName_ptr new_var, bool& is_exclusive, utf8_string& alias)
{
    if( is_exclusive )
    {
        bool check_order = false;
        varName_ptr otherV = get_variable_local( alias, new_var->loc, check_order);
        check_order = false;
        varType_ptr otherT = get_type_local( alias, new_var->loc, check_order);

        if( otherV )
        {
            //throw MultipleDefinition_exc(definition_name, _loc, otherV->loc);
            cout << "name '" << alias << "' has multiple definitions. Defined at: " << new_var->loc << " and " << otherV->loc << endl;
            is_exclusive = false;
            return;
        }
        else if( otherT )
        {
            //throw MultipleDefinition_exc(definition_name, new_type->loc, otherT->loc);
            cout << "name '" << alias << "' has multiple definitions. Defined at: " << new_var->loc << " and " << otherT->loc << endl;
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


    variable_table[alias] = new_var;
}

void sym_table_base::add_type(varType_ptr new_type, bool& is_exclusive, utf8_string& alias)
{
    if( is_exclusive )
    {
        bool check_order = false;
        varName_ptr otherV = get_variable_local( alias, new_type->loc, check_order);
        check_order = false;
        varType_ptr otherT = get_type_local( alias, new_type->loc, check_order);

        if( otherV )
        {
            //throw MultipleDefinition_exc(definition_name, _loc, otherV->loc);
            cout << "name '" << alias << "' has multiple definitions. Defined at: " << new_type->loc << " and " << otherV->loc << endl;
            is_exclusive = false;
            return;
        }
        else if( otherT )
        {
            //throw MultipleDefinition_exc(definition_name, new_type->loc, otherT->loc);
            cout << "name '" << alias << "' has multiple definitions. Defined at: " << new_type->loc << " and " << otherT->loc << endl;
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

    type_table[alias] = new_type;
}


varName_ptr sym_table_base::get_variable_local(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)
{
    auto table_itterator = variable_table.find( var_name );
    if(table_itterator != variable_table.end())
    {
        // var found
        varName_ptr variable = table_itterator->second;
        if( variable->is_ordered and ref_loc.is_comparible( variable->loc ) and not ref_loc.strictly_GT( variable->loc )  )
        {
            // out of order!
            if( check_order ) // we care
            {
                cout << "name '" << var_name << "' invoked before definition. Defined at: " << variable->loc << ". Referenced at: " << ref_loc << endl;
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

varType_ptr sym_table_base::get_type_local(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)
{
    auto table_itterator = type_table.find( var_name );
    if(table_itterator != type_table.end())
    {
        // var found
        varType_ptr type = table_itterator->second;
        if( type->is_ordered and ref_loc.is_comparible( type->loc ) and not ref_loc.strictly_GT( type->loc )  )
        {
            // out of order!
            if( check_order ) // we care
            {
                cout << "name '" << var_name << "' invoked before definition. Defined at: " << type->loc << ". Referenced at: " << ref_loc << endl;
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

utf8_string local_sym_table::get_unique_string()
{
    return parent_table->get_unique_string();
}
varName_ptr local_sym_table::get_variable_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)
{
    varName_ptr ret = get_variable_local( var_name, ref_loc, check_order );

    if(not ret)
        ret = parent_table->get_variable_global(var_name, ref_loc, check_order);

    return ret;
}

varType_ptr local_sym_table::get_type_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)
{
    varType_ptr ret = get_type_local( var_name, ref_loc, check_order );

    if(not ret)
        ret = parent_table->get_type_global(var_name, ref_loc, check_order);

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
    next_variable_ID = 0;

    // defined un-named C type
    auto variable_type = make_shared<varType_fromC>();
    variable_type->set_pointers(variable_type);
    variable_type->definition_name = "UNNAMED_C_TYPE";
    variable_type->C_name = "";
    variable_type->is_ordered = false;
    variable_type->unnamed_C_type = variable_type;

    bool is_exclusive = true;
    add_type(variable_type, is_exclusive);


    is_exclusive = true;
    auto void_T = make_shared<void_type>();
    add_type(void_T, is_exclusive);
}

void module_sym_table::set_name(csu::utf8_string& _name)
{
    name = _name;
    namespace_unique_name = "__cy__" + name;
}

varName_ptr module_sym_table::get_variable_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)
{
    varName_ptr ret = get_variable_local( var_name, ref_loc, check_order );

    // check imports here?

    return ret;
}

varType_ptr module_sym_table::get_type_global(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)
{
    varType_ptr ret = get_type_local( var_name, ref_loc, check_order );

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

utf8_string module_sym_table::get_unique_string()
{
    utf8_string out_string = to_string(next_variable_ID);
    next_variable_ID += 1;
    return out_string;
}

