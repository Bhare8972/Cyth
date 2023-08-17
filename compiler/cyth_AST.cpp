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

This file defines the Cyth abstract sytax tree
*/

#include <fstream>
#include <memory>
#include <algorithm>

#include "cyth_AST.hpp"
#include "AST_visitor.hpp"

using namespace std;
using namespace csu;


/////// basic AST node buisness /////

AST_node::AST_node()
{
    symbol_table = 0;
    verification_state = -1;
    parent_node = nullptr;
}

void AST_node::apply_visitor(AST_visitor_base* visitor)
{
    try
    {
        apply_visitor_inner( visitor );
    }
    catch(...)
    {
        cout << "exception in visitor to " << AST_node_name() << " at " << loc << endl;
        throw;
    }
}

void AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    throw "not implemented";
}

////// module  ///////

//module_AST_node::module_AST_node( csu::utf8_string name ) :
//    top_symbol_table( name cle)
module_AST_node::module_AST_node()
{
    //module_name = name;
    symbol_table = &top_symbol_table;
    max_symbol_loops = 1000; //probably define this elsewhere.
    main_status = -2;
}

void module_AST_node::add_AST_node(AST_node_ptr new_AST_element)
{
    module_contents.push_back( new_AST_element );
    loc = loc + new_AST_element->loc;
}

void module_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{

    visitor->module_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( module_contents.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;
    for( auto& node :  module_contents)
    {
        AST_visitor_base* child = visitor->get_child(i);
        visitor_children.push_back( child );
        node->apply_visitor( child );
        i ++;
    }

    visitor->module_up( this, visitor_children );
    visitor->ASTnode_up( this );

}

void module_AST_node::find_main_func()
{
    utf8_string func_name = "__main__";
    location_span LOC;
    bool order = false;
    varName_ptr main_ptr = top_symbol_table.get_variable_global( func_name, LOC, order );

    if( main_ptr )
    {
        /// there is a thing called __main__
        /// check type
        varType_ptr name_type_ptr = main_ptr->var_type;
        if( name_type_ptr->type_of_type == varType::defined_function_t )
        {
            DefFuncType* defined_func = dynamic_cast<DefFuncType*>(name_type_ptr.get());
            if( defined_func->overloads.size()==1 )
            {
                auto OL = defined_func->overloads.front();
                func_name = "void";
                order = false;
                auto VOID_type = top_symbol_table.get_type_global( func_name, LOC, order );

                if( (OL->return_type==VOID_type) and (OL->parameters->total_size()==0) )
                {
                    main_status = 1;
                    main_func_name = OL->c_reference;
                }
                else
                {
                    cout<<"ERROR: __main__ has wrong signature. Needs to be (void)__main()"<<endl;
                    main_status = -1;
                }
            }
            else
            {
                cout<<"ERROR: __main__ defined multiple times"<<endl;
                main_status = -1;
            }
        }
        else
        {
            main_status = 0;
        }
    }
    else
    {
        main_status = 0;
    }
}

module_AST_node::compiler_command::compiler_command( string _command, location_span _loc )
{
    command = _command;
    loc = _loc;
}

void module_AST_node::add_CompilerCommand( utf8_string _command, location_span _loc )
{
    compiler_comands.emplace_back( _command.to_cpp_string(), _loc );
}



///// IMPORTS /////

//import from C
import_C_AST_node::import_C_AST_node(csu::utf8_string _file_name, utf8_string _import_name, location_span _loc)
{
    import_name = _import_name;
    usage_name = _import_name;
    loc = _loc;
    import_file_name = _file_name;

    type = nullptr;
    variable = nullptr;
    variable_type = nullptr;
}

import_C_AST_node::import_C_AST_node(utf8_string _import_name, location_span _loc)
{
    import_name = _import_name;
    usage_name = _import_name;
    loc = _loc;

    type = nullptr;
    variable = nullptr;
    variable_type = nullptr;
}

void import_C_AST_node::set_usage_name(utf8_string _usage_name)
{
    usage_name = _usage_name;
}


void import_C_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->cImports_down( this );
    visitor->allImports_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->cImports_up( this );
    visitor->allImports_up( this );
    visitor->ASTnode_up( this );
}

//cyth imports
import_cyth_AST_node::import_cyth_AST_node(csu::utf8_string _import_module_string, utf8_string _import_name, location_span _loc)
{
    import_name = _import_name;
    usage_name = _import_name;
    loc = _loc;
    import_module_string = _import_module_string;

    //type = nullptr;
    //variable = nullptr;
    //variable_type = nullptr;
}

//import_cyth_AST_node::import_cyth_AST_node(utf8_string _import_name, location_span _loc)
//{
//    import_name = _import_name;
//    usage_name = _import_name;
//    loc = _loc;
//
//    //type = nullptr;
//    //variable = nullptr;
//    //variable_type = nullptr;
//}

void import_cyth_AST_node::set_usage_name(utf8_string _usage_name)
{
    usage_name = _usage_name;
}


void import_cyth_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->CythImports_down( this );
    visitor->allImports_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->CythImports_up( this );
    visitor->allImports_up( this );
    visitor->ASTnode_up( this );
}


/// CLASSES ///

// var definition
General_VarDefinition::General_VarDefinition()
{
    variable_symbol = nullptr;
}

