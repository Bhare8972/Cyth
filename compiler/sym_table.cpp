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
#include "useful_visitors.hpp"

using namespace csu;
using namespace std;


/////// some general expression writers ///////

class basic_function_call_writer : public expression_call_writer
{
public:

    parameter_to_arguments_mapper mapper;
    ostream& output;
    utf8_string exp;

    basic_function_call_writer(ostream& _output, func_param_ptr _parameters,
                               call_argument_list* _argument_node_ptr) :
       mapper(_parameters, _argument_node_ptr),
       output(_output)
    {}

    basic_function_call_writer(ostream& _output, parameter_to_arguments_mapper& _mapper) :
       mapper(_mapper),
       output(_output)
    {}

    basic_function_call_writer(ostream& _output, func_param_ptr _parameters,
                               vector<expression_AST_node*> &expressions, sym_table_base* _symbol_table) :
       mapper(_parameters, expressions, _symbol_table),
       output(_output)
    {}

    utf8_string get_C_expression() {  return exp; }
    void write_cleanup(){ mapper.write_cleanup(output); }
};


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

/// DEFINITION ///
bool varType::can_be_defined() // Returns false if Ctype is unknown
{
    return not (C_name == "");
}

// define new variable of this type.
void varType::C_definition_name(utf8_string& var_name, ostream& output)
{
    if( can_be_defined() )
    {
        output << C_name << " " << var_name ;
    }
}

/// CALLING ///
varType_ptr varType::is_callable(call_argument_list* argument_node_ptr, utf8_string& error_msg)
{
    error_msg = "type " + definition_name + " is not callable";
    return nullptr;
}

exp_writer_ptr varType::write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
    std::vector<csu::utf8_string>& argument_Cexpressions, std::ostream& output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE IS NOT CALLABLE THIS SHOULD NOT BE REACHED. varType::write_call");
    return nullptr;
}

/// ADDITION ///
bool varType::get_has_LHSaddition(varType* RHS_type)
{
    return false;
}

exp_writer_ptr varType::write_LHSaddition(expression_AST_ptr LHS_ast, utf8_string& LHS_exp, expression_AST_ptr RHS_ast,
     utf8_string& RHS_exp,  ostream& output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have addition ");
}

varType_ptr varType::get_LHSaddition_type(varType* RHS_type)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have addition ");
    return nullptr;
}

/// ASSIGNMENT ///
bool varType::get_has_assignment(varType* RHS_type)
{
    return false;
}

void varType::write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have assignment ");
}

varType_ptr varType::get_auto_type()
{
    return nullptr;
}

bool varType::get_has_assignTo(varType* LHS_type)
{
    return false;
}

void varType::write_assignTo(varType* LHS_type, expression_AST_node* RHS_AST_node,
                                  utf8_string& LHS, utf8_string& RHS_exp, ostream& output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have assign-to ");
}



/// REFERENCE ///
varType_ptr varType::is_reference_type(bool &is_ref)
{
    is_ref = false;
    return nullptr;
}

bool varType::can_take_reference(varType* RHS_type)
{  return false;  }

void varType::take_reference(varType* RHS_type, csu::utf8_string& LHS, csu::utf8_string& referencable_RHS_exp, std::ostream& output)
{ throw gen_exception("ERROR: TYPE ", definition_name, " is not a reference type. In varType::take_reference, should not be reached. "); }

bool varType::can_get_pointer(varType* output_type)
{
    return is_equivalent( output_type );
}

utf8_string varType::get_pointer(varType* output_type, utf8_string& exp,ostream& output)
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
varType_ptr varType::member_has_getter(utf8_string& member_name)
{
    return nullptr;
}

exp_writer_ptr varType::write_member_getter(utf8_string& expression, utf8_string& member_name,
                ostream& output, bool call_virtual)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have member access ");
    return nullptr;
}

varType_ptr varType::member_has_getref(utf8_string& member_name)
{
    return nullptr;
}

exp_writer_ptr varType::write_member_getref(utf8_string& expression, utf8_string& member_name,
                ostream& output, bool call_virtual )
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have member access ");
    return nullptr;
}

varType_ptr varType::set_member(utf8_string& member_name, varType* RHS_type)
{
    return nullptr;
}

/// CONSTRUCTORS ///

bool varType::has_explicit_constructor(call_argument_list* argument_node_ptr, utf8_string& error_msg)
{
    error_msg = "type "+definition_name+" cannot be explicitly constructed";
    return false;
}

void varType::write_explicit_constructor(call_argument_list* argument_node,
               utf8_string& LHS_Cexp, vector<utf8_string>& argument_Cexpressions, ostream& output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " cannot be explicitly constructed");
}

void varType::write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node, utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " is not copyable");
}



// C import

varType_fromC::varType_fromC()
{
    type_of_type = varType::c_import_t;
}
void varType_fromC::set_pointers(std::weak_ptr<varType> _self, std::weak_ptr<varType> _unnamed_C_type)
{
    unnamed_C_type = _unnamed_C_type;
    self = _self;
}

bool varType_fromC::is_equivalent(varType* RHS)
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

varType_ptr varType_fromC::copy(utf8_string _definition_name )
{
    if( _definition_name=="" )
    {
        _definition_name = definition_name;
    }

    auto new_var = make_shared<varType_fromC>();
    new_var->set_pointers( new_var, unnamed_C_type );
    new_var->definition_name = _definition_name;
    new_var->C_name = C_name;
    new_var->loc = loc;
    new_var->is_ordered = is_ordered;

    return new_var;
}

varType_ptr varType_fromC::is_callable(call_argument_list* argument_node_ptr, utf8_string& error_msg)
{
    if( argument_node_ptr->num_named() != 0 )
    {
        stringstream TMP;
        TMP << "C-function cannot take named arguments. Called at "<< argument_node_ptr->loc <<endl;
        error_msg = TMP.str();
        return nullptr;
    }

    return unnamed_C_type.lock();
}

exp_writer_ptr varType_fromC::write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
  vector<utf8_string>& argument_Cexpressions, ostream& output, bool call_virtual)
{

    stringstream out;
    out <<  '(' << ( LHS_Cexp->get_C_expression() ) << '(';

    bool do_comma = false;
    for(auto str_write : argument_Cexpressions)
    {
        if( do_comma )
        {
            out << ',';
        }
        else
        {
            do_comma = true;
        }

        out << str_write;
    }
    out << "))";

    auto ret = make_shared<simple_expression_writer>();
    ret->exp = out.str();


    return ret;
}


/// ADDITION ///
bool varType_fromC::get_has_LHSaddition(varType* RHS_type)
{
    return RHS_type->type_of_type == varType::c_import_t;
}

varType_ptr varType_fromC::get_LHSaddition_type(varType* RHS_type)
{
    if( RHS_type->type_of_type == varType::c_import_t )
    {
        return unnamed_C_type.lock();
    }
    else
    {
        return nullptr;
    }
}

exp_writer_ptr varType_fromC::write_LHSaddition(expression_AST_ptr LHS_ast, utf8_string& LHS_exp, expression_AST_ptr RHS_ast,
         utf8_string& RHS_exp, ostream& output, bool call_virtual)
{
    stringstream ret;
    ret << '(' << LHS_exp << '+' << RHS_exp << ')';
    return make_shared<simple_expression_writer>( ret.str() );
    //return ret.str();
}

/// ASSIGNMENT ///
bool varType_fromC::get_has_assignment(varType* RHS_type)
{
    return RHS_type->type_of_type == varType::c_import_t;
}

void varType_fromC::write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    output << LHS << '=' << RHS_exp <<';';
}

shared_ptr<varType> varType_fromC::get_auto_type()
{
    return self.lock();
}



/// CONSTRUCTION ///
bool varType_fromC::has_explicit_constructor(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg)
{
    if( argument_node_ptr->num_unnamed()!=1  or  argument_node_ptr->num_named() != 0 )
    {
        stringstream TMP;
        TMP << "C-type ( " <<  definition_name << ":" << C_name << " ) can only be constructed with one un-named argument at " << argument_node_ptr->loc;
        error_msg = TMP.str();
        return false;
    }

    auto type = argument_node_ptr->unNamedExp_by_index( 0 )->expression_return_type;
    if( type->type_of_type != varType::c_import_t )
    {
        stringstream TMP;
        TMP << "can only assign to a C-type from another C-type at " << argument_node_ptr->loc << endl;
        error_msg = TMP.str();
        return false;
    }
    return true;
}


void varType_fromC::write_explicit_constructor(call_argument_list* argument_node,
               utf8_string& LHS_Cexp, vector<utf8_string>& argument_Cexpressions, ostream& output)
{
    output << LHS_Cexp << '=' << argument_Cexpressions[0] << ';' << endl;
}

