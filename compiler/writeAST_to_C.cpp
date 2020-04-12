
#include "writeAST_to_C.hpp"
#include "AST_visitor.hpp"
#include <random>
#include <sstream>
#include <chrono>

using namespace csu;
using namespace std;


////  HEADER WRITERS ////
// writes import statements into header
class header_writeImports_visitor : public AST_visitorTree
{
public:
    bool do_children; // set on way down to true to do_children, otherwise will be false;
    ofstream& header_fout;
    string header_fout_name; // is this REALLY needed?

    header_writeImports_visitor(ofstream& _header_fout, string _header_fout_name) :
        header_fout( _header_fout )
    {
       // header_fout = _header_fout;
        header_fout_name = _header_fout_name;
        do_children = true;
    }

    header_writeImports_visitor(bool _do_children, ofstream& _header_fout, string _header_fout_name) :
        header_fout( _header_fout )
    {
        header_fout_name = _header_fout_name;
        do_children = _do_children;
    }

    bool apply_to_children() override
        { return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<header_writeImports_visitor>( false, header_fout, header_fout_name ); // if we are here, then we are on module.
        // The modules children should not apply the visitor, as only top-level statements can be imports
    }

    void module_down(module_AST_node* module) override
    {
        auto seed = chrono::system_clock::now().time_since_epoch().count();
        default_random_engine generator(seed);
        uniform_int_distribution<int> distribution(10000000, 99999999);

        string h_gaurd_ID =  to_string( distribution(generator) );
        header_fout << "#ifndef " << module->module_name << "_" << h_gaurd_ID << endl;
        header_fout << "#define " << module->module_name << "_" << h_gaurd_ID << endl;
    }

    void cImports_down(import_C_AST_node* ASTnode) override
    {
        if( ASTnode->file_name.get_length() != 0 )
        {
            header_fout << "#include \"" << ASTnode->file_name << "\""<<endl;
        }
    }

    void ClassDef_down( class_AST_node* class_node)
    {
        //do_children = true; // so we can handle the structures and methods inside a structure
        // ??

        header_fout << "struct "<< class_node->type_ptr->C_name << ";" << endl;
    }

};


// for writing the members inside classes
// mostly for variables. doesn't handle methods or other classes ...
class ClassInternal_visitor : public AST_visitorTree
{
public:
    ofstream& fout;
    bool do_children;

    ClassInternal_visitor(ofstream& _fout, bool chillins=true) :
        fout( _fout )
    {
        do_children = chillins;
    }

    bool apply_to_children() override
            { return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<ClassInternal_visitor>( fout, false );
    }

    void ClassVarDef_down( class_varDefinition_AST_node* class_var_def )
    {
        auto type = class_var_def->var_type->resolved_type;
        auto name = class_var_def->variable_symbol->C_name;

        type->C_definition_name(name, fout);
        fout<<';'<<endl;
    }

};


// writes definitions of functions, variables, and structs into header
// only visits top AST nodes... with exceptions (mainly methods)...
class header_writeDefinitions_visitor : public AST_visitorTree
{
public:
    bool do_children; // set this to true on way down to do children. Set to false initially (except on module..)
    ofstream& header_fout;
    string header_fout_name;

    header_writeDefinitions_visitor(ofstream& _header_fout, string _header_fout_name) :
        header_fout( _header_fout )
    {
        header_fout_name = _header_fout_name;
        do_children = true;
    }

    header_writeDefinitions_visitor(bool _do_children, ofstream& _header_fout, string _header_fout_name) :
        header_fout( _header_fout )
    {
        header_fout_name = _header_fout_name;
        do_children = _do_children;
    }

    bool apply_to_children() override
            { return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<header_writeDefinitions_visitor>( false, header_fout, header_fout_name ); // if we are here, then we are on module.
        // The modules children should not apply the visitor, as only top-level statements can be have definitions
    }

    void funcDef_down(function_AST_node* funcDef) override
    {
        funcDef->specific_overload->write_C_prototype( header_fout );
        header_fout << ';'<<endl;
    }