class_varDefinition_AST_node::class_varDefinition_AST_node(varType_ASTrepr_ptr _var_type, utf8_string _var_name, location_span _loc)
{
    var_type = _var_type;
    var_name = _var_name;
    loc = _loc;
    default_value = nullptr;
}


class_varDefinition_AST_node::class_varDefinition_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name, expression_AST_ptr _default_value, csu::location_span _loc)
{
    var_type = _var_type;
    var_name = _var_name;
    loc = _loc;
    default_value = _default_value;
}

void class_varDefinition_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->ClassVarDef_down( this );
    visitor->typed_VarDef_down(this);
    visitor->generic_VarDef_down(this);
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    int num = 1;
    if( default_value ){ num++; }
    visitor->initiate_children( num );

    AST_visitor_base* varType_visitor = visitor->get_child(0);
    var_type->apply_visitor( varType_visitor );

    AST_visitor_base* default_value_visitor = nullptr;
    if( default_value )
    {
        default_value_visitor = visitor->get_child(1);
        default_value->apply_visitor( default_value_visitor );
    }


    visitor->ClassVarDef_up( this, varType_visitor, default_value_visitor);
    visitor->typed_VarDef_up(this, varType_visitor);
    visitor->generic_VarDef_up(this);
    visitor->statement_up( this );
    visitor->ASTnode_up( this );
}

//inheritance list
void inheritanceList_AST_node::add_item(csu::utf8_string &item, csu::location_span &_loc)
{
    if( class_IDs.size() == 0 )
    {
        loc = _loc;
    }
    else
    {
        loc = loc + _loc;
    }

    class_IDs.emplace_back( item );
}


void inheritanceList_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->inheritanceList_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->inheritanceList_up( this );
    visitor->ASTnode_up( this );
}

// actual class
class_AST_node::class_AST_node(csu::utf8_string _name, inheritanceList_AST_ptr _inheritanceList, csu::location_span _loc)
{
    name = _name;
    loc = _loc;
    type_ptr = nullptr;
    inner_symbol_table = nullptr;

    inheritanceList = _inheritanceList;

    write_default_constructor = false;
    default_constructor_overload = nullptr;

    write_default_deconstructor = false;
    default_destructor_overload = nullptr;

    write_selfCopy_constructor = false;
    default_CopyConstructor_overload = nullptr;
}

void class_AST_node::add_var_def(ClassVarDef_AST_ptr new_variable_definition)
{
    loc = loc + new_variable_definition->loc;
    var_definitions.push_back( new_variable_definition );
}

void class_AST_node::add_method_def(method_AST_ptr new_method_definition)
{
    loc = loc + new_method_definition->loc;
    method_definitions.push_back( new_method_definition );
}

void class_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->ClassDef_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    bool has_inheritance = (inheritanceList!=nullptr);
    visitor->initiate_children( var_definitions.size() + method_definitions.size() + has_inheritance);

    int i = 0;
    list<AST_visitor_base*> var_def_visitor_children;
    for(auto var_def_child : var_definitions)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        var_def_visitor_children.push_back( child_visitor );
        var_def_child->apply_visitor( child_visitor );
        i += 1;
    }

    list<AST_visitor_base*> method_def_visitor_children;
    for(auto method_child : method_definitions)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        method_def_visitor_children.push_back( child_visitor );
        method_child->apply_visitor( child_visitor );
        i += 1;
    }

    AST_visitor_base* inheritance_list_child = nullptr;
    if( has_inheritance )
    {
        inheritance_list_child = visitor->get_child(i) ;
        inheritanceList->apply_visitor( inheritance_list_child );
    }

    visitor->ClassDef_up( this, var_def_visitor_children, method_def_visitor_children, inheritance_list_child );
    visitor->ASTnode_up( this );
}



// construct block
construct_AST_node::construct_AST_node(csu::location_span initial_loc)
{
    loc = initial_loc;
}

void construct_AST_node::add(constructElement_AST_ptr _newElement)
{
    contents.push_back( _newElement );
}

void construct_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->constructBlock_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    visitor->initiate_children( contents.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;

    for(auto child : contents)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        visitor_children.push_back( child_visitor );
        child->apply_visitor( child_visitor );
        i += 1;
    }

    visitor->constructBlock_up( this, visitor_children );
    visitor->ASTnode_up( this );
}



constructElement_AST_node::constructElement_AST_node(expression_AST_ptr _expression,  argumentList_AST_ptr _argument_list, csu::location_span initial_loc)
{
    loc = initial_loc;
    expression = _expression;
    argument_list = _argument_list;
}

void constructElement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->constructElement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );
    AST_visitor_base* ex_child = visitor->get_child(0) ;
    expression->apply_visitor( ex_child );
    AST_visitor_base* args_child = visitor->get_child(1) ;
    argument_list->apply_visitor( args_child );

    visitor->constructElement_up( this, ex_child, args_child );
    visitor->ASTnode_up( this );
}


/// FUNCTIONS ////

// block
block_AST_node::block_AST_node(location_span initial_loc)
{
    loc = initial_loc;
}

void block_AST_node::add_AST_node(AST_node_ptr new_AST_element, location_span _loc)
{
    contents.push_back( new_AST_element );
    loc = loc + _loc;
}

void block_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->block_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    visitor->initiate_children( contents.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;

    for(auto child : contents)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        visitor_children.push_back( child_visitor );
        child->apply_visitor( child_visitor );
        i += 1;
    }

    visitor->block_up( this, visitor_children );
    visitor->ASTnode_up( this );
}