bool varType_fromC::has_implicit_copy_constructor(varType* RHS_type)
{
    return (RHS_type->type_of_type==varType::c_import_t) and (C_name==RHS_type->C_name) ;
}

void varType_fromC::write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node, utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    output << LHS << '=' << RHS_exp <<';';
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


varType_ptr raw_C_pointer_reference::copy(csu::utf8_string _definition_name )
{
    if( _definition_name == "" )
    {
        _definition_name = definition_name;
    }

    auto new_var = make_shared<raw_C_pointer_reference>( referenced_type, assignment_mode );
    new_var->definition_name = _definition_name;

    return new_var;
}

bool raw_C_pointer_reference::is_equivalent(varType* RHS)
{
    if( RHS->type_of_type == varType::c_pointer_reference )
    {
        raw_C_pointer_reference* RHS_ptr = dynamic_cast<raw_C_pointer_reference*>( RHS );
        return referenced_type->is_equivalent( RHS_ptr->referenced_type.get() );
    }
    else
    {
        return false;
    }
}

/// DEFINITION
bool raw_C_pointer_reference::can_be_defined()
{
    return true;
}

void raw_C_pointer_reference::C_definition_name(utf8_string& var_name, ostream& output)
{
    utf8_string new_var_name = "*" + var_name;
    referenced_type->C_definition_name(new_var_name, output);
}

/// CALLING
varType_ptr raw_C_pointer_reference::is_callable(call_argument_list* argument_node_ptr, utf8_string& error_msg)
{
    return referenced_type->is_callable( argument_node_ptr,  error_msg );
}

exp_writer_ptr raw_C_pointer_reference::write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
    vector<csu::utf8_string>& argument_Cexpressions, ostream& output, bool call_virtual)
{
    utf8_string _true_LHS_ = "(*" + LHS_Cexp->get_C_expression() + ")";
    auto true_LHS = make_shared<simple_expression_writer>( _true_LHS_ );
    return referenced_type->write_call( argument_node, true_LHS, argument_Cexpressions, output, call_virtual=true );
}



/// ADDITION
bool raw_C_pointer_reference::get_has_LHSaddition(varType* RHS_type)
{
    return referenced_type->get_has_LHSaddition( RHS_type );
}

varType_ptr raw_C_pointer_reference::get_LHSaddition_type(varType* RHS_type)
{
    return referenced_type->get_LHSaddition_type( RHS_type );
}

exp_writer_ptr raw_C_pointer_reference::write_LHSaddition(expression_AST_ptr LHS_ast, utf8_string& LHS_exp, expression_AST_ptr RHS_ast,
        utf8_string& RHS_exp, ostream& output, bool call_virtual)
{
    utf8_string newLHS = "(*" + LHS_exp;
    newLHS += ")";
    return referenced_type->write_LHSaddition(LHS_ast, newLHS, RHS_ast, RHS_exp, output, true);
}

/// ASSIGNMENT
bool raw_C_pointer_reference::get_has_assignment(varType* RHS_type)
{
    if( assignment_mode == raw_C_pointer_reference::no_assignment )
    {
        return false;
    }
    else if( assignment_mode == raw_C_pointer_reference::only_reference_assignment )
    {
        return is_equivalent( RHS_type );
    }
    else if(  assignment_mode == raw_C_pointer_reference::take_reference_assignment )
    {
        return is_equivalent( RHS_type ) or referenced_type->is_equivalent( RHS_type );
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

void raw_C_pointer_reference::write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    if( assignment_mode == raw_C_pointer_reference::no_assignment )
    {
        throw gen_exception("Error 1 in raw_C_pointer_reference::write_assignment. Should not be reached." );
    }
    else if( assignment_mode == raw_C_pointer_reference::only_reference_assignment )
    {
        output << LHS << '=' << RHS_exp <<';';
    }
    else if(  assignment_mode == raw_C_pointer_reference::take_reference_assignment )
    {
        if( is_equivalent( RHS_type ) )
        {
            output << LHS << '=' << RHS_exp <<';';
        }
        else if( referenced_type->is_equivalent( RHS_type ) )
        {
//
            if( not RHS_AST_node->c_exp_can_be_referenced )
            {
                throw gen_exception("Expression cannot be referenced at ", RHS_AST_node->loc );
            }

            output << LHS << '=' << "&(" << RHS_exp << ");";
        }
        else
        {
            throw gen_exception("Error 2 in raw_C_pointer_reference::write_assignment. Should not be reached." );
        }
    }
    else if(  assignment_mode == raw_C_pointer_reference::full_assign_assignment )
    {
        utf8_string new_LHS = "(*" + LHS;
        new_LHS += ')';

        //if( referenced_type->get_has_assignment(RHS_type) )
        //{
            referenced_type->write_assignment(RHS_type, RHS_AST_node, new_LHS,  RHS_exp, output);
        //}
//        else if( RHS_type->can_implicit_castTo( referenced_type.get() ) )
//        {
//            RHS_type->write_implicit_castTo(referenced_type.get(),  RHS_AST_node, new_LHS,  RHS_exp, output );
//        }
    }
}

bool raw_C_pointer_reference::get_has_assignTo(varType* LHS_type)
{
    return LHS_type->get_has_assignment( referenced_type.get() ) or referenced_type->get_has_assignTo( LHS_type );
}


void raw_C_pointer_reference::write_assignTo(varType* LHS_type, expression_AST_node* RHS_AST_node,
                    utf8_string& LHS, utf8_string& RHS_exp, ostream& output, bool call_virtual)
{
    utf8_string newRHS = "(*" + RHS_exp;
    newRHS += ")";
    if( LHS_type->get_has_assignment( referenced_type.get() ) )
    {
        LHS_type->write_assignment(referenced_type.get(), RHS_AST_node, LHS, newRHS, output);
    }
    else
    {
        referenced_type->write_assignTo(LHS_type, RHS_AST_node, LHS, newRHS, output, true);
    }
}


/// CLASS members

varType_ptr raw_C_pointer_reference::member_has_getter(utf8_string& member_name)
{
    return referenced_type->member_has_getter( member_name );
}

exp_writer_ptr raw_C_pointer_reference::write_member_getter(csu::utf8_string& expression, utf8_string& member_name,
        ostream& output, bool call_virtual)
{
    utf8_string newLHS = "(*" + expression;
    newLHS += ")";
    return referenced_type->write_member_getter(newLHS, member_name,  output, true);
}

varType_ptr raw_C_pointer_reference::member_has_getref(utf8_string& member_name)
{
    return referenced_type->member_has_getref( member_name );
}

exp_writer_ptr raw_C_pointer_reference::write_member_getref( utf8_string& expression,
            utf8_string& member_name, ostream& output, bool call_virtual)
{
    utf8_string newLHS = "(*" + expression;
    newLHS += ")";
    return referenced_type->write_member_getref(newLHS, member_name,  output, true);
}

varType_ptr raw_C_pointer_reference::set_member(utf8_string& member_name, varType* RHS_type)
{
    return referenced_type->set_member(member_name, RHS_type);
}


void raw_C_pointer_reference::write_member_setter(expression_AST_node* exp_AST_node, utf8_string& LHS_expression, utf8_string& member_name,
             varType* RHS_type, utf8_string& RHS_expression, std::ostream& output, bool call_virtual)
{
    utf8_string newLHS = "(*" + LHS_expression;
    newLHS += ")";
    referenced_type->write_member_setter(exp_AST_node, newLHS, member_name, RHS_type, RHS_expression, output, true);
}


/// CASTING ///

bool raw_C_pointer_reference::can_implicit_castTo(varType* LHS_type)
{
    // this duplicity is because construction won't work as it should for references, thus will default here
    // so we want to try copc_construction first
    return LHS_type->has_implicit_copy_constructor( referenced_type.get() ) or referenced_type->can_implicit_castTo( LHS_type );
}

void raw_C_pointer_reference::write_implicit_castTo(varType* LHS_type, expression_AST_node* RHS_AST_node, utf8_string& LHS,
    utf8_string& RHS_exp, ostream& output, bool call_virtual)
{
    utf8_string newRHS = "(*" + RHS_exp;
    newRHS += ")";
    if( LHS_type->has_implicit_copy_constructor( referenced_type.get() ) )
    {
        LHS_type->write_implicit_copy_constructor(referenced_type.get(), RHS_AST_node, LHS, newRHS, output);
    }
    else
    {
        referenced_type->write_implicit_castTo(LHS_type, RHS_AST_node, LHS, newRHS, output, true);
    }
}

 /// Reference stuff ///
varType_ptr raw_C_pointer_reference::is_reference_type(bool &is_ref)
{
    is_ref = true;
    return referenced_type;
}

bool raw_C_pointer_reference::can_take_reference(varType* RHS_type)
{
    //return RHS_type->can_get_pointer( referenced_type.get() );
    return referenced_type->is_equivalent( RHS_type );
}

void raw_C_pointer_reference::take_reference(varType* RHS_type, utf8_string& LHS, utf8_string& referencable_RHS_exp, ostream& output)
{
    auto C_ptr_str = RHS_type->get_pointer(referenced_type.get(), referencable_RHS_exp, output);
    output << LHS << "=" << C_ptr_str << ";" << endl;

//    if( referenced_type->is_equivalent( RHS_type ) )
//    {
//        output << LHS << "= &(" << referencable_RHS_exp << ");" << endl;
//    }
//    else
//    {
//        bool is_ref = false;
//        auto RHS_ref_type = RHS_type->is_reference_type( is_ref );
//        if( is_ref )
//        {
//            auto reference_string = RHS_type->get_reference( referenced_type.get(), referencable_RHS_exp, output );
//            output << LHS << "= &(" << reference_string << ");" << endl;
//        }
//        else
//        {
//            throw gen_exception("cannot take reference of type ", RHS_type->definition_name, " to type ", definition_name );
//        }
//    }
}

bool raw_C_pointer_reference::can_get_pointer(varType* output_type)
{
    if( is_equivalent( output_type ) )
    {
        return true;
    }
    else if( referenced_type->is_equivalent( output_type ) )
    {
        return true;
    }
    else if( referenced_type->can_get_pointer(output_type) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

utf8_string raw_C_pointer_reference::get_pointer(varType* output_type, utf8_string& exp, ostream& output)
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
        throw gen_exception("cannot take reference of type ", output_type, " to type ", definition_name,
            ". This should not be reached in raw_C_pointer_reference::get_pointer" );
    }



//    if( referenced_type->is_equivalent( output_type ) )
//    {
//        return "*(" + exp +")";
//    }
//    else
//    {
//        throw gen_exception("cannot take reference of type ", output_type, " to type ", definition_name );
//
//    }
}

/// CONSTRUCTORS ///

bool raw_C_pointer_reference::has_implicit_copy_constructor(varType* RHS_type)
{
    return is_equivalent( RHS_type ) or referenced_type->is_equivalent( RHS_type );
}

void raw_C_pointer_reference::write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    if( is_equivalent( RHS_type ) )
    {
        output << LHS << '=' << RHS_exp <<';';
    }
    else if( referenced_type->is_equivalent( RHS_type ) )
    {
        if( not RHS_AST_node->c_exp_can_be_referenced )
        {
            throw gen_exception("Expression cannot be referenced at ", RHS_AST_node->loc );
        }

        output << LHS << '=' << "&(" << RHS_exp << ");";
    }
    else
    {
        throw gen_exception("Error 2 in raw_C_pointer_reference::write_assignment. Should not be reached." );
    }
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

bool func_parameter_info::is_equivalent(func_parameter_info* RHS)
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
            if( not thisT->is_equivalent( otherT.get() ) )
            {
                return false;
            }
        }
        return true;
    }
}