    void definitionStmt_down(definition_statement_AST_node* defStmt) override
    {
        header_fout << "extern ";
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, header_fout );
        header_fout << ";" << endl;
    }

    void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt) override
    {
        header_fout << "extern ";
        autoStmt->variable_symbol->var_type->C_definition_name( autoStmt->variable_symbol->C_name, header_fout );
        header_fout << ";" << endl;
    }

    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children) override
    {
        header_fout << "void " << module->module_name << "__init__(void);"<<endl;
        header_fout << "#endif"  <<  endl;
    }


    /// CLASSES ///
    void ClassDef_down( class_AST_node* class_node)
    {
        do_children = true; // so we can handle the structures and methods inside a structure

        header_fout << "struct "<< class_node->type_ptr->C_name << "{" << endl;

        ClassInternal_visitor internal_visitor(header_fout);
        class_node->apply_visitor( &internal_visitor );

        // define bits in struct
        header_fout << endl;
        header_fout << "};" << endl;
    }

    void methodDef_down(method_AST_node* methodDef) override
    {
        methodDef->specific_overload->write_C_prototype( header_fout );
        header_fout << ';'<<endl;
    }

};



//// SOURCE WRITERS ////
// writes the preamble. This includes import of header, simple functions, global variables, etc...
class source_writePreamble_visitor : public AST_visitorTree
{
public:
    bool do_children;
    ofstream& source_fout;
    string header_fname; // only defined for module

    source_writePreamble_visitor(ofstream& _source_fout, string _header_fname) :
        source_fout(_source_fout)
    {
        header_fname = _header_fname;
        do_children = true;
    }

    source_writePreamble_visitor(bool _do_children, ofstream& _source_fout) :
        source_fout( _source_fout )
    {
        do_children = _do_children;
    }

    bool apply_to_children() override
        { return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<source_writePreamble_visitor>( false, source_fout ); // if we are here, then we are on module.
        // The modules children should not apply the visitor, so far, only top-level statements are need for preamble!
    }


    void module_down(module_AST_node* module) override
    {
        source_fout << "#include \"" << header_fname << "\"" << endl;
        source_fout << endl;
/*
        // these should be included from a library.
        source_fout << "#define __Ctype_add_macro__(X, Y) \\" << endl;
        source_fout << "({ typeof (X) __cy_TMP_x__ = (X); \\" << endl;
        source_fout << " typeof (Y) __cy_TMP_y__ = (Y); \\" << endl;
        source_fout << " __cy_TMP_x__ + __cy_TMP_y__; })" << endl;
        source_fout << endl;

        source_fout << "#define __Ctype_assignment_macro__(X, Y) \\" << endl;
        source_fout << "({ typeof (Y) __cy_TMP_y__ = (Y); \\" << endl;
        source_fout << "*X = __cy_TMP_y__; })" << endl;
        source_fout << endl;*/

    }

    void definitionStmt_down(definition_statement_AST_node* defStmt) override
    {
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name,  source_fout  );
        source_fout<< ";" << endl << endl;
    }

    void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt) override
    {
        autoStmt->variable_symbol->var_type->C_definition_name( autoStmt->variable_symbol->C_name, source_fout );
        source_fout << ";" << endl << endl;
    }
};

//Next three visitors are the work horses. they write out the LHS_references, expressions and statements

// this is for writing the LHS of an assignment
class source_LHSreference_visitor : public AST_visitor_NoChildren
{
    // this is called solely by assignment at the moment.
    // it has levels, with top level is special. Top level acts like a setter, and thus needs the C-code and type info of the RHS of the assignment
    // the lower levels act like getters, and just build the appropriate LHS C-code.
    // This main class represents only the upper level. it has a child type that represents the lower levels.
    // Note that this upper level MAY call the lower level on itself or on children

private:

    class source_LHS_child : public AST_visitorTree
    {
    public:
        ofstream& source_fout;
        utf8_string LHS_C_code; // visitor assigns to this