// all callable definitions
callableDefinition_AST_node::callableDefinition_AST_node()
{
    return_type = nullptr;
    paramList = nullptr;
    returnType_mode = by_returnStatement_M;

    num_returns = 0;
    num_verified_returns = 0;
}

// function
function_AST_node::function_AST_node(utf8_string _name, location_span _loc, paramList_AST_ptr _paramList, block_AST_ptr _block)
{
    name = _name;
    loc = _loc;

    block_AST = _block;
    return_type_ASTnode = nullptr;
    paramList = _paramList;

    funcName = nullptr;
    funcType = nullptr;
    specific_overload = nullptr;

    inner_symbol_table = nullptr;
}

function_AST_node::function_AST_node(utf8_string _name, varType_ASTrepr_ptr _return_type_ASTnode, location_span _loc, paramList_AST_ptr _paramList, block_AST_ptr _block)
{
    name = _name;
    loc = _loc;

    block_AST = _block;
    return_type_ASTnode = _return_type_ASTnode;
    returnType_mode = by_explicitDefinition_M;
    paramList = _paramList;

    funcName = nullptr;
    funcType = nullptr;
    specific_overload = nullptr;

    inner_symbol_table = nullptr;
}

void function_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->funcDef_down( this );
    visitor->callableDef_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    int i = 0;
    if( return_type_ASTnode ){ i++; }

    visitor->initiate_children( 2+i );

    AST_visitor_base* returnType_child = 0;
    if( return_type_ASTnode )
    {
        returnType_child = visitor->get_child(0) ;
        return_type_ASTnode->apply_visitor( returnType_child );
    }

    AST_visitor_base* paramList_child = visitor->get_child(0+i) ;
    paramList->apply_visitor( paramList_child );
    AST_visitor_base* funcBody_child = visitor->get_child(1+i) ;
    block_AST->apply_visitor( funcBody_child );

    visitor->funcDef_up( this, returnType_child, paramList_child, funcBody_child );
    visitor->callableDef_up( this, paramList_child );
    visitor->ASTnode_up( this );
}

void function_AST_node::notify_return_type()
{
    // try to finish defining this type...
    funcType->define_overload_return( specific_overload, return_type );
}

// function parameters
function_parameter_list::function_parameter_list(location_span _loc, required_ptr _required_list, default_ptr _defaulted_list)
{
    loc = _loc;
    required_list = _required_list;
    defaulted_list = _defaulted_list;
}

func_param_ptr function_parameter_list::get_parameter_info()
{
    auto paramInfo = make_shared<func_parameter_info>();

    if( required_list )
    {
        paramInfo->required_parameters.reserve( required_list->param_list.size() );
        for(auto& param : required_list->param_list)
        {
            if( not param.variable_symbol )
            {
                throw gen_exception("variable symbol in parameters not set. In function_parameter_list::get_parameter_info");
            }
            paramInfo->required_parameters.push_back(param.variable_symbol);
        }
    }
    if(defaulted_list)
    {
        paramInfo->optional_parameters.reserve( defaulted_list->param_list.size() );
        for(auto& param : defaulted_list->param_list)
        {
            if( not param.variable_symbol )
            {
                throw gen_exception("variable symbol in parameters not set. In function_parameter_list::get_parameter_info");
            }
            paramInfo->optional_parameters.push_back(param.variable_symbol);
        }
    }
    return paramInfo;
}

void function_parameter_list::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->funcParams_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    int num_children = 0;
    if( required_list ){ num_children+=1; }
    if( defaulted_list ){ num_children+=1; }
    visitor->initiate_children( num_children );

    AST_visitor_base* reqList_child = 0;
    int child_i = 0;
    if( required_list )
    {
        reqList_child = visitor->get_child(child_i);
        child_i += 1;
        required_list->apply_visitor( reqList_child );
    }

    AST_visitor_base* defaultList_child = 0;
    if( defaulted_list )
    {
        defaultList_child = visitor->get_child(child_i);
        defaulted_list->apply_visitor( defaultList_child );
    }

    visitor->funcParams_up( this, reqList_child, defaultList_child );
    visitor->ASTnode_up( this );
}

function_parameter_list::required_params::required_params( location_span _loc )
{
    loc = _loc;
}

void function_parameter_list::required_params::add_typed_param( varType_ASTrepr_ptr _var_type, utf8_string& _var_name, location_span _loc )
{
    loc = loc + _loc;

    param_list.emplace_back();
    param_holder& new_params = param_list.back();
    new_params.loc = _loc;
    new_params.variable_symbol = nullptr;
    new_params.var_name = _var_name;
    new_params.var_type_ASTnode = _var_type;
}

void function_parameter_list::required_params::apply_visitor_inner(AST_visitor_base* visitor)
{

    visitor->reqParams_down( this );
    visitor->baseParams_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    visitor->initiate_children( param_list.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;

    for(auto child : param_list)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        visitor_children.push_back( child_visitor );
        child.var_type_ASTnode->apply_visitor( child_visitor );
        i += 1;
    }

    visitor->reqParams_up( this, visitor_children );
    visitor->baseParams_up( this, visitor_children  );
    visitor->ASTnode_up( this );

}

function_parameter_list::defaulted_params::defaulted_params( location_span _loc )
{
    loc = _loc;
}