bool func_parameter_info::is_distinguishable(func_param_ptr other_parameters)
{
    return not is_equivalent( other_parameters.get() );

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

void func_parameter_info::write_to_C(std::ostream& output, bool write_void_for_zero_params)
{
   // output << '(';



    if( total_size()==0  and write_void_for_zero_params)
    {
        output<<"void";
    }
    else
    {
        // do required parameters
        int req_param_i=0;
        for(; req_param_i<required_parameters.size(); req_param_i++ )
        {
            if( req_param_i !=0 )
            {
                output<<',';
            }

            auto paramname = required_parameters[ req_param_i ];
            paramname->var_type->C_definition_name( paramname->C_name, output );
        }

        for(int opt_param_i=0; opt_param_i<optional_parameters.size(); opt_param_i++ )
        {
            if( (opt_param_i !=0) or (req_param_i!=0) )
            {
                output<<',';
            }

            auto paramname = optional_parameters[ opt_param_i ];

            // put default bool bit
            output << "int "; // bools in c are just int
            output << paramname->definition_name ;
            output << "__use_default__,";

            paramname->var_type->C_definition_name( paramname->C_name, output );
        }

    }
    //output<<")";
}

void func_parameter_info::print(std::ostream& output)
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
parameter_to_arguments_mapper::argument_wrapper::argument_wrapper(call_argument_list* argument_node_ptr)
{
    int N = argument_node_ptr->num_unnamed();
    unnamed_arguments.reserve(N);
    for( int i=0; i<N; i++ )
    {
        unnamed_arguments.push_back( argument_node_ptr->unNamedExp_by_index(i).get() );
    }

    N = argument_node_ptr->num_named();
    named_argument_names.reserve(N);
    named_arguments.reserve(N);
    for( int i=0; i<N; i++ )
    {
        named_arguments.push_back( argument_node_ptr->namedExp_by_index(i).get() );
        named_argument_names.push_back( argument_node_ptr->namedExpName_by_index(i) );
    }
}

parameter_to_arguments_mapper::argument_wrapper::argument_wrapper( vector<expression_AST_node*> &expressions)
{
    int N = expressions.size();
    unnamed_arguments.reserve(N);
    for( int i=0; i<N; i++ )
    {
        unnamed_arguments.push_back( expressions[i] );
    }
}

int parameter_to_arguments_mapper::argument_wrapper::total_size()
{
    return unnamed_arguments.size() + named_argument_names.size();
}

void parameter_to_arguments_mapper::argument_wrapper::print(std::ostream& output)
{
    bool do_comma = false;

    output <<"( ";

    for( auto exp : unnamed_arguments )
    {
        if(do_comma)
        {
            output << ", " ;
        }
        else
        {
            do_comma = true;
        }

        output << exp->expression_return_type->definition_name ;
    }

    int num = named_arguments.size();
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

        auto exp = named_arguments[i];
        auto name = named_argument_names[i];

        output << name << '=' << exp->expression_return_type->definition_name;
    }

    output <<")";
}

expression_AST_node* parameter_to_arguments_mapper::argument_wrapper::expression_from_index(int i)
{
    if( i < unnamed_arguments.size() )
    {
        return unnamed_arguments[i];
    }
    else if( i < total_size() )
    {
        return named_arguments[i - unnamed_arguments.size()];
    }
    else
    {
        throw gen_exception("i is too large in call_argument_list::expression_from_index");
    }
}



parameter_to_arguments_mapper::parameter_to_arguments_mapper(func_param_ptr _parameters, call_argument_list* _argument_node_ptr) :
    arguments( _argument_node_ptr )
{
    parameters = _parameters;
    symbol_table = _argument_node_ptr->symbol_table;

    initialize();
}

parameter_to_arguments_mapper::parameter_to_arguments_mapper(func_param_ptr _parameters, std::vector<expression_AST_node*> &expressions, sym_table_base* _symbol_table) :
    arguments( expressions )
{
    parameters = _parameters;
    symbol_table = _symbol_table;

    initialize();
}