        source_LHS_child(ofstream& _source_fout) :
            source_fout(_source_fout)
            {
            }

        shared_ptr< AST_visitor_base > make_child(int number) override
        {
            return make_shared<source_LHS_child>( source_fout );
        }

        void LHS_varRef_up(LHS_varReference* varref) override
        {
            LHS_C_code = varref->variable_symbol->C_name; // assume its this simple for now. Hope it stays this way
        }

        void LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor) override
        {
            auto LHS_child = dynamic_cast<source_LHS_child*>( LHSref_visitor );
            utf8_string& child_LHS_Cref = LHS_child->LHS_C_code;

            LHS_C_code = LHSaccess->reference_type->write_member_getref( child_LHS_Cref, LHSaccess->name, source_fout );
        }
    };


    ofstream& source_fout;
    utf8_string RHS_C_code; // c-code that defines the RHS
    varType_ptr RHS_type;

public:

    source_LHSreference_visitor(ofstream& _source_fout, utf8_string& RHS_exp, varType_ptr _RHS_type) :
        source_fout(_source_fout)
    {
        RHS_C_code = RHS_exp;
        RHS_type = _RHS_type;
    }

    // note this will NEVER call 'up' visitors!

    void LHS_varRef_down(LHS_varReference* varref) override
    {
        // note we are still assuming that var refs are always simple. This may change.
        source_LHS_child LHS( source_fout );
        varref->apply_visitor( &LHS );

        varref->reference_type->write_assignment( RHS_type.get(), LHS.LHS_C_code , RHS_C_code, source_fout );
    }

    void LHS_accessor_down(LHS_accessor_AST_node* LHSaccess) override
    {
        source_LHS_child LHS( source_fout );
        LHSaccess->LHS_exp->apply_visitor( &LHS );

        // note we need the type of the child, not the final return type!!!
        auto child_type = LHSaccess->LHS_exp->reference_type;

        child_type->write_member_setter( LHS.LHS_C_code, LHSaccess->name, RHS_type.get(), RHS_C_code, source_fout );
    }
};

class source_expression_visitor : public AST_visitorTree
{
/// write out straight-forward expressions. does not write functions, classes, or those complex thingies.
/// this should ONLY be called on expressions.

public:
    ofstream& source_fout;
    utf8_string C_expression_code; // C expression code that returns the value of the cyth expression.
    //Note that each cyth expresion may require multiple C statements. Thus each cyth expression may write to the file, and to expression_code
    // because 'expression_code' contains code, it MUST be used by its parent