void function_parameter_list::defaulted_params::add_typed_param(  varType_ASTrepr_ptr _var_type, csu::utf8_string& _var_name,
                                                                expression_AST_ptr default_exp, csu::location_span _loc  )
{
    loc = loc + _loc;

    param_list.emplace_back();
    param_holder& new_params = param_list.back();
    new_params.loc = _loc;
    new_params.variable_symbol = nullptr;
    new_params.var_name = _var_name;
    new_params.var_type_ASTnode = _var_type;

    parameter_defaults.push_back( default_exp );
}

void function_parameter_list::defaulted_params::apply_visitor_inner(AST_visitor_base* visitor)
{

    visitor->defaultParams_down( this );
    visitor->baseParams_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    visitor->initiate_children( param_list.size()*2 );

    int i = 0;
    std::list<AST_visitor_base*> param_name_visitors;
    std::list<AST_visitor_base*> default_exp_visitors;

    auto expr_iter  = parameter_defaults.begin();
    for(auto child : param_list)
    {
        AST_visitor_base* name_visitor = visitor->get_child(i) ;
        param_name_visitors.push_back( name_visitor );
        child.var_type_ASTnode->apply_visitor( name_visitor );
        i += 1;

        AST_visitor_base* exp_visitor = visitor->get_child(i) ;
        default_exp_visitors.push_back( exp_visitor );
        (*expr_iter)->apply_visitor( exp_visitor );
        i += 1;

        ++ expr_iter;
    }

    visitor->defaultParams_up( this, param_name_visitors, default_exp_visitors );
    visitor->baseParams_up( this, param_name_visitors  );
    visitor->ASTnode_up( this );
}


// function call arguments.
call_argument_list::call_argument_list(csu::location_span _loc, un_arguments_ptr _unnamed_list, named_arguments_ptr _named_list)
{
    loc = _loc;
    unnamed_argument_list = _unnamed_list;
    named_argument_list = _named_list;
}

void call_argument_list::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->callArguments_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    int num_children = 0;
    if( unnamed_argument_list ){ num_children+=1; }
    if( named_argument_list ){ num_children+=1; }
    visitor->initiate_children( num_children );

    AST_visitor_base* unArgs_child = 0;
    int child_i = 0;
    if( unnamed_argument_list )
    {
        unArgs_child = visitor->get_child(child_i);
        child_i += 1;
        unnamed_argument_list->apply_visitor( unArgs_child );
    }

    AST_visitor_base* namedArgs_child = 0;
    if( named_argument_list )
    {
        namedArgs_child = visitor->get_child(child_i);
        named_argument_list->apply_visitor( namedArgs_child );
    }

    visitor->callArguments_up( this, unArgs_child, namedArgs_child );
    visitor->ASTnode_up( this );
}

int call_argument_list::num_unnamed()
{
    if( unnamed_argument_list )
    {
        return unnamed_argument_list->arguments.size();
    }
    else
    {
        return 0;
    }
}

int call_argument_list::num_named()
{
    if( named_argument_list )
    {
        return named_argument_list->arguments.size();
    }
    else
    {
        return 0;
    }
}

int call_argument_list::total_size()
{
    return num_unnamed() + num_named();
}

expression_AST_ptr call_argument_list::unNamedExp_by_index(int i)
{
    int num_unnamed_ = num_unnamed();
    if( i > num_unnamed_ )
    {
        throw gen_exception("i is too large in call_argument_list::unNamedExp_by_index");
    }
    else
    {
        return unnamed_argument_list->arguments[i];
    }
}

expression_AST_ptr call_argument_list::namedExp_by_index(int i)
{
    int num_named_ = num_named();
    if( i > num_named_ )
    {
        throw gen_exception("i is too large in call_argument_list::namedExp_by_index");
    }
    else
    {
        return named_argument_list->arguments[i];
    }
}

utf8_string call_argument_list::namedExpName_by_index(int i)
{
    int num_named_ = num_named();
    if( i > num_named_ )
    {
        throw gen_exception("i is too large in call_argument_list::namedExp_by_index");
    }
    else
    {
        return named_argument_list->names[i];
    }
}

expression_AST_ptr call_argument_list::expression_from_index(int i)
{
    int num_unnamed_ = num_unnamed();
    int num_named_ = num_named();

    if( i < num_unnamed_)
    {
        auto it = unnamed_argument_list->arguments.begin();
        advance(it, i);
        return *it;
    }
    else if( i < (num_unnamed_+num_named_))
    {
        i -= num_unnamed_;
        auto it = named_argument_list->arguments.begin();
        advance(it, i);
        return *it;
    }
    else
    {
        throw gen_exception("i is too large in call_argument_list::expression_from_index");
    }
}

void call_argument_list::print(std::ostream& output)
{
    bool do_comma = false;

    output <<"( ";

    if( unnamed_argument_list )
    {
        for( auto exp : unnamed_argument_list->arguments )
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
    }

    if( named_argument_list )
    {
        int num = num_named();
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

            auto exp = named_argument_list->arguments[i];
            auto name = named_argument_list->names[i];

            output << name << '=' << exp->expression_return_type->definition_name;
        }
    }

    output <<")";
}

