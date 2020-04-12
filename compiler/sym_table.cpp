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

using namespace csu;
using namespace std;

///// function writer base ////
void function_call_writer::write_call(utf8_string& LHS, ostream& output )
{
    output << LHS;
}


/////// types /////////

// varType (BASE)

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
bool varType::is_callable()
{
    return false;
}


funcCallWriter_ptr varType::get_resolved_call(location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments)
{
    throw gen_exception("ERROR: TYPE IS NOT CALLABLE ", call_loc);
    return nullptr;
}

/// ADDITION ///
bool varType::get_has_LHSaddition(varType* RHS_type)
{
    return false;
}

void varType::write_LHSaddition(varType* RHS_type, utf8_string& LHS_exp, utf8_string& RHS_exp, ostream& output)
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

void varType::write_assignment(varType* RHS_type,  utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    throw gen_exception("ERROR: TYPE ", definition_name, " does not have assignment ");
}

varType_ptr varType::get_auto_type()
{
    return nullptr;
}

/// CLASS THINGS ///
varType_ptr varType::member_has_getter(csu::utf8_string& member_name)
{
    return nullptr;
}

varType_ptr varType::member_has_getref(csu::utf8_string& member_name)
{
    return nullptr;
}

varType_ptr varType::set_member(utf8_string& member_name, varType* RHS_type)
{
    return nullptr;
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

funcCallWriter_ptr varType_fromC::get_resolved_call(location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments)
{
    if( arguments->named_arguments.size() !=0 )
    {
        cout<< "C-function cannot take named arguments. Called at "<<call_loc<<endl;
        return nullptr;
    }

    // make fake parameter info
    func_param_ptr fake_parameters = make_shared<func_parameter_info>();
    fake_parameters->required_parameters.reserve( arguments->unnamed_arguments.size() );

    for( int arg_i=0; arg_i<arguments->unnamed_arguments.size(); ++arg_i )
    {
        varType_ptr argument_type = arguments->unnamed_arguments[ arg_i ];

        // assume type is fully defined
        if(not argument_type->can_be_defined())
        {
            cout << "type " << argument_type->definition_name << " cannot be defined. Referenced "  << call_loc << endl;
            return nullptr;
        }

        varName_ptr param_name = make_shared<varName>();
        param_name->var_type = argument_type;
        param_name->definition_name = "P";
        param_name->definition_name += to_string(arg_i);
        param_name->C_name = param_name->definition_name;
        param_name->is_ordered = false; //?

        fake_parameters->required_parameters.push_back( param_name );
    }

    // make writer
    funcCallWriter_ptr output_writer = make_shared<function_call_writer>();

    //bool check_order = false;
    //utf8_string void_name = "void";
    //output_writer->return_type = calling_scope->get_type_global(void_name, call_loc, check_order);
    output_writer->return_type = unnamed_C_type.lock();
    output_writer->param_to_arg_map = make_shared<arguments_to_parameter_mapper>(arguments, fake_parameters);

    return output_writer;
}

/// ADDITION ///
bool varType_fromC::get_has_LHSaddition(varType* RHS_type)
{
    return RHS_type->type_of_type == varType::c_import_t;
}

varType_ptr varType_fromC::get_LHSaddition_type(varType* RHS_type)
{
    return unnamed_C_type.lock();
}

void varType_fromC::write_LHSaddition(varType* RHS_type, utf8_string& LHS_exp, utf8_string& RHS_exp, std::ostream& output)
{
    output << '(' << LHS_exp << '+' << RHS_exp << ')';
}

/// ASSIGNMENT ///
bool varType_fromC::get_has_assignment(varType* RHS_type)
{
    return RHS_type->type_of_type == varType::c_import_t;
}

void varType_fromC::write_assignment(varType* RHS_type,  utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    output << LHS << '=' << RHS_exp <<';';
}

shared_ptr<varType> varType_fromC::get_auto_type()
{
    return self.lock();
}




// raw c-pointer references
raw_C_pointer_reference::raw_C_pointer_reference(varType_ptr _ref_type)
{
    referenced_type = _ref_type;
    type_of_type = varType::c_pointer_reference;
    definition_name = "";
    C_name = _ref_type->C_name;
    loc = _ref_type->loc;
    is_ordered = false;
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
    return referenced_type->can_be_defined();
}

void raw_C_pointer_reference::C_definition_name(utf8_string& var_name, ostream& output)
{
    utf8_string new_var_name = "*" + var_name;
    referenced_type->C_definition_name(new_var_name, output);
}

/// CALLING
bool raw_C_pointer_reference::is_callable()
{
    return referenced_type->is_callable();
}

funcCallWriter_ptr raw_C_pointer_reference::get_resolved_call(location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments)
{
    return referenced_type->get_resolved_call( call_loc, calling_scope, arguments);
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

void raw_C_pointer_reference::write_LHSaddition(varType* RHS_type, utf8_string& LHS_exp, utf8_string& RHS_exp, ostream& output)
{
    utf8_string newLHS = "(*" + LHS_exp;
    newLHS += ")";
    return referenced_type->write_LHSaddition(RHS_type, newLHS, RHS_exp, output);
}

/// ASSIGNMENT
bool raw_C_pointer_reference::get_has_assignment(varType* RHS_type)
{
    return referenced_type->get_has_assignment(RHS_type);
}

void raw_C_pointer_reference::write_assignment(varType* RHS_type, utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
    utf8_string newLHS = "(*" + LHS;
    newLHS += ")";
    referenced_type->write_assignment(RHS_type, LHS,  RHS_exp, output);
}

/// CLASS members

varType_ptr raw_C_pointer_reference::member_has_getter(utf8_string& member_name)
{
    return referenced_type->member_has_getter( member_name );
}

utf8_string raw_C_pointer_reference::write_member_getter(utf8_string& expression, utf8_string& member_name, ostream& output)
{
    utf8_string newLHS = "(*" + expression;
    newLHS += ")";
    return referenced_type->write_member_getter(newLHS, member_name,  output);
}

varType_ptr raw_C_pointer_reference::member_has_getref(utf8_string& member_name)
{
    return referenced_type->member_has_getref( member_name );
}

utf8_string raw_C_pointer_reference::write_member_getref(utf8_string& expression, utf8_string& member_name, ostream& output)
{
    utf8_string newLHS = "(*" + expression;
    newLHS += ")";
    return referenced_type->write_member_getref(newLHS, member_name,  output);
}

varType_ptr raw_C_pointer_reference::set_member(utf8_string& member_name, varType* RHS_type)
{
    return referenced_type->set_member(member_name, RHS_type);
}


void raw_C_pointer_reference::write_member_setter(utf8_string& LHS_expression, utf8_string& member_name, varType* RHS_type, utf8_string& RHS_expression, ostream& output)
{
    utf8_string newLHS = "(*" + LHS_expression;
    newLHS += ")";
    return referenced_type->write_member_setter(newLHS, member_name, RHS_type, RHS_expression, output);
}






// function parameters
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


// function call arguments
int funcCall_argument_info::total_size()
{
    return unnamed_arguments.size() + named_arguments.size();
}

varType_ptr funcCall_argument_info::argType_from_index(int i)
{
    if( i<unnamed_arguments.size() )
    {
        return unnamed_arguments[i];
    }
    else if( i< total_size() )
    {
        return named_arguments[ i-unnamed_arguments.size() ];
    }
    else
    {
        throw gen_exception("index is too large in funcCall_argument_info::argType_from_index");
    }
}

void funcCall_argument_info::print(std::ostream& output)
{
    output <<"( ";
    for( auto type : unnamed_arguments )
    {
        output << type->definition_name <<", ";
    }

    for( int i=0; i<named_arguments.size(); ++i)
    {
        auto type = named_arguments[i];
        auto name = argument_names[i];

        output << name<<'='<<type->definition_name<<", ";
    }
    output <<")";
}

// how to map arguments to parameters/
arguments_to_parameter_mapper::arguments_to_parameter_mapper(callArg_types_ptr _arguments, func_param_ptr _parameters)
{
    arguments = _arguments;
    parameters = _parameters;

    is_good = true; // change if not true

    // increment when needed
    num_defaults_used = 0;
    num_casts = 0;

    int num_arguments = arguments->total_size();
    int num_unnamed_args = arguments->unnamed_arguments.size();
    int num_named_args = arguments->named_arguments.size();

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
        int parameter_i = parameters->index_from_name(  arguments->argument_names[named_arg_i] );
        if( parameter_i==-1 )
        {
            error = "no parameter has the name ";
            error += arguments->argument_names[named_arg_i];
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
            auto arg_type = arguments->argType_from_index(arg_index);

            if( not arg_type->is_equivalent(param_type.get()) ) // types not exactly the same. need to cast!
            {
                if( param_type->get_has_assignment(arg_type.get()) )
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
            // else: no cast needed

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

int arguments_to_parameter_mapper::comparison(funcCall_param_map_ptr RHS)
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
            return 0;
        }
    }
}

// function pointer
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

/// CALLING ///

funcCallWriter_ptr staticFuncPntr::get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments)
{
    funcCallWriter_ptr writer = make_shared<function_call_writer>();

    writer->param_to_arg_map = make_shared<arguments_to_parameter_mapper>(arguments, parameters);
    writer->return_type = return_type;

    if( writer->param_to_arg_map->is_good )
    {
        return writer;
    }
    else
    {
        cout << "function defined on " << loc << endl;
        cout << "  with parameters: ";
        parameters->print( cout );
        cout<<endl;
        cout<< " called with arguments: ";
        arguments->print(cout);
        cout<<endl;
        cout<< " cannot be called because "<<writer->param_to_arg_map->error << endl;

        return nullptr;
    }
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

void staticFuncPntr::write_assignment(varType* RHS_type, utf8_string& LHS, utf8_string& RHS_exp, ostream& output)
{
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

DefFuncType::ResolvedFunction_ptr DefFuncType::define_overload(location_span defining_loc, sym_table_base* defining_scope, func_param_ptr parameters, bool is_ordered)
{
    // first check that new overload does not conflict with current overloads
    for(auto OL : overloads)
    {
        // first check if distinguish by signiture
        if( OL->parameters->is_distinguishable( parameters ) )
        {
            continue;// can distinguish by signiture
        }

        // cannot distinguish, see if can override by scope
        if( defining_scope->is_sameScope( OL->defining_scope ) )
        {
            cout << "function defined twice in same scope " << OL->define_loc << " and " << defining_loc << endl;
            return nullptr; // cannot distinguish between functions!
        }
    }

    ResolvedFunction_ptr new_overload = make_shared<resolved_func>( );
    new_overload->defining_scope = defining_scope;
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
bool DefFuncType::is_callable()
{
    return true;
}

funcCallWriter_ptr DefFuncType::get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments)
{
    ResolvedFunction_ptr current_overload = nullptr;
    funcCall_param_map_ptr current_argument_map = nullptr;

    list< pair<ResolvedFunction_ptr, utf8_string> > attempted_resolutions;

    for(auto OL : overloads)
    {
        // first check this overload is reasonable
        //if( not OL->defining_scope->is_or_inScope( calling_scope ) )
        if( not calling_scope->is_or_inScope( OL->defining_scope ) )
        {
            attempted_resolutions.push_back( make_pair( OL, "overload not in scope" ) );
            continue;
        }

        if( OL->is_ordered and not call_loc.strictly_GT( OL->define_loc ) )
        {
            attempted_resolutions.push_back( make_pair( OL, "overload defined after call" ) );
            continue;
        }

        funcCall_param_map_ptr new_argument_map = make_shared<arguments_to_parameter_mapper>(arguments, OL->parameters);
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
            // signitures are the same, so now we check which one has the most inner scope
            else if( current_overload->defining_scope->is_or_inScope( OL->defining_scope ) )
            {
                // OL is the most inner (note they cannot be the same scope)
                current_overload = OL;
                current_argument_map = new_argument_map;
            }
        }
    }

    if( not current_overload )
    {// no resolution found
        cout<< "cannot resolve overload with arguments: " ;
        arguments->print( cout );
        cout << endl;
        for( auto P : attempted_resolutions )
        {
            cout << "overload defined " << P.first->define_loc << " with parameters ";
            P.first->parameters->print( cout );
            cout<<endl;
            cout << "  is invalid because "<<P.second<<endl;
        }

        return nullptr;
    }
    else
    {
        writer_ptr new_writer = make_shared<writer>();
        new_writer->param_to_arg_map = current_argument_map;
        new_writer->c_name = current_overload->c_reference;
        new_writer->return_type = current_overload->return_type;
        return new_writer;
    }
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

void DefFuncType::writer::write_call(csu::utf8_string& LHS, std::ostream& output )
{
    output << c_name;
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
}

bool DefClassType::is_equivalent(varType* RHS)
{
    return ((void*)this) == ((void*)RHS);
}

/// DEFINITION ///
void DefClassType::C_definition_name(utf8_string& var_name, ostream& output)
{
    output << "struct " << C_name << " " << var_name ;
}

/// CLASS THINGS ///
//shared_ptr<varType> DefClassType::access_member(utf8_string& member_name)
//{
//    location_span tmploc;
//    bool check_order = false;
//    auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);
//
//    if(varname)
//    {
//        return varname->var_type;
//    }
//    else
//    {
//        return nullptr;
//    }
//}
//
//void DefClassType::write_member_access(utf8_string& expression, utf8_string& member_name, ostream& output)
//{
//    location_span tmploc;
//    bool check_order = false;
//    auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);
//
//    if( not varname)
//    {
//        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
//    }
//
//    output << "(" << expression << "." << varname->C_name << ")";
//}

varType_ptr DefClassType::member_has_getref(utf8_string& member_name)
{
    location_span tmploc;
    bool check_order = false;
    auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    if(varname)
    {
        return varname->var_type;
    }
    else
    {
        return nullptr;
    }
}

utf8_string DefClassType::write_member_getref(utf8_string& expression, utf8_string& member_name, ostream& output)
{
    location_span tmploc;
    bool check_order = false;
    auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    if( not varname)
    {
        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
    }

    if( varname->var_type->type_of_type == varType::method_function_t)
    {
        return expression;
    }
    else
    {
        stringstream out;
        out << "(" << expression << "." << varname->C_name << ")";
        return out.str();
    }
}

varType_ptr DefClassType::member_has_getter(utf8_string& member_name)
{
    return member_has_getref(member_name);
}

utf8_string DefClassType::write_member_getter(utf8_string& expression, utf8_string& member_name, ostream& output)
{
    return write_member_getref(expression, member_name, output);
}


varType_ptr DefClassType::set_member(utf8_string& member_name, varType* RHS_type)
{
    // first get the member
    location_span tmploc;
    bool check_order = false;
    auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    if( not varname)
    {
        // don't give error, just return
        return nullptr;
    }

    if( RHS_type )
    {
        bool has_assign = varname->var_type->get_has_assignment( RHS_type );
        if( not has_assign )
        {
            return nullptr;
        }
    }

    // if here, then we are good
    return varname->var_type;
}

void DefClassType::write_member_setter(utf8_string& LHS_expression, utf8_string& member_name, varType* RHS_type, utf8_string& RHS_expression, ostream& output)
{
    // WELL! this is going to be nice and complicated!

    // first get the member
    location_span tmploc;
    bool check_order = false;
    auto varname = class_symbol_table->get_variable_local(member_name, tmploc, check_order);

    if( not varname)
    {
        throw gen_exception("ERROR in DefClassType::write_member_access. This should never be reached.");
    }

    // now write full LHS code
    utf8_string LHS = "(";
    LHS += LHS_expression;
    LHS += utf8_string(".");
    LHS += varname->C_name;
    LHS += utf8_string(")");

    // now assign to it!
    // assume we have assignment
    varname->var_type->write_assignment(RHS_type, LHS, RHS_expression, output);

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
    new_overload->define_loc = defining_loc;
    new_overload->c_reference = defining_scope->namespace_unique_name + "__";
    new_overload->c_reference += definition_name;
    new_overload->c_reference += utf8_string("__");
    new_overload->c_reference += defining_scope->get_unique_string();
    new_overload->return_type = nullptr;
    new_overload->parameters = parameters;
    new_overload->self_ptr_name = self_ptr_name;

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

 /// CALLING ///
bool MethodType::is_callable()
{
    return true;
}

funcCallWriter_ptr MethodType::get_resolved_call(csu::location_span call_loc, sym_table_base* calling_scope, callArg_types_ptr arguments)
{
    ResolvedMethod_ptr current_overload = nullptr;
    funcCall_param_map_ptr current_argument_map = nullptr;

    list< pair<ResolvedMethod_ptr, utf8_string> > attempted_resolutions;

    for(auto OL : overloads)
    {

        funcCall_param_map_ptr new_argument_map = make_shared<arguments_to_parameter_mapper>(arguments, OL->parameters);
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
        cout<< "cannot resolve overload with arguments: " ;
        arguments->print( cout );
        cout << endl;
        for( auto P : attempted_resolutions )
        {
            cout << "overload defined " << P.first->define_loc << " with parameters ";
            P.first->parameters->print( cout );
            cout<<endl;
            cout << "  is invalid because "<<P.second<<endl;
        }

        return nullptr;
    }
    else
    {
        writer_ptr new_writer = make_shared<writer>();
        new_writer->param_to_arg_map = current_argument_map;
        new_writer->c_name = current_overload->c_reference;
        new_writer->return_type = current_overload->return_type;
        return new_writer;
    }
}

void MethodType::resolved_method::write_C_prototype(std::ostream& output)
{
    stringstream OUT;
    OUT << "(";
    OUT << c_reference;
    OUT << "(";

    self_ptr_name->var_type->C_definition_name(self_ptr_name->C_name, OUT);
    if( parameters->total_size() > 0 )
    {
        OUT << ",";
    }

    parameters->write_to_C(OUT, false);
    OUT << "))";

    utf8_string name = OUT.str();
    return_type->C_definition_name(name, output);
}

void MethodType::writer::write_call(csu::utf8_string& LHS, std::ostream& output )
{
    output << c_name;
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

utf8_string local_sym_table::get_unique_string()
{
    return parent_table->get_unique_string();
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

utf8_string module_sym_table::get_unique_string()
{
    utf8_string out_string = to_string(next_variable_ID);
    next_variable_ID += 1;
    return out_string;
}