    source_expression_visitor(ofstream& _source_fout) :
        source_fout(_source_fout)
    {
    }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<source_expression_visitor>( source_fout );
    }

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp) override
    {
        stringstream exp;
        exp << intLitExp->literal;
        C_expression_code = exp.str();
    }

    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor) override
    {

        // define addition
        source_expression_visitor* LHS_exp_visitor_PTR = dynamic_cast<source_expression_visitor*>(LHS_exp_visitor);
        source_expression_visitor* RHS_exp_visitor_PTR = dynamic_cast<source_expression_visitor*>(RHS_exp_visitor);

        stringstream exp;
        binOprExp->expression_return_type->write_LHSaddition( binOprExp->right_operand->expression_return_type.get(),
                            LHS_exp_visitor_PTR->C_expression_code, RHS_exp_visitor_PTR->C_expression_code, exp );
        C_expression_code = exp.str();

    }

    void varReferance_up(varReferance_expression_AST_node* varRefExp) override
    {
        C_expression_code = varRefExp->variable_symbol->C_name; // the magic of compilers!
    }

    void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor) override
    {
        source_expression_visitor* EXP_visitor_PTR = dynamic_cast<source_expression_visitor*>(expChild_visitor);
        C_expression_code = EXP_visitor_PTR->C_expression_code;
    }

    void accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor) override
    {
        auto T = accessorExp->expression->expression_return_type;
        source_expression_visitor* EXP_visitor_PTR = dynamic_cast<source_expression_visitor*>(expChild_visitor);

        C_expression_code = T->write_member_getter(EXP_visitor_PTR->C_expression_code, accessorExp->name, source_fout);
    }

    void functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child) override
    {
TODO FIX!
        /// useful variables
        auto param_to_arg_map = funcCall->function_to_write->param_to_arg_map;
        auto parameter_names = param_to_arg_map->parameters;
        int total_C_arguments = parameter_names->required_parameters.size() + 2*parameter_names->optional_parameters.size();
        int total_num_parameters = parameter_names->total_size();
        auto symbol_table = funcCall->symbol_table;



        /// first we write the argument expressions
        vector<utf8_string> param_variables;
        // k
        param_variables.reserve( total_C_arguments );
        // k
        for(int param_i = 0; param_i < total_num_parameters; param_i++ )
        {

        // k
            auto param_name = parameter_names->parameter_from_index( param_i );
        // k
            int argument_index = param_to_arg_map->param_to_arg_map[param_i];// amazing bit of naming here... solid job.

        // k
            bool param_has_default  =  param_i>=(parameter_names->required_parameters.size());
            utf8_string use_default_vname;

        // all k
            if( param_has_default )
            {
                use_default_vname = "__cy__need_default_";
// NOT OKAY, SYMTABLE.
                use_default_vname += symbol_table->get_unique_string();
                param_variables.push_back( use_default_vname );
                source_fout << "int " << use_default_vname << "=0;" << endl;
            }
            utf8_string argument_vname = "__cy__arg_";

// NOT OKAY, SYMTABLE.
            argument_vname += symbol_table->get_unique_string();
            param_variables.push_back( argument_vname );

        // k
            param_name->var_type->C_definition_name(argument_vname, source_fout);
            source_fout << ';'<<endl;

        // all k
            if( argument_index == -1 )
            {
                // make a default value.
                source_fout << use_default_vname << "=1;" << endl;
            }
            else
            {

                // call the argument expression
// NOT OKAY argument_list
                auto expr = funcCall->argument_list->expression_from_index( argument_index );
                source_expression_visitor arg_expr_visitor( source_fout );
                expr->apply_visitor( &arg_expr_visitor );

                param_name->var_type->write_assignment( expr->expression_return_type.get(), argument_vname, arg_expr_visitor.C_expression_code, source_fout );
                source_fout<<endl;
            }
        }



        /// write function body

        stringstream out_exp;
        out_exp << '(';

        source_expression_visitor* EXP_visitor_PTR = dynamic_cast<source_expression_visitor*>(expression_child);
        funcCall->function_to_write->write_call( EXP_visitor_PTR->C_expression_code, out_exp );

        /// now we write the argument names ///
        out_exp <<'(';
        bool do_comma = false;
        for(utf8_string& arg_name : param_variables)
        {
            if( do_comma )
            {
                out_exp<<',';
            }
            else
            {
                do_comma = true;
            }

            out_exp<<arg_name;
        }
        out_exp << "))" ;

        C_expression_code = out_exp.str();
    }

};

class source_statement_visitor : public AST_visitorTree
{
/// write out straight-forward statements. does not write functions, classes, or those complex thingies.
/// can write out single statements or block of statements

public:
    ofstream& source_fout;
    bool do_children;

    source_statement_visitor(ofstream& _source_fout) :
        source_fout(_source_fout)
    {
        do_children = true;
    }

    bool apply_to_children() override
        { return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<source_statement_visitor>( source_fout );
    }


    //// things that cause this visitor to stop ////
    void funcDef_down(function_AST_node* funcDef) override
    {
        do_children = false;
    }

    //// "normal" things ////
    void statement_up(statement_AST_node* statment) override
    {
        source_fout <<  endl;
    }

    void expressionStatement_down(expression_statement_AST_node* expStmt) override
    {
        source_expression_visitor expr_vistr( source_fout );
        expStmt->expression->apply_visitor( &expr_vistr );

        // write exp
        source_fout << expr_vistr.C_expression_code << ";" << endl;
    }