function_argument_types_ptr call_argument_list::get_argument_types()
{
    auto out = make_shared<function_argument_types>();
    int num = num_unnamed() ;
    out->unnamed_argument_types.reserve( num );
    for(int i=0; i< num; i++)
    {
        out->unnamed_argument_types.push_back( unNamedExp_by_index(i)->expression_return_type );
    }

    num = num_named() ;
    out->named_argument_names.reserve( num );
    out->named_argument_types.reserve( num );
    for(int i=0; i< num; i++)
    {
        out->named_argument_types.push_back( namedExp_by_index(i)->expression_return_type );
        out->named_argument_names.push_back( namedExpName_by_index(i) );
    }

    return out;
}

call_argument_list::unnamed_arguments_T::unnamed_arguments_T( csu::location_span _loc )
{
    loc = _loc;
}

void call_argument_list::unnamed_arguments_T::add_argument( expression_AST_ptr new_argument, csu::location_span _loc )
{
    loc = loc+_loc;
    arguments.push_back(new_argument);
}

void call_argument_list::unnamed_arguments_T::apply_visitor_inner(AST_visitor_base* visitor)
{

    visitor->unArguments_down( this );
    visitor->baseArguments_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    visitor->initiate_children( arguments.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;
    for(auto child : arguments)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        visitor_children.push_back( child_visitor );
        child->apply_visitor( child_visitor );
        i += 1;
    }

    visitor->unArguments_up( this, visitor_children );
    visitor->baseArguments_up( this, visitor_children  );
    visitor->ASTnode_up( this );

}

call_argument_list::named_arguments_T::named_arguments_T( csu::location_span _loc )
{
    loc = _loc;
}

void call_argument_list::named_arguments_T::add_argument( csu::utf8_string& name, expression_AST_ptr new_argument, csu::location_span _loc )
{
    loc = loc+_loc;
    arguments.push_back(new_argument);
    names.push_back(name);
}

void call_argument_list::named_arguments_T::apply_visitor_inner(AST_visitor_base* visitor)
{

    visitor->namedArguments_down( this );
    visitor->baseArguments_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }
    visitor->initiate_children( arguments.size() );

    int i = 0;
    std::list<AST_visitor_base*> visitor_children;
    for(auto child : arguments)
    {
        AST_visitor_base* child_visitor = visitor->get_child(i) ;
        visitor_children.push_back( child_visitor );
        child->apply_visitor( child_visitor );
        i += 1;
    }

    visitor->namedArguments_up( this, visitor_children );
    visitor->baseArguments_up( this, visitor_children );
    visitor->ASTnode_up( this );

}






// METHODS
method_AST_node::method_AST_node(utf8_string _name, location_span _loc, paramList_AST_ptr _paramList, block_AST_ptr _block)
{
    name = _name;
    loc = _loc;

    block_AST = _block;
    return_type_ASTnode = nullptr;
    paramList = _paramList;

    methodName = nullptr;
    funcType = nullptr;
    specific_overload = nullptr;

    inner_symbol_table = nullptr;
}

method_AST_node::method_AST_node(utf8_string _name, varType_ASTrepr_ptr _return_type_ASTnode, location_span _loc, paramList_AST_ptr _paramList, block_AST_ptr _block)
{
    name = _name;
    loc = _loc;

    block_AST = _block;
    return_type_ASTnode = _return_type_ASTnode;
    returnType_mode = by_explicitDefinition_M;
    paramList = _paramList;

    methodName = nullptr;
    funcType = nullptr;
    specific_overload = nullptr;

    inner_symbol_table = nullptr;
}

void method_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{

    visitor->methodDef_down( this );
    visitor->callableDef_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    int have_return_node = 0;
    if( return_type_ASTnode ){ have_return_node=1; }

    visitor->initiate_children( 2+have_return_node );

    AST_visitor_base* returnType_child = 0;
    if( return_type_ASTnode )
    {
        returnType_child = visitor->get_child(0) ;
        return_type_ASTnode->apply_visitor( returnType_child );
    }

    AST_visitor_base* paramList_child = visitor->get_child(0+have_return_node) ;
    paramList->apply_visitor( paramList_child );
    AST_visitor_base* funcBody_child = visitor->get_child(1+have_return_node) ;
    block_AST->apply_visitor( funcBody_child );

    visitor->methodDef_up( this, returnType_child, paramList_child, funcBody_child );
    visitor->callableDef_up( this, paramList_child );
    visitor->ASTnode_up( this );

}

void method_AST_node::notify_return_type()
{
    // try to finish defining this type...
    funcType->define_overload_return( specific_overload, return_type );
}





////// names //////

// define type of a variable
varType_ASTrepr_node::varType_ASTrepr_node(utf8_string _name, location_span _loc)
{
    name = _name;
    loc = _loc;
    resolved_type = nullptr;
}

void varType_ASTrepr_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->varTypeRepr_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->varTypeRepr_up( this );
    visitor->ASTnode_up( this );

}




/// conditionals ///
activeCond_AST_node::activeCond_AST_node(expression_AST_ptr _if_expression, block_AST_ptr _block_AST, conditional_AST_ptr _child_conditional)
{
    block_AST = _block_AST;
    inner_symbol_table = nullptr;

    if_expression = _if_expression;
    child_conditional = _child_conditional;
}

//IF
if_AST_node::if_AST_node(expression_AST_ptr _if_expression, block_AST_ptr block_AST, conditional_AST_ptr _child_conditional, location_span _loc ) :
    activeCond_AST_node(_if_expression, block_AST, _child_conditional)
{
    loc = _loc;
    verification_state = -1;
    symbol_table = nullptr;
}