void parameter_to_arguments_mapper::initialize()
{
    is_good = true; // change if not true

    // increment when needed
    num_defaults_used = 0;
    num_casts = 0;
    num_pointer_casts = 0;

    int num_arguments = arguments.total_size();
    int num_unnamed_args =  arguments.unnamed_arguments.size() ;
    int num_named_args = arguments.named_arguments.size() ;

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
        auto param_name = arguments.named_argument_names[named_arg_i];
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
            auto arg_type = arguments.expression_from_index(arg_index)->expression_return_type;
            bool param_is_reference = false;
            auto param_ref_type = param_type->is_reference_type( param_is_reference );

            if( param_is_reference )
            {
                if( not param_type->can_take_reference( arg_type.get() ) )
                {
                    if( arg_type->can_get_pointer( param_type.get() ) )
                    {
                        num_pointer_casts += 1;
                    }
                    else if( param_ref_type->has_implicit_copy_constructor(arg_type.get()) or arg_type->can_implicit_castTo(param_ref_type.get()) )
                    {
                        //we're good
                        num_casts += 1;
                    }
                    else
                    {
                        // cannot cast
                        error = "type \"";
                        error += param_type->definition_name;
                        error += utf8_string( "\" cannot reference type \"");
                        error += arg_type->definition_name;

                        is_good = false;
                    }
                } // else we are good

            }
            else if( not arg_type->is_equivalent(param_type.get()) ) // types not exactly the same. need to cast!
            {
                if( param_type->has_implicit_copy_constructor(arg_type.get()) or arg_type->can_implicit_castTo(param_type.get()) )
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
            else if( not arg_type->has_implicit_copy_constructor(param_type.get()) ) // we still have to call the copy constructor!
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
            // check we have a default
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

int parameter_to_arguments_mapper::comparison(shared_ptr<parameter_to_arguments_mapper> RHS)
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

void parameter_to_arguments_mapper::write_arguments(vector<utf8_string>& argument_Cexpressions, vector<utf8_string> &out, ostream& output)
{
    /// useful variables
    int total_C_arguments = parameters->required_parameters.size() + 2*parameters->optional_parameters.size();
    int total_num_parameters = parameters->total_size();


    /// first we write the argument expressions
    out.reserve( total_C_arguments );
    for(int param_i = 0; param_i < total_num_parameters; param_i++ )
    {
        auto param_name = parameters->parameter_from_index( param_i );
        int argument_index = param_to_arg_map[param_i];// amazing bit of naming here... solid job.

        bool param_has_default  =  param_i>=(parameters->required_parameters.size());
        utf8_string use_default_vname;


        if( param_has_default )
        {
            use_default_vname = "__cy__need_default_";
            use_default_vname += symbol_table->get_unique_string();
            out.push_back( use_default_vname );
            output << "int " << use_default_vname << "=0;" << endl;
        }

        utf8_string argument_vname = "__cy__arg_";

        argument_vname += symbol_table->get_unique_string();
        out.push_back( argument_vname );

        param_name->var_type->C_definition_name(argument_vname, output); // note this is destructed inside of, at the end of , the function.
        output << ';'<<endl;
        param_name->var_type->initialize(argument_vname, output);

        if( argument_index == -1 )
        {
            // make a default value.
            output << use_default_vname << "=1;" << endl;
        }
        else
        {
            auto expr = arguments.expression_from_index( argument_index );
            auto expr_type = expr->expression_return_type;
            auto argument_C_exp = argument_Cexpressions[argument_index];

            bool param_is_reference = false;
            auto param_ref_type = param_name->var_type->is_reference_type( param_is_reference );

            if( param_is_reference )
            {
                if( param_name->var_type->can_take_reference( expr_type.get() )  or  expr_type->can_get_pointer( param_ref_type.get() ) )
                {
                    auto C_arg_exp_to_reference = argument_C_exp;

                    if( not expr->c_exp_can_be_referenced )
                    { // need to copy the argument
                        utf8_string memTmp_vname = "__cy__arg_MemTmp_";
                        memTmp_vname +=  symbol_table->get_unique_string();

                        expr_type->C_definition_name(memTmp_vname, output);
                        output << ';'<<endl;
                        expr_type->initialize(memTmp_vname, output);

                        names_to_cleanup.push_back( memTmp_vname );
                        types_to_cleanup.push_back( expr_type.get() );

                        expr->has_output_ownership = false;
                        output << memTmp_vname << "=" << (argument_C_exp) << ";" << endl;
// TODO: memory moved here.
                        C_arg_exp_to_reference = memTmp_vname;
                    }

                    // this can do a pointer cast if necessary.
                    param_name->var_type->take_reference( expr_type.get(), argument_vname, C_arg_exp_to_reference, output );
                }
                else
                {
                    utf8_string memTmp_vname = "__cy__arg_MemTmp_";
                    memTmp_vname +=  symbol_table->get_unique_string();

                    param_ref_type->C_definition_name(memTmp_vname, output);
                    output << ';'<<endl;
                    param_ref_type->initialize(memTmp_vname, output);

                    names_to_cleanup.push_back( memTmp_vname );
                    types_to_cleanup.push_back( param_ref_type.get() );

                    if( param_ref_type->has_implicit_copy_constructor(  expr->expression_return_type.get() )  )
                    {
                        param_ref_type->write_implicit_copy_constructor(expr->expression_return_type.get(), expr, memTmp_vname, argument_C_exp, output);
                    }
                    else if( expr->expression_return_type->can_implicit_castTo( param_name->var_type.get() ) )
                    {
                        expr->expression_return_type->write_implicit_castTo(param_ref_type.get(), expr, memTmp_vname, argument_C_exp, output);
                    }
                    else
                    {
                        throw gen_exception("cannot convert ", expr->expression_return_type->definition_name, " to ",  param_ref_type->definition_name,
                                            " in parameter_to_arguments_mapper::write_arguments. This should not be reached" );
                    }

                    param_name->var_type->take_reference(param_ref_type.get(), argument_vname, memTmp_vname, output);
                }

//
//                if( param_name->var_type->is_equivalent( expr_type.get() )  )
//                {
//                    // just take it!
//
//                }
//                else if( not param_ref_type->is_equivalent( expr->expression_return_type.get()  ) )
//                {
//                    utf8_string memTmp_vname = "__cy__arg_MemTmp_";
//                    memTmp_vname +=  symbol_table->get_unique_string();
//
//                    param_ref_type->C_definition_name(memTmp_vname, output);
//                    output << ';'<<endl;
//
//                    names_to_cleanup.push_back( memTmp_vname );
//                    types_to_cleanup.push_back( param_ref_type.get() );
//
//                    if( param_ref_type->has_implicit_copy_constructor(  expr->expression_return_type.get() )  )
//                    {
//                        param_ref_type->write_implicit_copy_constructor(expr->expression_return_type.get(), expr, memTmp_vname, argument_Cexpressions[argument_index], output);
//                    }
//                    else if( expr->expression_return_type->can_implicit_castTo( param_name->var_type.get() ) )
//                    {
//                        expr->expression_return_type->write_implicit_castTo(param_ref_type.get(), expr, memTmp_vname, argument_Cexpressions[argument_index], output);
//                    }
//                    else
//                    {
//                        throw gen_exception("cannot convert ", expr->expression_return_type->definition_name, " to ",  param_ref_type->definition_name,
//                                            " in parameter_to_arguments_mapper::write_arguments. This should not be reached" );
//                    }
//
//                    param_name->var_type->take_reference(param_ref_type.get(), argument_vname, memTmp_vname, output);
//
//                }
//                else if( not expr->c_exp_can_be_referenced )
//                {
//                    utf8_string memTmp_vname = "__cy__arg_MemTmp_";
//                    memTmp_vname +=  symbol_table->get_unique_string();
//
//                    param_ref_type->C_definition_name(memTmp_vname, output);
//                    output << ';'<<endl;
//
//                    expr->has_output_ownership = false;
//                    output << memTmp_vname << "=" << (expr->writer->get_C_expression()) << ";" << endl;
//                    param_name->var_type->take_reference(param_ref_type.get(), argument_vname, memTmp_vname, output);
//                }

            }
            else if( param_name->var_type->has_implicit_copy_constructor(  expr->expression_return_type.get() )  )
            {
                param_name->var_type->write_implicit_copy_constructor(expr->expression_return_type.get(), expr, argument_vname, argument_Cexpressions[argument_index], output);
            }
            else if( expr->expression_return_type->can_implicit_castTo( param_name->var_type.get() ) )
            {
                expr->expression_return_type->write_implicit_castTo(param_name->var_type.get(), expr, argument_vname, argument_Cexpressions[argument_index], output);
            }
            output<<endl;
        }
    }
}

void parameter_to_arguments_mapper::write_cleanup( ostream& output )
{
    for( int i=0; i<names_to_cleanup.size(); i++)
    {
        auto name = names_to_cleanup[i];
        auto type = types_to_cleanup[i];

        type->write_destructor( name, output );
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

bool staticFuncPntr::is_equivalent(varType* RHS)
{
    if( RHS->type_of_type ==  varType::function_pntr_t )
    {
        staticFuncPntr* RHS_funcP = dynamic_cast<staticFuncPntr*>( RHS );

        bool param_equiv =  parameters->is_equivalent( RHS_funcP->parameters.get() );
        bool return_equiv = return_type->is_equivalent( RHS_funcP->return_type.get() );

        return param_equiv and return_equiv;
    }
    else
    {
        return false;
    }
}


varType_ptr staticFuncPntr::copy(utf8_string _definition_name )
{
    if( _definition_name == "" )
    {
        _definition_name = definition_name;
    }

    auto new_var = make_shared<staticFuncPntr>( );
    new_var->definition_name = _definition_name;

    new_var->self = new_var;
    new_var->loc = loc;
    new_var->is_ordered = is_ordered;
    new_var->parameters = parameters;
    new_var->return_type = return_type;
    new_var->C_name = C_name;

    return new_var;
}

/// CALLING ///
varType_ptr staticFuncPntr::is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg)
{
    parameter_to_arguments_mapper param_to_arg_map( parameters, argument_node_ptr );

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
        argument_node_ptr->print(MSG);
        MSG << " at " << argument_node_ptr->loc;
        MSG << endl;
        MSG << " cannot be called because " << param_to_arg_map.error << endl;
        error_msg = MSG.str();

        return nullptr;
    }
}

exp_writer_ptr staticFuncPntr::write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
  vector<utf8_string>& argument_Cexpressions, ostream& output, bool call_virtual)
{

    auto function_writer = make_shared<basic_function_call_writer>( output, parameters, argument_node );

    vector< utf8_string > params_to_write;

    function_writer->mapper.write_arguments(argument_Cexpressions, params_to_write, output);


    stringstream out;

    out << '(' << ( LHS_Cexp->get_C_expression() ) << '(';

    bool write_comma = false;

    for(auto &param : params_to_write)
    {
        if( write_comma )
        {
            out << ", ";
        }
        else
        {
            write_comma = true;
        }

        out << param;
    }

    out << "))" ;

    function_writer->exp = out.str();


    return function_writer;
}

/// ASSIGNMENT ///
bool staticFuncPntr::get_has_assignment(varType* RHS_type)
{
    if( RHS_type->type_of_type == varType::function_pntr_t )
    {
        return is_equivalent( RHS_type );
    }
    else if( RHS_type->type_of_type == varType::defined_function_t )
    {
        DefFuncType* RHS_def_func = dynamic_cast<DefFuncType*>(RHS_type);
        if( RHS_def_func->overloads.size()==1 )
        {
            DefFuncType::ResolvedFunction_ptr single_overload = RHS_def_func->overloads.front();

            bool param_eq = parameters->is_equivalent( single_overload->parameters.get() );
            bool return_eq = return_type->is_equivalent( single_overload->return_type.get() );

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

void staticFuncPntr::write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
                                  utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    write_destructor(LHS, output);

    if( RHS_type->type_of_type == varType::function_pntr_t )
    {
        output << LHS << '=' << RHS_exp <<';';
    }
    else if( RHS_type->type_of_type == varType::defined_function_t )
    { // we can't trust the RHS name here.
        auto defined_func = dynamic_cast<DefFuncType*>(RHS_type);
        output << LHS << '=' << defined_func->single_overload_Cname() <<';';// probably only place single_overload_Cname is used?
    }
}

shared_ptr<varType> staticFuncPntr::get_auto_type()
{
    return self.lock();
}

/// DEFINITION ///

void staticFuncPntr::C_definition_name(utf8_string& var_name, ostream& output)
{
    stringstream OUT;
    OUT << "(*" << var_name << ")";
    OUT << "(";
    parameters->write_to_C(OUT);
    OUT << ")";

    utf8_string name = OUT.str();
    return_type->C_definition_name(name, output);
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

varType_ptr DefFuncType::copy(utf8_string _definition_name )
{
    if( _definition_name == "" )
    {
        _definition_name = definition_name;
    }

    auto new_var = make_shared<DefFuncType>( _definition_name, loc);
    new_var->is_ordered = is_ordered;
    new_var->C_name = C_name;
    new_var->overloads = overloads;

//     for(auto old_overload : overloads)
//     {
//         cout << old_scope->name << " " << old_overload->defining_scope->name << endl;
//
//         if( old_scope->is_or_inScope( old_overload->defining_scope ) )
//         {
//             auto new_overload = make_shared<resolved_func>(  );
//             new_overload->is_ordered = false;
//             new_overload->defining_scope = new_scope;
//             new_overload->define_loc = old_overload->define_loc;
//             new_overload->c_reference = old_overload->c_reference;
//             new_overload->parameters = old_overload->parameters;
//             new_overload->return_type = old_overload->return_type;
//
//             cout << "new scope " << new_scope << endl;
//
//             new_var->overloads.push_back( new_overload );
//         }
//     }

    return new_var;
}

DefFuncType::ResolvedFunction_ptr DefFuncType::define_overload(location_span defining_loc, sym_table_base* defining_scope, func_param_ptr parameters, bool is_ordered)
{
    // first check that new overload does not conflict with current overloads
//     for(auto OL : overloads)
//     {
//         first check if distinguish by signiture
//         if( OL->parameters->is_distinguishable( parameters ) )
//         {
//             continue;// can distinguish by signiture
//         }
//
//         cannot distinguish, see if can override by scope
//         if( defining_scope->is_sameScope( OL->defining_scope ) )
//         {
//             cout << "function defined twice in same scope " << OL->define_loc << " and " << defining_loc << endl;
//             return nullptr; // cannot distinguish between functions!
//         }
//     }

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
DefFuncType::resolved_pair DefFuncType::get_resolution(call_argument_list* argument_node_ptr, utf8_string& error_msg)
{
    ResolvedFunction_ptr current_overload = nullptr;
    shared_ptr<parameter_to_arguments_mapper> current_argument_map = nullptr;

    list< pair<ResolvedFunction_ptr, utf8_string> > attempted_resolutions;

    for(auto OL : overloads)
    {
        // first check this overload is reasonable
        //if( not OL->defining_scope->is_or_inScope( calling_scope ) )
//         if( not calling_scope->is_or_inScope( OL->defining_scope ) )
//         {
//             cout << "arg " << calling_scope << "  " << OL->defining_scope << endl;
//             attempted_resolutions.push_back( make_pair( OL, "overload not in scope" ) );
//             continue;
//         }

        if( OL->is_ordered and not argument_node_ptr->loc.strictly_GT( OL->define_loc ) )
        {
            attempted_resolutions.push_back( make_pair( OL, "overload defined after call" ) );
            continue;
        }

        shared_ptr<parameter_to_arguments_mapper> new_argument_map = make_shared<parameter_to_arguments_mapper>(OL->parameters, argument_node_ptr);
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
        argument_node_ptr->print( TMP );
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

varType_ptr DefFuncType::is_callable(call_argument_list* argument_node_ptr, utf8_string& error_msg)
{
    auto OV = get_resolution( argument_node_ptr, error_msg );
    if(OV.first)
        return OV.first->return_type;
    return nullptr;
}

exp_writer_ptr DefFuncType::write_call(call_argument_list* argument_node, exp_writer_ptr LHS_Cexp,
        vector<utf8_string>& argument_Cexpressions, ostream& output, bool call_virtual)
{
    utf8_string TMP;
    auto OV = get_resolution( argument_node, TMP );
    if( not OV.first )
        return nullptr;

    auto func = OV.first;

    auto writer = make_shared<basic_function_call_writer>(output, *OV.second);

    vector< utf8_string > params_to_write;

    writer->mapper.write_arguments(argument_Cexpressions, params_to_write, output);

    stringstream out;

    out << '(' << func->c_reference << '(';

    bool write_comma = false;

    for(auto &param : params_to_write)
    {
        if( write_comma )
        {
            out << ", ";
        }
        else
        {
            write_comma = true;
        }

        out << param;
    }

    out << "))" ;

    writer->exp = out.str();

    return writer;

}


shared_ptr<varType> DefFuncType::get_auto_type()
{
    if( overloads.size() == 1 )
    {
        ResolvedFunction_ptr single_overload = overloads.front();

        auto funcPntr = make_shared<staticFuncPntr>();
        funcPntr->loc = single_overload->define_loc; // probably not right? probably doesn't matter, becouse these arn't proper symbols
        funcPntr->is_ordered = single_overload->is_ordered;
        funcPntr->self = funcPntr;
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

void DefFuncType::resolved_func::write_C_prototype(std::ostream& output)
{
    stringstream OUT;
    OUT << "(";
    OUT << c_reference;
    OUT << "(";
    parameters->write_to_C(OUT);
    OUT << "))";

    utf8_string name = OUT.str();
    return_type->C_definition_name(name, output);
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

bool DefClassType::is_equivalent(varType* RHS)
{
    //return ((void*)this) == ((void*)RHS);
    return (RHS->type_of_type==varType::defined_class_t) and (C_name == RHS->C_name); // I hope this works.
}

varType_ptr DefClassType::copy(utf8_string _definition_name )
{
//    if( _definition_name == "" )
//    {
//        _definition_name = definition_name;
//    }
//
//    auto new_var = make_shared<DefClassType>( _definition_name, C_name, is_ordered, class_symbol_table, loc );
//    new_var->vtableType_cname = vtableType_cname;
//    new_var->global_vtableVar_cname = global_vtableVar_cname;
   // return new_var;

    cout<< "using DefClassType::copy, which... is problamatic..."<<endl;
    return as_class();
}


/// DEFINITION ///
void DefClassType::C_definition_name(utf8_string& var_name, ostream& output)
{
    output << "struct " << C_name << " " << var_name;
}

void DefClassType::initialize(utf8_string& var_exp, std::ostream& output)
{
    // initilize the vtable
    output << "(" << var_exp << ").__cy_vtable = &" << global_vtableVar_cname << ";" << endl;

    // now initialize children
    // note that this initializes the immediate parents!!    however, all the vtables are wrong!
    // this is wasteful, as vtables are set for every parent for every level. Need check to see if is parent, initilize differently
    for( auto &X : class_symbol_table->variable_table )
    {
        auto def_name = X.first;
        auto type = X.second->var_type;

        auto get_exp = write_member_getref( var_exp, def_name, output );
        auto exp_str = get_exp->get_C_expression();
        type->initialize( exp_str, output );
    }

    // now we initialize the vtables of all parents
    for( int parent_index=0; parent_index!=full_inheritance_tree.size(); ++parent_index )
    {
        auto parent_reference = write_parent_access(parent_index, var_exp, output);
        output << "(" << parent_reference << ").__cy_vtable = &" << global_parentVtable_cnames[parent_index] << ";" << endl;
    }
}


/// REFERENCING ///
bool DefClassType::can_get_pointer(varType* output_type)
{
    if(   is_equivalent( output_type )  )
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

utf8_string DefClassType::get_pointer(varType* output_type, utf8_string& exp, ostream& output)
{
    if(is_equivalent( output_type ) )
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
                auto ret = write_parent_access(parent_pointer, exp, output);
                ret = "&(" + exp + ")";
                return ret;
            }
        }

        throw gen_exception("ERROR: TYPE ", definition_name, " cannot be referenced as ", output_type->definition_name,
                            ". In DefClassType::get_pointer, should not be reached." );
    }
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

exp_writer_ptr DefClassType::write_member_getref(utf8_string& expression, utf8_string& member_name,
      ostream& output, bool call_virtual)
{
    //location_span tmploc;
    //bool check_order = false;
    //auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    int parent_index;
    auto varname = get_member_full_inheritance(parent_index, member_name);

    if( not varname)
    {
        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
    }

    utf8_string expression_to_use = expression;
    if( parent_index != -1 )
    {
        expression_to_use = write_parent_access( parent_index, expression, output );
    }

    if( varname->var_type->type_of_type == varType::method_function_t)
    {
        //return expression_to_use;
        return make_shared< MethodType::method_access_writer >(expression_to_use, call_virtual);
    }
    else
    {
        stringstream out;
        out << "(" << expression_to_use << "." << varname->C_name << ")";
        return make_shared< simple_expression_writer >( out.str() );
        //return out.str();
    }
}

varType_ptr DefClassType::member_has_getter(utf8_string& member_name)
{
    return member_has_getref(member_name);
}

exp_writer_ptr DefClassType::write_member_getter(csu::utf8_string& expression, utf8_string& member_name,
                ostream& output, bool call_virtual)
{
    return write_member_getref( expression, member_name, output, call_virtual);
}

varType_ptr DefClassType::set_member(utf8_string& member_name, varType* RHS_type)
{
    // first get the member
    //location_span tmploc;
    //bool check_order = false;
    //auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    int parent_index;
    auto varname = get_member_full_inheritance(parent_index, member_name);

    if( not varname)
    {
        // don't give error, just return
        return nullptr;
    }

    if( RHS_type )
    {
        bool has_assign = varname->var_type->get_has_assignment( RHS_type )  or  RHS_type->get_has_assignTo( varname->var_type.get() );
        if( not has_assign )
        {
            return nullptr;
        }
    }

    // if here, then we are good
    return varname->var_type;
}

void DefClassType::write_member_setter(expression_AST_node* exp_AST_node, utf8_string& LHS_expression, utf8_string& member_name,
             varType* RHS_type, utf8_string& RHS_expression, std::ostream& output, bool call_virtual)
{
    // WELL! this is going to be nice and complicated!

    // first get the member
    //location_span tmploc;
    //bool check_order = false;
    //auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    int parent_index;
    auto varname = get_member_full_inheritance(parent_index, member_name);

    if( not varname)
    {
        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
    }

    // now write full LHS code
    utf8_string LHS = "(";
    if( parent_index==-1 )
    {
        LHS += LHS_expression;
    }
    else
    {
        LHS += write_parent_access(parent_index,  LHS_expression, output );
    }
    LHS += utf8_string(".");
    LHS += varname->C_name;
    LHS += utf8_string(")");

    // now assign to it!
    if( varname->var_type->get_has_assignment( RHS_type ) )
    {
        varname->var_type->write_assignment(RHS_type, exp_AST_node, LHS, RHS_expression, output);
    }
    else if( RHS_type->get_has_assignTo( varname->var_type.get() ))
    {
        RHS_type->write_assignTo(varname->var_type.get(), exp_AST_node, LHS, RHS_expression, output);
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
void DefClassType::write_default_constructor(csu::utf8_string& var_name, std::ostream& output)
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

    auto init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
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

    //// NOW WE CALL THE FUNCTION ////
    vector<utf8_string> fake_param_variables;
    call_argument_list fake_arguments(loc, nullptr, nullptr );
    fake_arguments.symbol_table = class_symbol_table.get();

    auto var_name_writer = make_shared<MethodType::method_access_writer>( var_name, false );

    auto writer = init_method_type->write_call( &fake_arguments, var_name_writer, fake_param_variables, output );

    output << writer->get_C_expression() << ';' << endl;
    writer->write_cleanup();
}

bool DefClassType::has_explicit_constructor(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg)
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
        auto ret = init_method_var->var_type->is_callable( argument_node_ptr, TMP );
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
        auto ret = exInit_method_var->var_type->is_callable( argument_node_ptr, TMP );
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

void DefClassType::write_explicit_constructor(call_argument_list* argument_node,
               utf8_string& LHS_Cexp, vector<csu::utf8_string>& argument_Cexpressions, ostream& output)
{
    auto var_name_writer = make_shared<MethodType::method_access_writer>( LHS_Cexp, false );

    // try implicit
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( init_method_var )
    {
        utf8_string tmp;
        auto ret = init_method_var->var_type->is_callable( argument_node, tmp );
        if( ret )
        {
            auto writer = init_method_var->var_type->write_call(argument_node, var_name_writer, argument_Cexpressions, output );
            output << writer->get_C_expression() << ';' <<endl;
            writer->write_cleanup();
            return;
        }
    }

    // now try explicit!
    utf8_string exInitSTR("__exInit__");
    check_order = false;
    auto exInit_method_var = class_symbol_table->get_variable_local(exInitSTR, tmp_loc, check_order);
    if( exInit_method_var )
    {
        auto writer = exInit_method_var->var_type->write_call(argument_node, var_name_writer, argument_Cexpressions, output );
        output << writer->get_C_expression() << ';' <<endl;
        writer->write_cleanup();
    }

}

bool DefClassType::has_implicit_copy_constructor(varType* RHS_type)
{
    // default constructor??
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( not init_method_var )
    {// no init method!
        return false;
    }

    auto init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
    for( auto &instance : init_method_type->overloads )
    {
        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
        {
            auto TYP = instance->parameters->required_parameters.front()->var_type;

            if( TYP->type_of_type == c_pointer_reference )
            {
                auto TYP_casted = dynamic_pointer_cast<raw_C_pointer_reference>(TYP);
                if( TYP_casted->referenced_type->is_equivalent(RHS_type) )
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void DefClassType::write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node, utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( not init_method_var )
    {// no init method! eventually this should never happen?
        return;
    }



    auto init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
    MethodType::ResolvedMethod_ptr found_method = nullptr;
    for( auto &instance : init_method_type->overloads )
    {
        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
        {
            auto TYP = instance->parameters->required_parameters.front()->var_type;

            if( TYP->type_of_type == c_pointer_reference )
            {
                auto TYP_casted = dynamic_pointer_cast<raw_C_pointer_reference>(TYP);
                if( TYP_casted->referenced_type->is_equivalent(RHS_type) )
                {
                    found_method = instance;
                    break;
                }
            }
        }
    }

    if( not found_method)
    {
        throw gen_exception("cannot find overload in DefClassType::write_implicit_copy_constructor. This should not be reaached!");
    }


    vector<utf8_string> argument_Cexpressions;
    argument_Cexpressions.push_back(RHS_exp);

    vector<expression_AST_node*> argument_expressions;
    argument_expressions.push_back(RHS_AST_node);


    auto var_name_writer = make_shared<MethodType::method_access_writer>( LHS, false );

    auto writer = init_method_type->write_call(found_method, argument_expressions,
               RHS_AST_node->symbol_table, var_name_writer, argument_Cexpressions, output);
    output << writer->get_C_expression() << ';' <<endl;
    writer->write_cleanup();
}


/// destructors ///
void DefClassType::write_destructor(csu::utf8_string& var_name, std::ostream& output, bool call_virtual)
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
    vector<utf8_string> fake_param_variables;
    call_argument_list fake_arguments(loc, nullptr, nullptr );
    fake_arguments.symbol_table = class_symbol_table.get();


//    exp_writer_ptr writer;
//    if( call_virtual )
//    {
//        auto var_name_writer = make_shared<MethodType::method_access_writer>( var_name, true );
//        writer = del_method_var->var_type->write_call(&fake_arguments, var_name_writer, fake_param_variables, output );
//    }
//    else
//    {
//        auto var_name_writer = make_shared<MethodType::method_access_writer>( var_name, false );
//        //auto methodTYPE = del_method_var->var_type->as_method();
//        //writer = methodTYPE->write_nonvirtual_call(&fake_arguments, var_name_writer, fake_param_variables, output );
//        writer = del_method_var->var_type->write_call(&fake_arguments, var_name_writer, fake_param_variables, output );
//    }

    auto var_name_writer = make_shared<MethodType::method_access_writer>( var_name, call_virtual );
    exp_writer_ptr writer = del_method_var->var_type->write_call(&fake_arguments, var_name_writer, fake_param_variables, output );

    output << writer->get_C_expression() << ';' <<endl;
    writer->write_cleanup();
}

/// ASSIGNMENT ///  just replicates copy constructor (implicit copy constructror now, should be explicit)
/// This needs FIXING!!///
bool DefClassType::get_has_assignment(varType* RHS_type)
{
    // default constructor??
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( not init_method_var )
    {// no init method!
        return false;
    }

    auto init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
    for( auto &instance : init_method_type->overloads )
    {
        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
        {
            auto TYP = instance->parameters->required_parameters.front()->var_type;

            if( TYP->type_of_type == c_pointer_reference )
            {
                auto TYP_casted = dynamic_pointer_cast<raw_C_pointer_reference>(TYP);
                if( TYP_casted->referenced_type->is_equivalent(RHS_type) )
                {
                    return true;
                }
            }
        }
    }
    return false;
}


void DefClassType::write_assignment(varType* RHS_type, expression_AST_node* RHS_AST_node,
   utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    utf8_string initSTR("__init__");
    location_span tmp_loc; // some default location
    bool check_order = false;
    auto init_method_var = class_symbol_table->get_variable_local(initSTR, tmp_loc, check_order);
    if( not init_method_var )
    {
        throw gen_exception("cannot find overload in DefClassType::write_implicit_copy_constructor. This should not be reaached!");
    }



    auto init_method_type = dynamic_pointer_cast<MethodType>( init_method_var->var_type );
    MethodType::ResolvedMethod_ptr found_method = nullptr;
    for( auto &instance : init_method_type->overloads )
    {
        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
        {
            auto TYP = instance->parameters->required_parameters.front()->var_type;

            if( TYP->type_of_type == c_pointer_reference )
            {
                auto TYP_casted = dynamic_pointer_cast<raw_C_pointer_reference>(TYP);
                if( TYP_casted->referenced_type->is_equivalent(RHS_type) )
                {
                    found_method = instance;
                    break;
                }
            }
        }
    }

    if( not found_method)
    {
        throw gen_exception("cannot find overload in DefClassType::write_implicit_copy_constructor. This should not be reaached!");
    }


    // write destructor
    write_destructor(LHS, output);


    // now assignment!
    vector<utf8_string> argument_Cexpressions;
    argument_Cexpressions.push_back(RHS_exp);

    vector<expression_AST_node*> argument_expressions;
    argument_expressions.push_back(RHS_AST_node);


    auto var_name_writer = make_shared<MethodType::method_access_writer>( LHS, false );

    auto writer = init_method_type->write_call(found_method, argument_expressions,
               RHS_AST_node->symbol_table, var_name_writer, argument_Cexpressions, output);
    output << writer->get_C_expression() << ';' <<endl;
    writer->write_cleanup();
}

/// CASTING TO OTHER TYPES ///
// can be inherited and virtual
bool DefClassType::can_implicit_castTo(varType* LHS_type)
{
    utf8_string convertSTR("__convert__");

    int parent_index;
    auto convert_method_var = get_member_full_inheritance(parent_index, convertSTR);

    if( not convert_method_var )
    {
        return false;
    }

    auto convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
    for( auto &instance : convert_method_type->overloads )
    {
        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
        {
            auto TYP = instance->parameters->required_parameters.front()->var_type;

            if( TYP->type_of_type == c_pointer_reference )
            {
                auto TYP_casted = dynamic_pointer_cast<raw_C_pointer_reference>(TYP);
                if( TYP_casted->referenced_type->is_equivalent(LHS_type) )
                {
                    return true;
                }
            }
        }
    }
    return false;
}


void DefClassType::write_implicit_castTo(varType* LHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS, csu::utf8_string& RHS_exp,
                std::ostream& output, bool call_virtual)
{
    utf8_string convertSTR("__convert__");

    int parent_index;
    auto convert_method_var = get_member_full_inheritance(parent_index, convertSTR);

    if( not convert_method_var )
    {
        throw gen_exception("cannot find overload in DefClassType::write_implicit_castTo. This should not be reached!");
    }


    auto convert_method_type = dynamic_pointer_cast<MethodType>( convert_method_var->var_type );
    MethodType::ResolvedMethod_ptr found_method = nullptr;
    for( auto &instance : convert_method_type->overloads )
    {
        if( (instance->parameters->required_parameters.size() == 1) and (instance->parameters->optional_parameters.size() == 0))
        {
            auto TYP = instance->parameters->required_parameters.front()->var_type;

            if( TYP->type_of_type == c_pointer_reference )
            {
                auto TYP_casted = dynamic_pointer_cast<raw_C_pointer_reference>(TYP);
                if( TYP_casted->referenced_type->is_equivalent(LHS_type) )
                {
                    found_method = instance;
                    break;
                }
            }
        }
    }

    if( not found_method)
    {
        throw gen_exception("cannot find overload in DefClassType::write_implicit_castTo. This should not be reached!");
    }



    utf8_string expression_to_use = RHS_exp;
    if( parent_index != -1 )
    {
        expression_to_use = write_parent_access( parent_index, RHS_exp, output );
    }

    auto var_name_writer = make_shared< MethodType::method_access_writer >(expression_to_use, call_virtual);




    // now assignment!
    vector<utf8_string> argument_Cexpressions;
    argument_Cexpressions.push_back(LHS);


// I HOPE THIS WORKS!! SOOOO hacky!!
    auto LHS_exp = make_shared<varReferance_expression_AST_node>( LHS, RHS_AST_node->loc );
    LHS_exp->has_output_ownership = false;
    LHS_exp->c_exp_can_be_referenced = true;
    LHS_exp->variable_symbol = make_shared<varName>();
    LHS_exp->variable_symbol->var_type = LHS_type->shared_from_this();
    LHS_exp->variable_symbol->C_name = LHS;
    LHS_exp->variable_symbol->loc = RHS_AST_node->loc ;
    LHS_exp->variable_symbol->is_ordered = false;
    LHS_exp->expression_return_type = LHS_exp->variable_symbol->var_type;


    vector<expression_AST_node*> argument_expressions;
    argument_expressions.push_back( LHS_exp.get() );


    //auto var_name_writer = make_shared<MethodType::method_access_writer>( RHS_exp, call_virtual );
    auto writer = convert_method_type->write_call(found_method, argument_expressions,
               RHS_AST_node->symbol_table, var_name_writer, argument_Cexpressions, output);

    output << writer->get_C_expression() << ';' <<endl;
    writer->write_cleanup();
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
        while( (overload_iter != overload_end_iter) and not ( do_inhereted_overloads or method_self_type->is_equivalent( (*overload_iter)->self_ptr_name->var_type.get() ) ) )
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
    }
    return (++(*this));


//    while( method_iter != method_end_iter )
//    {
//        if( method_iter->second->var_type->type_of_type == varType::method_function_t )
//        {
//            // yay!
//            auto method = method_get();
//            overload_iter = method->overloads.begin();
//            overload_end_iter = method->overloads.end();
//            break;
//        }
//        ++method_iter;
//    }
//
//    return *this;
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
        if( par_class->is_equivalent( parent_class.get() ) )
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
        if( par_class->is_equivalent( parent_class.get() ) )
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

utf8_string DefClassType::write_parent_access(int parent_index, utf8_string& exp, ostream& output)
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

    ret = "(" + exp + ")" + ret;
    return ret;
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

varType_ptr MethodType::copy( utf8_string _definition_name )
{
    if( _definition_name == "" )
    {
        _definition_name = definition_name;
    }

    auto new_var = make_shared<MethodType>( _definition_name, loc, self_ptr_name );
    new_var->num_undefined_overloads = num_undefined_overloads;

    return new_var;
}

 /// CALLING ///

MethodType::resolved_pair MethodType::get_resolution(call_argument_list* argument_node_ptr, utf8_string& error_msg)
{

    ResolvedMethod_ptr current_overload = nullptr;
    shared_ptr<parameter_to_arguments_mapper> current_argument_map = nullptr;

    list< pair<ResolvedMethod_ptr, utf8_string> > attempted_resolutions;

    for(auto OL : overloads)
    {

        shared_ptr<parameter_to_arguments_mapper> new_argument_map = make_shared<parameter_to_arguments_mapper>(OL->parameters, argument_node_ptr);

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
        argument_node_ptr->print( TMP );
        TMP << " at " << argument_node_ptr->loc ;
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

varType_ptr MethodType::is_callable(call_argument_list* argument_node_ptr, csu::utf8_string& error_msg)
{
    auto OL = get_resolution( argument_node_ptr, error_msg );
    if(OL.first)
        return OL.first->return_type;
    else
        return nullptr;
}


exp_writer_ptr MethodType::write_call(call_argument_list* argument_node,
               exp_writer_ptr LHS_Cexp, vector<utf8_string>& argument_Cexpressions, ostream& output, bool call_virtual)
    // NOTE: call_virtual doesn't do anything, virtuality is controlled via LHS_Cexp
{

    // note that LHS_Cexp is reference to the class object..I think

    utf8_string tmp;
    auto OV = get_resolution( argument_node, tmp );
    if( not OV.first)
        return nullptr;

    auto func = OV.first;

    auto writer = make_shared<basic_function_call_writer>(output, *OV.second);

    vector< utf8_string > params_to_write;

    writer->mapper.write_arguments(argument_Cexpressions, params_to_write, output);

    stringstream out;

    out << '(' ;

    auto method_access = dynamic_pointer_cast<MethodType::method_access_writer>( LHS_Cexp );
    auto parent_class_CExp = method_access->get_C_expression();
    bool virtual_call_allowed = method_access->virtual_call_allowed;

    if( func->is_virtual and virtual_call_allowed )
    {
        bool tmp;
        auto self_type = self_ptr_name->var_type->is_reference_type(tmp)->as_class(); // this is very clumsy


        if( func->overriden_method )
        {
            utf8_string method_name = func->overriden_method->c_reference;
            utf8_string class_exp = self_type->write_parent_access(func->parental_vtable_location, parent_class_CExp, output );
            utf8_string vtable_entry = "(" + class_exp + ").__cy_vtable->" + method_name ;
            utf8_string self_ptr_exp = "((void*)&("+class_exp+"))-" + vtable_entry+"_offset";

            out << "(" << vtable_entry << ")( " << self_ptr_exp;
        }
        else
        {
            utf8_string method_name = func->c_reference;
            utf8_string vtable_entry = "(" + parent_class_CExp + ").__cy_vtable->" + method_name ;
            utf8_string self_ptr_exp = "((void*)&("+parent_class_CExp+"))-" + vtable_entry+"_offset";

            out << "(" << vtable_entry << ")( " << self_ptr_exp;
        }
    }
    else
    {
        out << func->c_reference << endl;
        out << "( (void*)&(" << parent_class_CExp << ") ";
    }


    for(auto &param : params_to_write)
    {
        out << ", " << param;
    }

    out << "))" ;

    writer->exp = out.str();

    return writer;

}

//exp_writer_ptr MethodType::write_nonvirtual_call(call_argument_list* argument_node,
//               utf8_string& LHS_Cexp, vector<utf8_string>& argument_Cexpressions, ostream& output)
//{
//    // note that LHS_Cexp is reference to the class object..I think
//
//    utf8_string tmp;
//    auto OV = get_resolution( argument_node, tmp );
//    if( not OV.first)
//        return nullptr;
//
//    auto func = OV.first;
//    auto writer = make_shared<basic_function_call_writer>(output, *OV.second);
//
//    vector< utf8_string > params_to_write;
//
//    writer->mapper.write_arguments(argument_Cexpressions, params_to_write, output);
//
//    stringstream out;
//
//    out << '(' ;
//
//
//    out << func->c_reference << endl;
//    out << "( (void*)&(" << LHS_Cexp << ") ";
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
//
//}

exp_writer_ptr MethodType::write_call(ResolvedMethod_ptr resolved_method, vector<expression_AST_node*> &argument_expressions,
               sym_table_base* _symbol_table, exp_writer_ptr LHS_Cexp, vector<utf8_string>& argument_Cexpressions, ostream& output)
{
// note that LHS_Cexp is reference to the class object..I think

    auto writer = make_shared<basic_function_call_writer>(output, resolved_method->parameters, argument_expressions, _symbol_table);

    if( not writer->mapper.is_good )
    {// no resolution found
        cout<< "cannot resolve overload with arguments: " ;
        writer->mapper.arguments.print( cout );
        cout << endl;
        cout << "overload defined " << resolved_method->define_loc << " with parameters ";
        resolved_method->parameters->print( cout );
        cout<<endl;
        cout << "  is invalid because "<< writer->mapper.error <<endl;
        cout << endl;
        return nullptr;
    }

    vector< utf8_string > params_to_write;

    writer->mapper.write_arguments(argument_Cexpressions, params_to_write, output);

    stringstream out;
    out << '(' ;

    // it's annoying how much this duplicates from previous call

    auto method_access = dynamic_pointer_cast<MethodType::method_access_writer>( LHS_Cexp );
    auto parent_class_CExp = method_access->get_C_expression();
    bool virtual_call_allowed = method_access->virtual_call_allowed;

    if( resolved_method->is_virtual and virtual_call_allowed  )
    {
        bool tmp;
        auto self_type = self_ptr_name->var_type->is_reference_type(tmp)->as_class(); // this is very clumsy


        if( resolved_method->overriden_method )
        {
            utf8_string method_name = resolved_method->overriden_method->c_reference;
            utf8_string class_exp = self_type->write_parent_access(resolved_method->parental_vtable_location, parent_class_CExp, output );
            utf8_string vtable_entry = "(" + class_exp + ").__cy_vtable->" + method_name ;
            utf8_string self_ptr_exp = "((void*)&("+class_exp+"))-" + vtable_entry+"_offset";

            out << "(" << vtable_entry << ")( " << self_ptr_exp;
        }
        else
        {
            utf8_string method_name = resolved_method->c_reference;
            utf8_string vtable_entry = "(" + parent_class_CExp + ").__cy_vtable->" + method_name ;
            utf8_string self_ptr_exp = "((void*)&("+parent_class_CExp+"))-" + vtable_entry+"_offset";

            out << "(" << vtable_entry << ")( " << self_ptr_exp;
        }
    }
    else
    {
        out << resolved_method->c_reference << endl;
        out << "( (void*)&(" << parent_class_CExp << ") ";
    }


    for(auto &param : params_to_write)
    {
        out << ", " << param;
    }

    out << "))" ;

    writer->exp = out.str();

    return writer;
}



void MethodType::resolved_method::write_C_prototype(std::ostream& output)
{
    stringstream OUT;
    OUT << "(";
    OUT << c_reference;
    OUT << "(";

    //self_ptr_name->var_type->C_definition_name(self_ptr_name->C_name, OUT);
    OUT << "void* " << self_ptr_name->C_name << "_";
    if( parameters->total_size() > 0 )
    {
        OUT << ",";
    }

    parameters->write_to_C(OUT, false);
    OUT << "))";

    utf8_string name = OUT.str();
    return_type->C_definition_name(name, output);
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
    return not self_ptr_name->var_type->is_equivalent( ovrld->self_ptr_name->var_type.get() );
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

varName_ptr sym_table_base::get_variable_local(csu::utf8_string& var_name, csu::location_span& ref_loc, bool& check_order)
{
    auto table_itterator = variable_table.find( var_name );
    if(table_itterator != variable_table.end())
    {
        // var found
        varName_ptr variable = table_itterator->second;
        if( variable->is_ordered and not ref_loc.strictly_GT( variable->loc )  )
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
        if( type->is_ordered and not ref_loc.strictly_GT( type->loc )  )
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
    variable_type->set_pointers(variable_type,variable_type);
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