    void definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child) override
    {
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, source_fout );
        source_fout << ";" << endl << endl;
    }

    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child) override
    {
        source_expression_visitor expr_vistr( source_fout );
        assignStmt->expression->apply_visitor( &expr_vistr );

        source_LHSreference_visitor LHS_vistr( source_fout, expr_vistr.C_expression_code, assignStmt->expression->expression_return_type);
        assignStmt->LHS->apply_visitor( &LHS_vistr );


        //assignStmt->LHS->reference_type->write_assignment( assignStmt->expression->expression_return_type.get(),
                                           //     LHS_vistr.C_LHS_ref_code, expr_vistr.C_expression_code, source_fout );
        source_fout << endl;
    }

    void autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child) override
    {
        stringstream exp;
        autoStmt->variable_symbol->var_type->C_definition_name( autoStmt->variable_symbol->C_name, exp );

        source_expression_visitor expr_vistr( source_fout );
        autoStmt->expression->apply_visitor( &expr_vistr );

        utf8_string LHS = exp.str();
        autoStmt->variable_symbol->var_type->write_assignment( autoStmt->expression->expression_return_type.get(),
                                                LHS, expr_vistr.C_expression_code, source_fout );
        source_fout << endl;
    }

    void returnStatement_down(return_statement_AST_node* returnStmt) override
    {
        source_expression_visitor expr_vistr( source_fout );
        returnStmt->expression->apply_visitor( &expr_vistr );

        // we need to do some casting here?

        // write return
        source_fout << "return " << expr_vistr.C_expression_code << ";" << endl;
    }

};

// writes the module setup function
class source_moduleExpresion_visitor : public AST_visitorTree
{
// essentially skims top-level AST for simple expressions, and writes them to the setup function.
// uses source_statement_visitor to actually do the writing.
// does not write functions, classes, or complex thingies

public:
    ofstream& source_fout;
    bool do_children; //true for module (first node applied), false for next level. Lower levels not done
    bool do_writing; // default is true. Set on way down to false if nesisary

    source_moduleExpresion_visitor(ofstream& _source_fout) :
        source_fout(_source_fout)
    {
        do_children = true;
        do_writing = true;
    }

    source_moduleExpresion_visitor(bool _do_children, ofstream& _source_fout) :
        source_fout(_source_fout)
    {
        do_children = _do_children;
        do_writing = true;
    }

    bool apply_to_children() override
        { return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<source_moduleExpresion_visitor>( false, source_fout );
    }


    void module_down(module_AST_node* module) override
    {
        source_fout << "void " << module->module_name << "__init__(void){" << endl;
    }

    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children) override
    {
        source_fout << "}" << endl;
    }

    void definitionStmt_down(definition_statement_AST_node* defStmt) override
    {
        do_writing = false; // DO NOT WRITE DEFS!
    }

    void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt) override
    {
        source_expression_visitor expr_vistr( source_fout );
        autoStmt->expression->apply_visitor( &expr_vistr );

        autoStmt->variable_symbol->var_type->write_assignment( autoStmt->expression->expression_return_type.get(),
                            autoStmt->variable_symbol->C_name, expr_vistr.C_expression_code, source_fout );
        source_fout << endl;

        do_writing = false; // 'cause its weird!!
    }

    void statement_down(statement_AST_node* statment) override
    {
        if(do_writing)
        {
            source_statement_visitor statement_writer( source_fout );
            statment->apply_visitor( &statement_writer );
        }
    }
};

class source_function_visitor : public AST_visitorTree
{
// looks for functions and methods! and writes their source using the above expression writer.
// probably needs to be VERY recursive due to nested functions

public:
    ofstream& source_fout;