void if_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->ifCond_down( this );
    visitor->activeCond_down( this );
    visitor->conditional_down( this );
    visitor->flowControl_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    int num_children = 2;
    if( child_conditional )
    {
        num_children += 1;
    }

    visitor->initiate_children( num_children );

    AST_visitor_base* ifExp_child = visitor->get_child(0);
    if_expression->apply_visitor( ifExp_child );

    AST_visitor_base* ifBlock_child = visitor->get_child(1);
    block_AST->apply_visitor( ifBlock_child );

    AST_visitor_base* childCond_visit = nullptr;
    if( child_conditional )
    {
        childCond_visit = visitor->get_child(2);
        child_conditional->apply_visitor( childCond_visit );
    }


    visitor->ifCond_up( this, ifExp_child, ifBlock_child, childCond_visit );
    visitor->activeCond_up( this, ifExp_child, ifBlock_child, childCond_visit  );
    visitor->conditional_up( this, ifBlock_child );
    visitor->flowControl_up( this, ifBlock_child );
    visitor->ASTnode_up( this );
}


// ELIF
elif_AST_node::elif_AST_node(expression_AST_ptr _if_expression, block_AST_ptr block_AST, conditional_AST_ptr _child_conditional, location_span _loc ) :
    activeCond_AST_node(_if_expression, block_AST, _child_conditional)
{
    loc = _loc;
    verification_state = -1;
    symbol_table = nullptr;
}

void elif_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->elifCond_down( this );
    visitor->activeCond_down( this );
    visitor->conditional_down( this );
    visitor->flowControl_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    int num_children = 2;
    if( child_conditional )
    {
        num_children += 1;
    }

    visitor->initiate_children( num_children );

    AST_visitor_base* ifExp_child = visitor->get_child(0);
    if_expression->apply_visitor( ifExp_child );

    AST_visitor_base* elifBlock_child = visitor->get_child(1);
    block_AST->apply_visitor( elifBlock_child );

    AST_visitor_base* childCond_visit = nullptr;
    if( child_conditional )
    {
        childCond_visit = visitor->get_child(2);
        child_conditional->apply_visitor( childCond_visit );
    }

    visitor->elifCond_up( this, ifExp_child, elifBlock_child, childCond_visit );
    visitor->activeCond_up( this, ifExp_child, elifBlock_child, childCond_visit  );
    visitor->conditional_up( this, elifBlock_child );
    visitor->flowControl_up( this, elifBlock_child );
    visitor->ASTnode_up( this );
}


else_AST_node::else_AST_node(block_AST_ptr _block_AST, location_span _loc)
{
    loc = _loc;
    verification_state = -1;
    symbol_table = nullptr;

    block_AST = _block_AST;
    inner_symbol_table = nullptr;
}

void else_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->else_down( this );
    visitor->conditional_down( this );
    visitor->flowControl_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );

    AST_visitor_base* elseBlock_child = visitor->get_child(0);
    block_AST->apply_visitor( elseBlock_child );

    visitor->else_up( this, elseBlock_child );
    visitor->conditional_up( this, elseBlock_child );
    visitor->flowControl_up( this, elseBlock_child );
    visitor->ASTnode_up( this );
}



/// LOOPS ///

// while
whileLoop_AST_node::whileLoop_AST_node(block_AST_ptr _block_AST, expression_AST_ptr _whileExp, csu::location_span _loc)
{
    loc = _loc;
    verification_state = -1;
    symbol_table = nullptr;

    block_AST = _block_AST;
    inner_symbol_table = nullptr;

    while_expression = _whileExp;
}

void whileLoop_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->while_down( this );
    visitor->loop_down( this );
    visitor->flowControl_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );


    AST_visitor_base* whileExp_child = visitor->get_child(0);
    while_expression->apply_visitor( whileExp_child );

    AST_visitor_base* block_child = visitor->get_child(1);
    block_AST->apply_visitor( block_child );

    visitor->while_up( this, whileExp_child, block_child );
    visitor->loop_up( this, block_child );
    visitor->flowControl_up( this, block_child );
    visitor->ASTnode_up( this );
}

// for
forLoop_AST_node::forLoop_AST_node(block_AST_ptr _block_AST, expression_AST_ptr _whileExp, statement_AST_ptr _initial_statement, statement_AST_ptr _update_statement, csu::location_span _loc)
{
    loc = _loc;
    verification_state = -1;
    symbol_table = nullptr;

    block_AST = _block_AST;
    inner_symbol_table = nullptr;

    while_expression = _whileExp;
    initial_statement = _initial_statement;
    update_statement = _update_statement;
}

void forLoop_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->for_down( this );
    visitor->loop_down( this );
    visitor->flowControl_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 4 );

    AST_visitor_base* initialStmt_child = visitor->get_child(0);
    initial_statement->apply_visitor( initialStmt_child );

    AST_visitor_base* updateStmt_child = visitor->get_child(1);
    update_statement->apply_visitor( updateStmt_child );

    AST_visitor_base* whileExp_child = visitor->get_child(2);
    while_expression->apply_visitor( whileExp_child );

    AST_visitor_base* block_child = visitor->get_child(3);
    block_AST->apply_visitor( block_child );

    visitor->for_up( this, initialStmt_child, updateStmt_child, whileExp_child, block_child );
    visitor->loop_up( this, block_child );
    visitor->flowControl_up( this, block_child );
    visitor->ASTnode_up( this );
}




/// statements ///

// expression statement
expression_statement_AST_node::expression_statement_AST_node(expression_AST_ptr _expression)
{
    //type_of_statement = statement_AST_node::expression_t;
    expression = _expression;
    loc = expression->loc;
}

void expression_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->expressionStatement_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0) ;
    expression->apply_visitor( child );

    visitor->expressionStatement_up( this, child );
    visitor->statement_up( this );
    visitor->ASTnode_up( this );

}


// definition
definition_statement_AST_node::definition_statement_AST_node(varType_ASTrepr_ptr _var_type, utf8_string _var_name, location_span _loc)
{
    //type_of_statement = statement_AST_node::definition_t;

    var_type = _var_type;
    var_name = _var_name;
    loc = _loc;
    variable_symbol = nullptr;
}

void definition_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->definitionStmt_down( this );
    visitor->typed_VarDef_down( this );
    visitor->generic_VarDef_down(this);
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0);
    var_type->apply_visitor( child );


    visitor->definitionStmt_up( this, child );
    visitor->typed_VarDef_up( this, child);
    visitor->generic_VarDef_up(this);
    visitor->statement_up( this );
    visitor->ASTnode_up( this );
}

// definition and construction
definitionNconstruction_statement_AST_node::definitionNconstruction_statement_AST_node(varType_ASTrepr_ptr _var_type, utf8_string _var_name, argumentList_AST_ptr _argument_list, location_span _loc)
{
    //type_of_statement = statement_AST_node::definition_t;

    var_type = _var_type;
    var_name = _var_name;
    loc = _loc;
    variable_symbol = nullptr;

    argument_list = _argument_list;
}

void definitionNconstruction_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->definitionNconstructionStmt_down( this );
    visitor->typed_VarDef_down(this);
    visitor->generic_VarDef_down(this);
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );
    AST_visitor_base* varType_child = visitor->get_child(0);
    var_type->apply_visitor( varType_child );
    AST_visitor_base* argumentList_child = visitor->get_child(1);
    argument_list->apply_visitor( argumentList_child );

    visitor->definitionNconstruction_up( this, varType_child,  argumentList_child);
    visitor->typed_VarDef_up( this, varType_child);
    visitor->generic_VarDef_up(this);
    visitor->statement_up( this );
    visitor->ASTnode_up( this );
}

// definition and assignment
definitionNassignment_statement_AST_node::definitionNassignment_statement_AST_node(varType_ASTrepr_ptr _var_type, csu::utf8_string _var_name,
                                                                                   expression_AST_ptr _expression, csu::location_span _loc)
    {
    //type_of_statement = statement_AST_node::definition_t;

    var_type = _var_type;
    var_name = _var_name;
    loc = _loc;
    variable_symbol = nullptr;

    expression = _expression;
}

void definitionNassignment_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->definitionNassignmentStmt_down( this );
    visitor->typed_VarDef_down(this);
    visitor->generic_VarDef_down(this);
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );
    AST_visitor_base* varType_child = visitor->get_child(0);
    var_type->apply_visitor( varType_child );
    AST_visitor_base* exp_child = visitor->get_child(1);
    expression->apply_visitor( exp_child );

    visitor->definitionNassignment_up( this, varType_child,  exp_child);
    visitor->typed_VarDef_up( this, varType_child);
    visitor->generic_VarDef_up(this);
    visitor->statement_up( this );
    visitor->ASTnode_up( this );
}



// assignment
assignment_statement_AST_node::assignment_statement_AST_node( LHSref_AST_ptr _LHS, expression_AST_ptr _expression, location_span _loc)
{
    //type_of_statement = statement_AST_node::definition_t;
    LHS = _LHS;
    loc = _loc;
    expression = _expression;
}

void assignment_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->assignmentStmt_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );
    AST_visitor_base* LHS_child = visitor->get_child(1) ;
    LHS->apply_visitor( LHS_child );
    AST_visitor_base* exp_child = visitor->get_child(0) ;
    expression->apply_visitor( exp_child );

    visitor->assignmentStmt_up( this, LHS_child, exp_child );
    visitor->statement_up( this );
    visitor->ASTnode_up( this );
}

// auto definition
auto_definition_statement_AST_node::auto_definition_statement_AST_node(utf8_string& _var_name, expression_AST_ptr _expression, location_span _loc)
{
    var_name = _var_name;
    expression = _expression;
    loc = _loc;
}

void auto_definition_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->autoDefStmt_down( this );
    visitor->generic_VarDef_down(this);
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0) ;
    expression->apply_visitor( child );

    visitor->autoDefStmt_up( this, child );
    visitor->generic_VarDef_up(this);
    visitor->statement_up( this );
    visitor->ASTnode_up( this );
}


// return statement
return_statement_AST_node::return_statement_AST_node(expression_AST_ptr _expression, location_span _loc)
{
    expression = _expression;
    loc = _loc;
    callable_to_escape = nullptr;
}

void return_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->returnStatement_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* child = visitor->get_child(0) ;
    expression->apply_visitor( child );

    visitor->returnStatement_up( this, child );
    visitor->statement_up( this );
    visitor->ASTnode_up( this );

}