    source_function_visitor(ofstream& _source_fout) :
        source_fout(_source_fout)
    {
    }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<source_function_visitor>( source_fout );
    }

    void funcDef_up(function_AST_node* funcDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child, AST_visitor_base* funcBody_child) override
    {
        funcDef->specific_overload->write_C_prototype( source_fout );
        source_fout << endl;
        source_fout << "{" << endl;

        /// write stuff for default params here!!!
        if( funcDef->paramList->defaulted_list )
        {
            auto default_params = funcDef->paramList->defaulted_list;
            int num_params = default_params->param_list.size();

            auto param_name_iter = default_params->param_list.begin();
            auto default_exp_iter =  default_params->parameter_defaults.begin();
            for( int i=0; i<num_params; i++ )
            {
                source_fout << "if("<< param_name_iter->variable_symbol->definition_name<<"__use_default__"<<"){"<<endl;

                source_expression_visitor expr_vistr( source_fout );
                (*default_exp_iter)->apply_visitor( &expr_vistr );

                param_name_iter->var_type_ASTnode->resolved_type->write_assignment( (*default_exp_iter)->expression_return_type.get(),
                            param_name_iter->variable_symbol->C_name, expr_vistr.C_expression_code, source_fout );

                source_fout << "}"<<endl;

            }
        }

        // write block of statements
        source_statement_visitor statement_writer( source_fout );
        funcDef->block_AST->apply_visitor( &statement_writer );

        source_fout << "}" << endl << endl;
    }

    void methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child)
    {
        methodDef->specific_overload->write_C_prototype( source_fout );
        source_fout << endl;
        source_fout << "{" << endl;

        /// write stuff for default params here!!!
        if( methodDef->paramList->defaulted_list )
        {
            auto default_params = methodDef->paramList->defaulted_list;
            int num_params = default_params->param_list.size();

            auto param_name_iter = default_params->param_list.begin();
            auto default_exp_iter =  default_params->parameter_defaults.begin();
            for( int i=0; i<num_params; i++ )
            {
                source_fout << "if("<< param_name_iter->variable_symbol->definition_name<<"__use_default__"<<"){"<<endl;

                source_expression_visitor expr_vistr( source_fout );
                (*default_exp_iter)->apply_visitor( &expr_vistr );

                param_name_iter->var_type_ASTnode->resolved_type->write_assignment( (*default_exp_iter)->expression_return_type.get(),
                            param_name_iter->variable_symbol->C_name, expr_vistr.C_expression_code, source_fout );

                source_fout << "}"<<endl;

            }
        }

        // write block of statements
        source_statement_visitor statement_writer( source_fout );
        methodDef->block_AST->apply_visitor( &statement_writer );

        source_fout << "}" << endl << endl;
    }

};

//// the main function ////

void write_module_to_C(module_AST_ptr module, string fout_name)
{
    //// first we write the header ///
    string header_fname = fout_name + ".h";
    ofstream C_header_file(header_fname);

    // imports //
    header_writeImports_visitor import_writer(C_header_file, header_fname);
    module->apply_visitor( &import_writer );

    // definitions //
    header_writeDefinitions_visitor definitions_writer(C_header_file, header_fname);
    module->apply_visitor( &definitions_writer );



    //// now we write the source files ////
    string source_fname = fout_name + ".c";
    ofstream C_source_file(source_fname);

    // preamble, anything that needs to come before main body
    source_writePreamble_visitor preamble_writer(C_source_file, header_fname);
    module->apply_visitor( &preamble_writer );

    // main body

    // functions!
    source_function_visitor function_writer(C_source_file);
    module->apply_visitor( &function_writer );


    // module setup (top level expressions, call other module setups)
    source_moduleExpresion_visitor initilizer_writer(C_source_file);
    module->apply_visitor( &initilizer_writer );

    // write main function?
    if( module->main_status == 1 )
    {
        string source_main_fname = fout_name + "_main.c";
        ofstream C_source_main_file(source_main_fname);

        C_source_main_file << "#include \"" << header_fname << "\"" <<endl;
        C_source_main_file << "void main(void){" <<endl;
        // write prep functions for all other modules here!!
        C_source_main_file << module->module_name << "__init__();" << endl;
        C_source_main_file << module->main_func_name << "();" << endl;
        C_source_main_file << '}' << endl;
    }
}