// loop control
loopCntrl_statement_AST_node::loopCntrl_statement_AST_node(cntrl_type _type, int _depth, csu::location_span _loc)
{
    loc = _loc;
    depth = _depth;
    type = _type;
}

void loopCntrl_statement_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->loopCntrlStatement_down( this );
    visitor->statement_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 0 );

    visitor->loopCntrlStatement_up( this );
    visitor->statement_up( this );
    visitor->ASTnode_up( this );
}





/// LHS references ///
// base
LHS_reference_AST_node::LHS_reference_AST_node()
{
    level = -1;
    reference_type = nullptr;
}

// var ref
LHS_varReference::LHS_varReference(utf8_string& _name, location_span _loc )
{
    var_name = _name;
    loc = _loc;
    variable_symbol = nullptr;
}

void LHS_varReference::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->LHS_varRef_down( this );
    visitor->LHSReference_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->LHS_varRef_up( this );
    visitor->LHSReference_up( this );
    visitor->ASTnode_up( this );
}

//member access
LHS_accessor_AST_node::LHS_accessor_AST_node(LHSref_AST_ptr _LHS_exp, utf8_string _name, location_span _loc)
{
    loc = _loc;
    name = _name;
    LHS_exp = _LHS_exp;
}

void LHS_accessor_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->LHS_accessor_down( this );
    visitor->LHSReference_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* EXP_child = visitor->get_child(0);
    LHS_exp->apply_visitor( EXP_child );

    visitor->LHS_accessor_up( this, EXP_child );
    visitor->LHSReference_up( this );
    visitor->ASTnode_up( this );
}







///  expressions ///

expression_AST_node::expression_AST_node()
{
    expression_return_type = nullptr;
    //has_output_ownership = true;

    //writer = nullptr;
}

// int literal
intLiteral_expression_AST_node::intLiteral_expression_AST_node(utf8_string _literal, location_span _loc)
{
    //type_of_expression = expression_AST_node::int_literal_t;
    literal = _literal;
    loc = _loc;
}

void intLiteral_expression_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->intLiteral_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->intLiteral_up( this );
    visitor->expression_up( this );
    visitor->ASTnode_up( this );

}

// binary operator
binOperator_expression_AST_node::binOperator_expression_AST_node(expression_AST_ptr _left_operand, expression_type _type, expression_AST_ptr _right_operand)
{
    mode_of_operation = binOperator_expression_AST_node::LHS_m;
    left_operand = _left_operand;
    type_of_operation = _type;
    right_operand = _right_operand;
    loc = left_operand->loc + right_operand->loc;
}

void binOperator_expression_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->binOperator_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );
    AST_visitor_base* LHS_child = visitor->get_child(0) ;
    left_operand->apply_visitor( LHS_child );
    AST_visitor_base* RHS_child = visitor->get_child(1) ;
    right_operand->apply_visitor( RHS_child );

    visitor->binOperator_up( this, LHS_child, RHS_child );
    visitor->expression_up( this );
    visitor->ASTnode_up( this );

}

//variable name reference
varReferance_expression_AST_node::varReferance_expression_AST_node(csu::utf8_string _var_name, csu::location_span _loc)
{
    var_name = _var_name;
    variable_symbol = nullptr;
    loc = _loc;
}

void varReferance_expression_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->varReferance_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->varReferance_up( this );
    visitor->expression_up( this );
    visitor->ASTnode_up( this );
}

// member accessor expression
accessor_expression_AST_node::accessor_expression_AST_node(expression_AST_ptr _expression, utf8_string _name, location_span _loc)
{
    expression = _expression;
    name = _name;
    loc = _loc;
}

void accessor_expression_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->accessorExp_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* EXP_child = visitor->get_child(0);
    expression->apply_visitor( EXP_child );

    visitor->accessorExp_up( this, EXP_child );
    visitor->expression_up( this );
    visitor->ASTnode_up( this );
}

// parenthesis expression grouping
ParenGrouped_expression_AST_node::ParenGrouped_expression_AST_node(expression_AST_ptr _expression, csu::location_span _loc)
{
    loc = _loc;
    expression = _expression;
}

void ParenGrouped_expression_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->ParenExpGrouping_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 1 );
    AST_visitor_base* EXP_child = visitor->get_child(0);
    expression->apply_visitor( EXP_child );

    visitor->ParenExpGrouping_up( this, EXP_child );
    visitor->expression_up( this );
    visitor->ASTnode_up( this );
}

// function call
functionCall_expression_AST_node::functionCall_expression_AST_node( expression_AST_ptr _expression, argumentList_AST_ptr _argument_list, location_span _loc)
{
    loc = _loc;
    expression = _expression;
    argument_list = _argument_list;
}

void functionCall_expression_AST_node::apply_visitor_inner(AST_visitor_base* visitor)
{
    visitor->functionCall_Exp_down( this );
    visitor->expression_down( this );
    visitor->ASTnode_down( this );

    if( not visitor->apply_to_children() ){ return; }

    visitor->initiate_children( 2 );
    AST_visitor_base* exp_child = visitor->get_child(0) ;
    expression->apply_visitor( exp_child );
    AST_visitor_base* args_child = visitor->get_child(1) ;
    argument_list->apply_visitor( args_child );

    visitor->functionCall_Exp_up( this, exp_child, args_child );
    visitor->expression_up( this );
    visitor->ASTnode_up( this );
}





