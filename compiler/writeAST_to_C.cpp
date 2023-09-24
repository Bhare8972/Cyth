
#include <random>
#include <chrono>
#include <algorithm>


#include "writeAST_to_C.hpp"
#include "AST_visitor.hpp"
#include "module_manager.hpp"

using namespace csu;
using namespace std;


////  HEADER WRITERS ////
// writes import statements into header
class header_writeImports_visitor : public AST_visitorTree
{
public:
    bool do_children; // set on way down to true to do_children, otherwise will be false;
    Csource_out_ptr header_fout;
    string header_fout_name; // is this REALLY needed?

    header_writeImports_visitor(Csource_out_ptr _header_fout, string _header_fout_name) :
        header_fout( _header_fout )
    {
       // header_fout = _header_fout;
        header_fout_name = _header_fout_name;
        do_children = true;
    }

    header_writeImports_visitor(bool _do_children, Csource_out_ptr _header_fout, string _header_fout_name) :
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
        header_fout->out_strm() << "#ifndef " << module->module_name << "_" << h_gaurd_ID << endl;
        header_fout->out_strm() << "#define " << module->module_name << "_" << h_gaurd_ID << endl;
    }

    void cImports_down(import_C_AST_node* ASTnode) override
    {
        if( ASTnode->found_file_location.length() != 0 )
        {
            header_fout->out_strm() << "#include \"" << ASTnode->found_file_location << "\""<<endl;
        }
    }


    virtual void CythImports_down(import_cyth_AST_node* ASTnode) override
    {
        if( ASTnode->import_module_Cheader_fname.get_length() != 0 )
        {
            header_fout->out_strm() << "#include \"" << ASTnode->import_module_Cheader_fname << "\""<<endl;
        }
    }

    void ClassDef_down( class_AST_node* class_node)
    {
        //do_children = true; // so we can handle the structures and methods inside a structure
        // ??

        header_fout->out_strm() << "struct " << class_node->type_ptr->C_name << ";" << endl;
    }

};


// for defining the data members inside classes
// mostly for variables. doesn't handle methods or other classes ...
class Class_DataDefiner : public AST_visitorTree
{
public:
    Csource_out_ptr fout;
    bool do_children;

    Class_DataDefiner(Csource_out_ptr _fout, bool chillins=true) :
        fout( _fout )
    {
        do_children = chillins;
    }

    bool apply_to_children() override
            { return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<Class_DataDefiner>( fout, false );
    }

    void ClassVarDef_down( class_varDefinition_AST_node* class_var_def ) override
    {
        auto type = class_var_def->var_type->resolved_type;
        auto name = class_var_def->variable_symbol->C_name;

        fout->out_strm() << fout->ln_strt ;
        type->C_definition_name(name, fout);
        fout->out_strm() << ';' << endl;
    }

};

// writes definitions of functions, variables, and structs into header
// only visits top AST nodes... with exceptions (mainly methods)...
class header_writeDefinitions_visitor : public AST_visitorTree
{
public:
    bool do_children; // set this to true on way down to do children. Set to false initially (except on module..)
    Csource_out_ptr header_fout;
    string header_fout_name;

    header_writeDefinitions_visitor(Csource_out_ptr _header_fout, string _header_fout_name) :
        header_fout( _header_fout )
    {
        header_fout_name = _header_fout_name;
        do_children = true;
    }

    header_writeDefinitions_visitor(bool _do_children, Csource_out_ptr _header_fout, string _header_fout_name) :
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
        header_fout->out_strm() << ';'<<endl;
    }

    void definitionStmt_down(definition_statement_AST_node* defStmt) override
    {
        header_fout->out_strm() << "extern ";
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, header_fout );
        header_fout->out_strm() << ";" << endl;
    }

    void definitionNconstructionStmt_down(definitionNconstruction_statement_AST_node* defStmt) override
    {
        header_fout->out_strm() << "extern ";
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, header_fout );
        header_fout->out_strm() << ";" << endl;
    }

    void definitionNassignmentStmt_down(definitionNassignment_statement_AST_node* defStmt) override
    {
        header_fout->out_strm() << "extern ";
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, header_fout );
        header_fout->out_strm() << ";" << endl;
    }

    void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt) override
    {
        header_fout->out_strm() << "extern ";
        autoStmt->variable_symbol->var_type->C_definition_name( autoStmt->variable_symbol->C_name, header_fout );
        header_fout->out_strm() << ";" << endl;
    }

    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children) override
    {
        header_fout->out_strm() << "void " << module->module_name << "__init__(void);"<<endl;
        header_fout->out_strm() << "#endif"  <<  endl;
    }


    /// CLASSES ///
    void ClassDef_down( class_AST_node* class_node) override
    {
        do_children = true; // so we can handle the structures and methods inside a structure
        // anouther, more correct approach, would be to use anouther visiter
        auto class_type = class_node->type_ptr;
        //auto class_symbols = class_type->class_symbol_table;


    // first we write ancilary structs/classes //
        // starting with the vtable //
        header_fout->out_strm() << "struct "<< class_type->vtableType_cname << "{" << endl;
        header_fout->enter_scope();

        // Write all virtual methods that don't override and are defined in this class
        DefClassType::methodOverloadIter methoditer = class_type->methodOverloadIter_begin( false );
        DefClassType::methodOverloadIter methoditer_end = class_type->methodOverloadIter_end();
        for( ; methoditer!=methoditer_end; ++methoditer )
        {

            auto overload = methoditer.overload_get();

            if( overload->is_virtual and overload->overriden_method==nullptr )// virtual and does not overload
            {
                // first, define the offset
                header_fout->out_strm() << header_fout->ln_strt << "long " << overload->c_reference << "_offset;" << endl;

                // then the function pointer
                utf8_string TMP("");
                header_fout->out_strm() << header_fout->ln_strt;
                overload->return_type->C_definition_name(TMP, header_fout);

                //stringstream OUT;
                header_fout->out_strm() << "(*" <<  overload->c_reference << ")(";

                //overload->self_ptr_name->var_type->C_definition_name(overload->self_ptr_name->C_name, OUT);
                header_fout->out_strm() << "void* " << overload->self_ptr_name->C_name << "_";
                if( overload->parameters->total_size() > 0 )
                {
                    header_fout->out_strm() << ",";
                }

                overload->parameters->write_to_C(header_fout, false);
                header_fout->out_strm() << ")";

                //utf8_string name = OUT.str();
                header_fout->out_strm() <<';'<<endl;

            }
        }

        header_fout->out_strm() << endl;
        header_fout->leave_scope();
        header_fout->out_strm() << "};" << endl;




    // define the global vtable variable
        header_fout->out_strm() << header_fout->ln_strt << "extern struct " << class_type->vtableType_cname << " " <<  class_type->global_vtableVar_cname << ";" << endl;
    // and parent tables
        for( unsigned int parent_i=0;  parent_i< (class_type->full_inheritance_tree.size()); ++parent_i)
        {
            auto parent_type = class_type->full_inheritance_tree[ parent_i ];
            auto vtable_name = class_type->global_parentVtable_cnames[ parent_i ];
            header_fout->out_strm() << header_fout->ln_strt << "extern struct " << parent_type->vtableType_cname << " " <<  vtable_name << ";" << endl;
        }




    // now we define the actual class struct

        header_fout->out_strm() << header_fout->ln_strt << "struct "<< class_type->C_name << "{" << endl;
        header_fout->enter_scope();

        //write vtable
        header_fout->out_strm() << "struct " << class_type->vtableType_cname << " *__cy_vtable;" << endl;

        // write parents
        for( auto &parent_name : class_type->parent_class_names)
        {
            header_fout->out_strm() << header_fout->ln_strt ;
            parent_name->var_type->C_definition_name(parent_name->C_name, header_fout);
            header_fout->out_strm() << ';' << endl;
        }

        //write members
        Class_DataDefiner internal_visitor(header_fout);
        class_node->apply_visitor( &internal_visitor );


        header_fout->out_strm() << endl;
        header_fout->leave_scope();
        header_fout->out_strm() << header_fout->ln_strt << "};" << endl;





    // write automated functions
        // write default constructor?
        if(class_node->write_default_constructor)
        {
            header_fout->out_strm() << header_fout->ln_strt ;
            class_node->default_constructor_overload->write_C_prototype( header_fout );
            header_fout->out_strm() << ';'<<endl;
        }
        // write default destructor?
        if(class_node->write_default_deconstructor)
        {
            header_fout->out_strm() << header_fout->ln_strt ;
            class_node->default_destructor_overload->write_C_prototype( header_fout );
            header_fout->out_strm() << ';'<<endl;
        }
        // default copy constructor?
        if( class_node->write_selfCopy_constructor )
        {
            header_fout->out_strm() << header_fout->ln_strt ;
            class_node->default_CopyConstructor_overload->write_C_prototype( header_fout );
            header_fout->out_strm() << ';'<<endl;
        }
        // defaulted assignments!
        for( auto &overload_var_pair : class_node->assignments_to_default)
        {
            header_fout->out_strm() << header_fout->ln_strt ;
            overload_var_pair.first->write_C_prototype( header_fout );
            header_fout->out_strm() << ';'<<endl;
        }

    }

    void methodDef_down(method_AST_node* methodDef) override
    {
        header_fout->out_strm() << header_fout->ln_strt ;
        methodDef->specific_overload->write_C_prototype( header_fout );
        header_fout->out_strm() << ';'<<endl;
    }

};



//// SOURCE WRITERS ////
// writes the preamble. This includes import of header, simple functions, global variables, etc...
// initializes the vtables
class source_writePreamble_visitor : public AST_visitorTree
{
public:
    bool do_children;
    Csource_out_ptr source_fout;
    string header_fname; // only defined for module

    source_writePreamble_visitor(Csource_out_ptr _source_fout, string _header_fname) :
        source_fout(_source_fout)
    {
        header_fname = _header_fname;
        do_children = true;
    }

    source_writePreamble_visitor(bool _do_children, Csource_out_ptr _source_fout) :
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
        source_fout->out_strm() << "#include \"" << header_fname << "\"" << endl;
        source_fout->out_strm() << endl;
    }

    void typed_VarDef_down(Typed_VarDefinition* vardef) override // hope this works??
    {
        vardef->var_type->resolved_type->C_definition_name( vardef->variable_symbol->C_name,  source_fout  );
        source_fout->out_strm() << ";" << endl << endl;
    }

    void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt) override
    {
        autoStmt->variable_symbol->var_type->C_definition_name( autoStmt->variable_symbol->C_name, source_fout );
        source_fout->out_strm() << ";" << endl << endl;
    }

    void ClassDef_down( class_AST_node* class_node) override
    {

        // we defined the vtables!
        auto class_type = class_node->type_ptr;
        auto class_symbols = class_type->class_symbol_table;

        //we define the symbol table for each parent class, and work our way down.
        // loop over each parent
        for( int parent_class_index=0; parent_class_index<class_type->full_inheritance_tree.size();  ++parent_class_index )
        {
            auto &parent_class_type = class_type->full_inheritance_tree[ parent_class_index ];
            auto &vtable_name = class_type->global_parentVtable_cnames[ parent_class_index ];
            auto &connection_map = class_type->method_connection_table[ parent_class_index ];

            // define the struct
            source_fout->out_strm() << "struct "<< parent_class_type->vtableType_cname << " " << vtable_name << " = {" << endl;
            source_fout->enter_scope();

            // now we loop over every vtabled method and its override
            for( auto &connection_pair : connection_map )
            {
                auto &parent_method = connection_pair.second.methodOverload_to_override;
                auto &overriding_method = connection_pair.second.methodOverload_that_overrides;
                auto &overriding_class = connection_pair.second.class_that_overrides;




                //source_fout->out_strm() << source_fout->ln_strt << " ." << parent_method->c_reference << "_offset =" << offset << "," <<endl;

                source_fout->out_strm() << source_fout->ln_strt << " ." << parent_method->c_reference << "_offset = 0";

                // there are three options for the offset
                int override_to_parent_index = overriding_class->get_parent_index( parent_class_type );
                if( parent_method==overriding_method ) // never got overriden
                {
                    //source_fout << " ." << parent_method->c_reference << "_offset =0,"<< endl;
                    // offset is zero, nothing to do
                }
                else if( override_to_parent_index >= 0 ) // there is a simple relationship between the overriding class and the parent
                {
                    // need to find the byte-offset between the parent and overriding_class
                    auto parental_iterator = overriding_class->parentIter_begin( override_to_parent_index );
                    auto parental_iterator_end = overriding_class->parentIter_end( );

                    auto previous_parent = parental_iterator.get();
                    ++parental_iterator;
                    for( ; parental_iterator!=parental_iterator_end; ++parental_iterator )
                    {
                        auto child_class = parental_iterator.get();

                        auto parent_varname = child_class->parent_class_names[ child_class->get_immediate_parent_index(previous_parent) ];
                        //utf8_string TMP;
                        //stringstream defClass_out;
                        //child_class->C_definition_name(TMP, defClass_out);
                        //offset = offset + "+offsetof(" + defClass_out.str() + ", " + parent_varname->C_name + ")";

                        source_fout->out_strm() << "+offsetof(";
                        utf8_string TMP("");
                        child_class->C_definition_name(TMP, source_fout);
                        source_fout->out_strm() << ", " << parent_varname->C_name << ")";

                        previous_parent = child_class;
                    }

                    //source_fout << " ." << parent_method->c_reference << "_offset ="<< current_calculation << "," <<endl;

                }
                else // things are...... complicated
                {
// is this correct?? how to check?
                    // first we find the byte offset between this class and the overrideing class
                    int this_to_override_index =  class_type->get_parent_index( overriding_class );
                    auto parentalOverride_iterator = class_type->parentIter_begin( this_to_override_index );
                    auto parentalOverride_iterator_end = class_type->parentIter_begin( this_to_override_index );

                    auto previous_parent = parentalOverride_iterator.get();
                    ++parentalOverride_iterator;
                    for( ; parentalOverride_iterator!=parentalOverride_iterator_end; ++parentalOverride_iterator )
                    {
                        auto child_class = parentalOverride_iterator.get();

                        //offset = offset + "+offsetof(" + child_class->C_name + ", " + previous_parent->C_name + ")";
                        source_fout->out_strm() << "+offsetof(" << child_class->C_name << ", " << previous_parent->C_name << ")";

                        previous_parent = child_class;
                    }

                    // then we subtract off the offset to the parent class
                    int this_to_parent_index =  class_type->get_parent_index( parent_class_type );
                    auto parentalParent_iterator = class_type->parentIter_begin( this_to_parent_index );
                    auto parentalParent_iterator_end = class_type->parentIter_begin( this_to_parent_index );

                    previous_parent = parentalParent_iterator.get();
                    ++parentalParent_iterator;
                    for( ; parentalParent_iterator!=parentalParent_iterator_end; ++parentalParent_iterator )
                    {
                        auto child_class = parentalParent_iterator.get();

                        //offset = offset + "-offsetof(" + child_class->C_name + ", " + previous_parent->C_name + ")";
                        source_fout->out_strm() << "-offsetof(" << child_class->C_name << ", " << previous_parent->C_name << ")";

                        previous_parent = child_class;
                    }


                }
                // write to the file
                source_fout->out_strm() << "," << endl;
                source_fout->out_strm() << source_fout->ln_strt << " ." << parent_method->c_reference << " =" <<  overriding_method->c_reference << "," << endl;
            }
            source_fout->leave_scope();
            source_fout->out_strm() << "};" << endl;
        }


        // now we set our own vtable
        source_fout->out_strm() << "struct "<< class_node->type_ptr->vtableType_cname << " " << class_node->type_ptr->global_vtableVar_cname << " = {" << endl;
        source_fout->enter_scope();

        auto methoditer = class_type->methodOverloadIter_begin( false );
        auto methoditer_end = class_type->methodOverloadIter_end();
        for( ; methoditer!=methoditer_end; ++methoditer )
        {
            auto overload = methoditer.overload_get();
            if( overload->is_virtual and overload->overriden_method==nullptr) // IE, is part of this vtable
            {
                source_fout->out_strm() << source_fout->ln_strt << " ." << overload->c_reference << "_offset = 0," <<endl;
                source_fout->out_strm() << source_fout->ln_strt << " ." << overload->c_reference << " =" <<  overload->c_reference << ","<< endl;
            }
        }

        source_fout->leave_scope();
        source_fout->out_strm()  << "};" << endl;
    }
};


/// source_LHSreference_visitor ///

shared_ptr< AST_visitor_base > source_LHSreference_visitor::source_LHS_child::make_child(int number)
{
    return make_shared<source_LHS_child>( source_fout );
}

void source_LHSreference_visitor::source_LHS_child::LHS_varRef_up(LHS_varReference* varref)
{
    //LHS_C_code = varref->variable_symbol->C_name; // assume its this simple for now. Hope it stays this way
    //varref->writer = make_shared<simple_expression_writer>( varref->variable_symbol->C_name );
    final_expression = make_shared<simple_C_expression>(varref->variable_symbol->C_name, varref->variable_symbol->var_type,
                                                        true, false);
}

void source_LHSreference_visitor::source_LHS_child::LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor)
{
    //auto LHS_child = dynamic_cast<source_LHS_child*>( LHSref_visitor );
    //utf8_string& child_LHS_Cref = LHS_child->LHS_C_code;

    //LHS_C_code = LHSaccess->reference_type->write_member_getref( child_LHS_Cref, LHSaccess->name, source_fout );


    //auto child_LHS_Cref = LHSaccess->LHS_exp->writer->get_C_expression();
    //HSaccess->writer = LHSaccess->reference_type->write_member_getref( child_LHS_Cref, LHSaccess->name, source_fout );

    auto LHS_child = dynamic_cast<source_LHS_child*>( LHSref_visitor );
    auto LHS_exp = LHS_child->final_expression;
    auto LHS_type = LHS_exp->cyth_type;

    final_expression = LHS_type->write_member_getref(LHS_exp, LHSaccess->name, source_fout);

    final_expression->add_cleanup_child( LHS_exp );
}


source_LHSreference_visitor::source_LHSreference_visitor(Csource_out_ptr _source_fout, C_expression_ptr _RHS_exp) :
    RHS_exp( _RHS_exp ),
    source_fout(_source_fout)
{ }

// note this will NEVER call 'up' visitors!

void source_LHSreference_visitor::LHS_varRef_down(LHS_varReference* varref)
{
    // note we are still assuming that var refs are always simple. This may change.
    source_LHS_child LHS( source_fout );
    varref->apply_visitor( &LHS );
    auto LHS_C_code = LHS.final_expression;
    auto RHS_type = RHS_exp->cyth_type;

    if( varref->reference_type->get_has_assignment( RHS_type ))
    {
        //auto Cexp = varref->writer->get_C_expression();
        //varref->reference_type->write_assignment( RHS_type.get(), RHS_AST_node, Cexp , RHS_C_code, source_fout );
        varref->reference_type->write_assignment(LHS_C_code, RHS_exp, source_fout);
    }
    else if( RHS_type->get_has_assignTo( varref->reference_type ) )
    {
        //auto Cexp = varref->writer->get_C_expression();
        //RHS_type->write_assignTo(varref->reference_type.get(),  RHS_AST_node, Cexp , RHS_C_code, source_fout );

        RHS_type->write_assignTo( LHS_C_code, RHS_exp, source_fout );
    }
    else // trust!
    {
        throw gen_exception( "error in source_LHSreference_visitor::LHS_varRef_down. This should never be reached" );
        //auto Cexp = varref->writer->get_C_expression();
        //RHS_type->write_implicit_castTo( varref->reference_type.get(),  RHS_AST_node, Cexp , RHS_C_code, source_fout );
    }

    LHS_C_code->write_cleanup( source_fout );
}

void source_LHSreference_visitor::LHS_accessor_down(LHS_accessor_AST_node* LHSaccess)
{
    auto parent_expressionAST = LHSaccess->LHS_exp;
    auto parent_type = parent_expressionAST->reference_type;
    auto& member_name = LHSaccess->name;

    source_LHS_child parent_visitor( source_fout );
    parent_expressionAST->apply_visitor( &parent_visitor );
    auto parent_C_code = parent_visitor.final_expression;

    parent_type->write_member_setter(parent_C_code, member_name, RHS_exp, source_fout);

    parent_C_code->write_cleanup( source_fout );
}


/// function call helper visitor //
// need this to get collect the expressions of the function arguments
class function_argument_acclimator : public revisitor_tree<source_expression_visitor>
{
public:
    vector<C_expression_ptr> argument_expressions;

    shared_ptr< revisitor_tree<source_expression_visitor> > make_revistor_child(int number)
    {
        return make_shared<function_argument_acclimator>( );
    }

    void callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs)
    {
        function_argument_acclimator*  unArgs_child_casted = nullptr;
        function_argument_acclimator*  namedArgs_casted = nullptr;

        int T = 0;
        if( unArgs_child )
        {
            unArgs_child_casted = dynamic_cast<function_argument_acclimator*>( unArgs_child );
            T += unArgs_child_casted->argument_expressions.size();
        }
        if( namedArgs )
        {
            namedArgs_casted = dynamic_cast<function_argument_acclimator*>( namedArgs );
            T += namedArgs_casted->argument_expressions.size();
        }

        argument_expressions.reserve( T );

        if( unArgs_child_casted )
        {
            for(auto C_exp : unArgs_child_casted->argument_expressions )
            {
                argument_expressions.push_back( C_exp );
            }
        }
        if( namedArgs_casted )
        {
            for(auto C_exp : namedArgs_casted->argument_expressions )
            {
                argument_expressions.push_back( C_exp );
            }
        }
    }

    void baseArguments_up(call_argument_list::base_arguments_T* argList, list<AST_visitor_base*>& visitor_children)
    {
        argument_expressions.reserve( visitor_children.size() );
        for( auto ptr : visitor_children )
        {
            auto src_exp_ptr = dynamic_cast<function_argument_acclimator*>( ptr )->twin;

            argument_expressions.push_back( src_exp_ptr->final_expression );
        }
    }
};


/// source_expression_visitor ///

shared_ptr< AST_visitor_base > source_expression_visitor::make_child(int number)
{
    return make_shared<source_expression_visitor>( source_fout );
}

void source_expression_visitor::append_childExp_to_finalExp()
// this loops over all child expressions, and adds their final_expression to this final_expresson
{
    for(auto child : children)
    {
        auto cast_child = dynamic_pointer_cast<source_expression_visitor>( child );
        if( cast_child->final_expression )
        {
            final_expression->add_cleanup_child( cast_child->final_expression );
        }
    }
}


void source_expression_visitor::intLiteral_up(intLiteral_expression_AST_node* intLitExp)
{

    if( intLitExp->expression_return_type->definition_name == "UNNAMED_C_TYPE" )
    {
        stringstream exp;
        exp << intLitExp->literal;

        final_expression = make_shared<simple_C_expression>( exp.str(), intLitExp->expression_return_type,
                                                            false, false );
    }
    else
    {
        auto cy_int_type = intLitExp->expression_return_type;
        auto output_name = "__cy__tmp_" + source_fout->get_unique_string();

        cy_int_type->C_definition_name( output_name, source_fout );
        source_fout->out_strm() << ';' << endl;
        cy_int_type->initialize( output_name, source_fout );

        auto LHS_writer = make_shared<owned_name>( cy_int_type, output_name );

        cy_int_type->write_default_constructor(output_name, source_fout);

        utf8_string member("__val__");
        auto val_setter = cy_int_type->write_member_getref( LHS_writer, member, source_fout );
        source_fout->out_strm() <<  source_fout->ln_strt << val_setter->get_C_expression() << '=' << intLitExp->literal << ';' << endl;
        val_setter->write_cleanup( source_fout );


        final_expression = LHS_writer;

    }
}

void source_expression_visitor::binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
{

    source_expression_visitor* LHS_visitor = dynamic_cast<source_expression_visitor*>(LHS_exp_visitor);
    source_expression_visitor* RHS_visitor = dynamic_cast<source_expression_visitor*>(RHS_exp_visitor);

    auto LHS_Cexp = LHS_visitor->final_expression;
    auto RHS_Cexp = RHS_visitor->final_expression;

    auto return_type = binOprExp->expression_return_type;

    switch(binOprExp->type_of_operation)
    {

case binOperator_expression_AST_node::empty : // this should NEVER happen
    cout << "ERROR in source_expression_visitor::binOperator_up. This should not be reached"<< endl;
    break;
/// MATH operators ///
// power
case binOperator_expression_AST_node::power_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSpower(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSpower(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// multiplication
case binOperator_expression_AST_node::multiplication_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSmultiplication(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSmultiplication(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// division
case binOperator_expression_AST_node::division_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSdivision(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSdivision(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// modulus
case binOperator_expression_AST_node::modulus_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSmodulus(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSmodulus(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// addition
case binOperator_expression_AST_node::addition_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSaddition(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSaddition(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// subtraction
case binOperator_expression_AST_node::subtraction_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSsubtraction(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSsubtraction(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

/// comparison operators ///
// <
case binOperator_expression_AST_node::lessThan_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSlessThan(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSlessThan(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// >
case binOperator_expression_AST_node::greatThan_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_RHSgreatThan(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSgreatThan(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// ==
case binOperator_expression_AST_node::equalTo_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSequalTo(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSequalTo(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// !=
case binOperator_expression_AST_node::notEqual_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSnotEqual(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSnotEqual(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// <=
case binOperator_expression_AST_node::lessEqual_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSlessEqual(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSlessEqual(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;

// >=
case binOperator_expression_AST_node::greatEqual_t :
    if(binOprExp->mode_of_operation == binOperator_expression_AST_node::LHS_m)
    {
        final_expression = LHS_Cexp->cyth_type->write_LHSgreatEqual(LHS_Cexp, RHS_Cexp, source_fout);
    }
    else
    {
        final_expression = RHS_Cexp->cyth_type->write_RHSgreatEqual(LHS_Cexp, RHS_Cexp, source_fout);
    }
    break;


default:
    throw gen_exception("bad operation in source_expression_visitor::binOperator_up");
    }

    append_childExp_to_finalExp();

}



void source_expression_visitor::binBoolOperator_down(binBoolOp_expression_AST_node* binBoolOprExp)
{

    do_children = false;  // we need to do something weird to allow for short circuiting


// a function to avoid code bloat
    struct {
    C_expression_ptr operator() (expression_AST_ptr operand, varType_ptr bool_type, Csource_out_ptr source_fout)
    {
    // write the expression
        source_expression_visitor opVisitor( source_fout );
        operand->apply_visitor( &opVisitor );

        auto Op_Cexp = opVisitor.final_expression;
        auto Op_type = Op_Cexp->cyth_type;

    // make sure is a bool
        C_expression_ptr exp_to_use;

        if( bool_type->is_equivalent(Op_type) )
        {
            exp_to_use = Op_Cexp;
        }
        else if( bool_type->has_implicit_copy_constructor(Op_type, cast_enum::pntr_casts) )
        {
            utf8_string LHS_bool_var = "__TMP_bool_" + source_fout->get_unique_string();

            bool_type->C_definition_name(LHS_bool_var, source_fout);
            source_fout->out_strm() << ';' << endl;
            bool_type->initialize(LHS_bool_var, source_fout);
            exp_to_use = make_shared<owned_name>(bool_type, LHS_bool_var);

            bool_type->write_implicit_copy_constructor( exp_to_use, Op_Cexp,
                                        source_fout, cast_enum::pntr_casts);

            exp_to_use->add_cleanup_child( Op_Cexp );
        }
        else if( Op_type->can_implicit_castTo(bool_type) )
        {
            utf8_string LHS_bool_var = "__TMP_bool_" + source_fout->get_unique_string();

            bool_type->C_definition_name(LHS_bool_var, source_fout);
            source_fout->out_strm() << ';' << endl;
            bool_type->initialize(LHS_bool_var, source_fout);
            exp_to_use = make_shared<owned_name>(bool_type, LHS_bool_var);

            Op_type->write_implicit_castTo( exp_to_use, Op_Cexp, source_fout);

            exp_to_use->add_cleanup_child( Op_Cexp );
        }
        else
        {
            throw gen_exception("This should not be reached. Type ", Op_type->definition_name, " cannot be implicitly converted to bool. at",
                                operand->loc);
        }

    // get C-expression

        Op_Cexp = bool_type->toC(exp_to_use,  source_fout);

        Op_Cexp->add_cleanup_child( exp_to_use );

        return Op_Cexp;

    }
    } writeBoolExp;



    varType_ptr bool_type = binBoolOprExp->expression_return_type; // ought to be correct


/// first write LHS
    auto LHS_cExp = writeBoolExp( binBoolOprExp->left_operand, bool_type, source_fout);


/// define and initialize output variable
    auto output_name = "__cy__tmp_" + source_fout->get_unique_string();

    bool_type->C_definition_name( output_name, source_fout );
    source_fout->out_strm() << ';' << endl;
    bool_type->initialize( output_name, source_fout );
    bool_type->write_default_constructor(output_name, source_fout);

    final_expression = make_shared<owned_name>( bool_type, output_name );


    utf8_string member("__val__");

/// operator specific details
    if( binBoolOprExp->type_of_operation == binBoolOp_expression_AST_node::and_t )
    {
    // initialize to false
        auto val_setter = bool_type->write_member_getref( final_expression, member, source_fout );
        source_fout->out_strm() <<  source_fout->ln_strt << val_setter->get_C_expression() << "= 0;" << endl;
        val_setter->write_cleanup( source_fout );

    // check LHS
        source_fout->out_strm() <<  source_fout->ln_strt << "if("<< LHS_cExp->get_C_expression() <<"){"<< endl;

    // write RHS
        auto RHS_cExp = writeBoolExp( binBoolOprExp->right_operand, bool_type, source_fout);
    // check RHS
        source_fout->out_strm() <<  source_fout->ln_strt << "if("<< RHS_cExp->get_C_expression() <<"){"<< endl;

        // our value is true!
        val_setter = bool_type->write_member_getref( final_expression, member, source_fout );
        source_fout->out_strm() <<  source_fout->ln_strt << val_setter->get_C_expression() << "= 1;" << endl;
        val_setter->write_cleanup( source_fout );

    // cleanup!!
        source_fout->out_strm() <<  source_fout->ln_strt << "}" << endl;
        RHS_cExp->write_cleanup( source_fout );

        source_fout->out_strm() <<  source_fout->ln_strt << "}" << endl;

    }
    else if( binBoolOprExp->type_of_operation == binBoolOp_expression_AST_node::or_t )
    {
    // initialize to true
        auto val_setter = bool_type->write_member_getref( final_expression, member, source_fout );
        source_fout->out_strm() <<  source_fout->ln_strt << val_setter->get_C_expression() << "= 1;" << endl;
        val_setter->write_cleanup( source_fout );

    // check LHS
        source_fout->out_strm() <<  source_fout->ln_strt << "if( !("<< LHS_cExp->get_C_expression() <<")){"<< endl;

    // write RHS
        auto RHS_cExp = writeBoolExp( binBoolOprExp->right_operand, bool_type, source_fout);
    // check RHS
        source_fout->out_strm() <<  source_fout->ln_strt << "if( !("<< RHS_cExp->get_C_expression() <<")){"<< endl;

        // our value is true!
        val_setter = bool_type->write_member_getref( final_expression, member, source_fout );
        source_fout->out_strm() <<  source_fout->ln_strt << val_setter->get_C_expression() << "= 0;" << endl;
        val_setter->write_cleanup( source_fout );

    // cleanup!!
        source_fout->out_strm() <<  source_fout->ln_strt << "}" << endl;
        RHS_cExp->write_cleanup( source_fout );

        source_fout->out_strm() <<  source_fout->ln_strt << "}" << endl;

    }
    else if( binBoolOprExp->type_of_operation == binBoolOp_expression_AST_node::xor_t )
    {
      // no short-circuiting!!
     // write RHS
        auto RHS_cExp = writeBoolExp( binBoolOprExp->right_operand, bool_type, source_fout);

     // set

        auto val_setter = bool_type->write_member_getref( final_expression, member, source_fout );
        source_fout->out_strm() <<  source_fout->ln_strt << val_setter->get_C_expression() << "= ( !(" << LHS_cExp->get_C_expression() << ") != !(" << RHS_cExp->get_C_expression() << "));"<< endl;
        val_setter->write_cleanup( source_fout );

    // cleanup!!
        RHS_cExp->write_cleanup( source_fout );

    }
    else
    {
        throw gen_exception("This should not be reached in source_expression_visitor::binBoolOperator_up");
    }

    LHS_cExp->write_cleanup( source_fout );

}

void source_expression_visitor::varReferance_up(varReferance_expression_AST_node* varRefExp)
{
    final_expression = make_shared<simple_C_expression>( varRefExp->variable_symbol->C_name, varRefExp->expression_return_type,
                                                        true, false );
}

void source_expression_visitor::ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor)
{
    source_expression_visitor* EXP_visitor_PTR = dynamic_cast<source_expression_visitor*>(expChild_visitor);
    final_expression = EXP_visitor_PTR->final_expression;

    //parenGroupExp->writer = make_shared<simple_expression_writer>( parenGroupExp->expression->writer->get_C_expression() );
    //parenGroupExp->c_exp_can_be_referenced = parenGroupExp->expression->c_exp_can_be_referenced;
}



void source_expression_visitor::accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor)
{
    auto parentType = accessorExp->expression->expression_return_type;

    source_expression_visitor* EXP_visitor_PTR = dynamic_cast<source_expression_visitor*>(expChild_visitor);
    auto parent_C_exp = EXP_visitor_PTR->final_expression;


    //auto child_exp = accessorExp->expression->writer->get_C_expression();
    final_expression = parentType->write_member_getter(parent_C_exp, accessorExp->name, source_fout);
    append_childExp_to_finalExp();
}

void source_expression_visitor::functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child)
{
    // apply to get all argument expressions
    function_argument_acclimator func_args; //class function_argument_acclimator : public revisitor_tree<source_expression_visitor>
    source_expression_visitor* arguments_child_PTR = dynamic_cast<source_expression_visitor*>(arguments_child);
    apply_revisitor(funcCall->argument_list.get(), arguments_child_PTR, &func_args  );

    // argument types
    auto argument_types = funcCall->argument_list->get_argument_types();


    // get parent expressions
    source_expression_visitor* expression_child_PTR = dynamic_cast<source_expression_visitor*>(expression_child);
    auto parent_C_exp = expression_child_PTR->final_expression;
    auto parent_type = parent_C_exp->cyth_type;

    // CALL!
    final_expression = parent_type->write_call( argument_types, parent_C_exp, func_args.argument_expressions, source_fout);
    append_childExp_to_finalExp();

}





/// source_statement_visitor ///

// a helper class
class statment_destructuble_finder : public AST_visitor_NoChildren
// apply to statements to find variable declartions. Then call "cleanup" to write destructors for thos variables
{
public:
    list< varName_ptr > names_to_destruct;

    void generic_VarDef_down(General_VarDefinition* vardef) override
    {
        names_to_destruct.push_back( vardef->variable_symbol );
    }


    void cleanup(Csource_out_ptr source_fout, location max_location)
    //WIll not destruct variables declared after max_location
    {
        for( auto var_to_destruct : names_to_destruct )
        {
            if( var_to_destruct->loc.end  <= max_location )
            {
                var_to_destruct->var_type->write_destructor( var_to_destruct->C_name, source_fout);
            }
        }
    }

    void cleanup(Csource_out_ptr source_fout)
    {
        for( auto var_to_destruct : names_to_destruct )
        {
            var_to_destruct->var_type->write_destructor( var_to_destruct->C_name, source_fout);
        }
    }

};

void cleanup_block(block_AST_node* block, Csource_out_ptr source_fout, location max_location)
// this will write destructors for all declared variables in this exact block. WIll not destruct variables declared after max_location
{
    // apply visitor to each node
    statment_destructuble_finder finder;

    for( auto &AST_node : block->contents )
    {
        AST_node->apply_visitor( &finder );
    }

    // now destruct it all!
    finder.cleanup( source_fout, max_location );
}





source_statement_visitor::source_statement_visitor(Csource_out_ptr _source_fout, bool _write_definitions) :
    source_fout(_source_fout)
{
    do_children = true;
    write_definitions = _write_definitions;
    expression_to_cleanup = nullptr;
}

shared_ptr< AST_visitor_base > source_statement_visitor::make_child(int number)
{
    return make_shared<source_statement_visitor>( source_fout );
}


/// things that cause this visitor to stop ////
void source_statement_visitor::funcDef_down(function_AST_node* funcDef)
{
    do_children = false;
}

/// "normal" things ////
void source_statement_visitor::ASTnode_up(AST_node* ASTnode)
{
    if( expression_to_cleanup )
    {
        expression_to_cleanup->write_cleanup( source_fout );
    }
}


void source_statement_visitor::block_up(block_AST_node* block, std::list<AST_visitor_base*>& visitor_children)
{
    cleanup_block(block, source_fout, block->loc.end);
}

// flow control

void source_statement_visitor::activeCond_down(activeCond_AST_node* condNode)
{

    source_expression_visitor expr_vistr( source_fout );
    condNode->if_expression->apply_visitor( &expr_vistr );
    auto exp_type = condNode->if_expression->expression_return_type;

    //utf8_string conditional_vname = "__cy__retTMP_" + source_fout->get_unique_string();

    utf8_string final_C_exp;
    expression_to_cleanup = expr_vistr.final_expression;

    if( exp_type->type_of_type == varType::c_import_t ) // is simple C-type
    {
        final_C_exp = expr_vistr.final_expression->get_C_expression();
    }
    else // not C-type. Need to check if bool, and try to convert if not
    {
        utf8_string bool_name("bool");
        location_span loc;
        bool check_order = false;
        varType_ptr bool_type = condNode->symbol_table->get_type_global(bool_name, loc, check_order);

        C_expression_ptr bool_exp_to_write = expr_vistr.final_expression;


        if( not bool_type ) // bool type not defined yet.
        {
            throw gen_exception("This should not be reached. expression is not c-type, and bool is not defined, at ", condNode->if_expression->loc);
        }
        else if( not exp_type->is_equivalent(bool_type) ) // needs to cast to bool
        {
            utf8_string new_bool_var = "__TMP_bool_" + source_fout->get_unique_string();

            bool_type->C_definition_name(new_bool_var, source_fout);
            source_fout->out_strm() << ';' << endl;
            bool_type->initialize(new_bool_var, source_fout);


            bool_exp_to_write = make_shared<owned_name>(bool_type, new_bool_var);
            expression_to_cleanup->add_cleanup_child( bool_exp_to_write );

            if( bool_type->has_implicit_copy_constructor(exp_type, cast_enum::pntr_casts) )
            {
                bool_type->write_implicit_copy_constructor( bool_exp_to_write, expr_vistr.final_expression,
                                            source_fout, cast_enum::pntr_casts);

            }
            else if( exp_type->can_implicit_castTo(bool_type) )
            {
                exp_type->write_implicit_castTo( bool_exp_to_write, expr_vistr.final_expression, source_fout);
            }
            else
            {
                throw gen_exception("This should not be reached. Type ", exp_type->definition_name, " cannot be implicitly converted to bool. at",
                                    condNode->if_expression->loc);
            }
        }

        if( not bool_exp_to_write->can_be_referenced ) // make sure the exp can be referenced
        {
            C_expression_ptr old_exp = bool_exp_to_write;


            utf8_string new_bool_var = "__TMP_bool_" + source_fout->get_unique_string();

            bool_type->C_definition_name(new_bool_var, source_fout);
            source_fout->out_strm() << ';' << endl;
            bool_type->initialize(new_bool_var, source_fout);

            bool_exp_to_write = make_shared<owned_name>(bool_type, new_bool_var);
            expression_to_cleanup->add_cleanup_child( bool_exp_to_write );

            bool_type->write_implicit_copy_constructor( bool_exp_to_write, old_exp, source_fout, cast_enum::implicit_casts);


        }

        // now convert bool to C int (I hope?)
        auto C_exp = bool_exp_to_write->cyth_type->toC(bool_exp_to_write, source_fout);
        expression_to_cleanup->add_cleanup_child( C_exp );
        final_C_exp = C_exp->get_C_expression();

    }

    // write the if!
    source_fout->out_strm() << source_fout->ln_strt << "if( " << final_C_exp << ")" << endl;
    source_fout->out_strm() << source_fout->ln_strt << '{' << endl;
    source_fout->enter_scope();

    // now block, and then children will be written

}

void source_statement_visitor::activeCond_up(activeCond_AST_node* condNode, AST_visitor_base* ifExp_child, AST_visitor_base* block_child, AST_visitor_base* childConditional)
{
    source_fout->leave_scope();
    source_fout->out_strm() << source_fout->ln_strt << '}' << endl;
    // expression will then be cleaned up
}

void source_statement_visitor::elifCond_down(elif_AST_node* elifNode)
{
    source_fout->leave_scope();
    source_fout->out_strm() << source_fout->ln_strt << '}' << endl;
    source_fout->out_strm() << source_fout->ln_strt << "else" << endl;
    source_fout->out_strm() << source_fout->ln_strt << '{' << endl;
    source_fout->enter_scope();
}

void source_statement_visitor::else_down(else_AST_node* elifNode)
{
    source_fout->leave_scope();
    source_fout->out_strm() << source_fout->ln_strt << '}' << endl;
    source_fout->out_strm() << source_fout->ln_strt << "else" << endl;
    source_fout->out_strm() << source_fout->ln_strt << '{' << endl;
    source_fout->enter_scope();
}


/// LOOPS
// first some helper functions
// will need to be adjusted when we do iter-loops
void write_start(loop_AST_node* loop_node, expression_AST_ptr cntr_exp, Csource_out_ptr source_fout)
{
// initialize any loop controls
    for( auto& cntrlNode : loop_node->control_statments )
    {
        if( (not cntrlNode.next) and (cntrlNode.cntrl_node->depth !=0) )
        {
            // this loop is terminal for a multi-level loop-control stmt
            // need to declare the control variable
            // first make sure we have a name
            if( cntrlNode.cntrl_node->var_control_name.get_length() == 0 ) // no idea when this is set... so set here if necisary
            {
               cntrlNode.cntrl_node->var_control_name = "__cy__loopCntrl__" + source_fout->get_unique_string();
            }

            // write
            source_fout->out_strm() << source_fout->ln_strt << "int " << cntrlNode.cntrl_node->var_control_name  << "=0;" << endl;

        }
    }


// write the start
    source_fout->out_strm() << source_fout->ln_strt << "while( 1 ) {" << endl;
    source_fout->enter_scope();


// write the expression, and convert as necisary
    source_expression_visitor expr_vistr( source_fout );
    cntr_exp->apply_visitor( &expr_vistr );
    auto exp_type = cntr_exp->expression_return_type;

    utf8_string final_C_exp;
    C_expression_ptr expression_to_cleanup = expr_vistr.final_expression;

    if( exp_type->type_of_type == varType::c_import_t ) // is simple C-type
    {
        final_C_exp = expr_vistr.final_expression->get_C_expression();
    }
    else // not C-type. Need to check if bool, and try to convert if not
    {
        utf8_string bool_name("bool");
        location_span loc;
        bool check_order = false;
        varType_ptr bool_type = loop_node->symbol_table->get_type_global(bool_name, loc, check_order);

        C_expression_ptr bool_exp_to_write = expr_vistr.final_expression;


        if( not bool_type ) // bool type not defined yet.
        {
            throw gen_exception("This should not be reached. expression is not c-type, and bool is not defined, at ", cntr_exp->loc);
        }
        else if( not exp_type->is_equivalent(bool_type) ) // needs to cast to bool
        {
            utf8_string new_bool_var = "__TMP_bool_" + source_fout->get_unique_string();

            bool_type->C_definition_name(new_bool_var, source_fout);
            source_fout->out_strm() << ';' << endl;
            bool_type->initialize(new_bool_var, source_fout);


            bool_exp_to_write = make_shared<owned_name>(bool_type, new_bool_var);
            expression_to_cleanup->add_cleanup_child( bool_exp_to_write );

            if( bool_type->has_implicit_copy_constructor(exp_type, cast_enum::pntr_casts) )
            {
                bool_type->write_implicit_copy_constructor( bool_exp_to_write, expr_vistr.final_expression,
                                            source_fout, cast_enum::pntr_casts);

            }
            else if( exp_type->can_implicit_castTo(bool_type) )
            {
                exp_type->write_implicit_castTo( bool_exp_to_write, expr_vistr.final_expression, source_fout);
            }
            else
            {
                throw gen_exception("This should not be reached. Type ", exp_type->definition_name, " cannot be implicitly converted to bool. at",
                                    cntr_exp->loc);
            }
        }

        if( not bool_exp_to_write->can_be_referenced ) // make sure the exp can be referenced
        {
            C_expression_ptr old_exp = bool_exp_to_write;


            utf8_string new_bool_var = "__TMP_bool_" + source_fout->get_unique_string();

            bool_type->C_definition_name(new_bool_var, source_fout);
            source_fout->out_strm() << ';' << endl;
            bool_type->initialize(new_bool_var, source_fout);

            bool_exp_to_write = make_shared<owned_name>(bool_type, new_bool_var);
            expression_to_cleanup->add_cleanup_child( bool_exp_to_write );

            bool_type->write_implicit_copy_constructor( bool_exp_to_write, old_exp, source_fout, cast_enum::implicit_casts);


        }

        // now convert bool to C int (I hope?)
        auto C_exp = bool_exp_to_write->cyth_type->toC(bool_exp_to_write, source_fout);
        expression_to_cleanup->add_cleanup_child( C_exp );
        final_C_exp = C_exp->get_C_expression();

    }

    utf8_string cntrl_var = "__loopCntrlTmp_" + source_fout->get_unique_string();
    source_fout->out_strm() << source_fout->ln_strt << "int " << cntrl_var << " = " << final_C_exp << ";" << endl;
    expression_to_cleanup->write_cleanup( source_fout );
    source_fout->out_strm() << source_fout->ln_strt << "if( ! " << cntrl_var << ") {break;}" << endl;
}

void end_loop_controls(loop_AST_node* loop_node, Csource_out_ptr source_fout)
{
    for( auto& cntrlNode : loop_node->control_statments )
    {
        if(  cntrlNode.next ) // this does not apply to terminal loop
        {
            if( cntrlNode.cntrl_node->var_control_name.get_length() == 0 ) // no idea when this is set... so set here if necisary
            {
               cntrlNode.cntrl_node->var_control_name = "__cy__loopCntrl__" + source_fout->get_unique_string();
            }

            source_fout->out_strm() << source_fout->ln_strt << "if("  << cntrlNode.cntrl_node->var_control_name << "){" << endl;
            source_fout->enter_scope();

            block_AST_ptr block = cntrlNode.next->current_loop->block_AST;
            cleanup_block(block.get(), source_fout, loop_node->loc.start);

            if( cntrlNode.next->next ) // penultimate loop behaves differently
            {
                source_fout->out_strm() << source_fout->ln_strt << "break;" << endl;
            }
            else
            {
                if( cntrlNode.cntrl_node->type == loopCntrl_statement_AST_node::break_t )
                {
                    source_fout->out_strm() << source_fout->ln_strt << "break;" << endl;
                }
                else
                {
                    source_fout->out_strm() << source_fout->ln_strt << "continue;" << endl;
                }
            }


            source_fout->leave_scope();
            source_fout->out_strm() << source_fout->ln_strt << "}"<<endl;
        }
    }
}

void source_statement_visitor::while_down(whileLoop_AST_node* whileLoopNode)
{
    write_start(whileLoopNode,  whileLoopNode->while_expression,  source_fout);

    // now block will be written
}

void source_statement_visitor::while_up(whileLoop_AST_node* whileLoopNode, AST_visitor_base* whileExp_child, AST_visitor_base* Block_child)
{
    source_fout->leave_scope();
    source_fout->out_strm() << source_fout->ln_strt << '}' << endl;

    end_loop_controls(whileLoopNode, source_fout);
}


void source_statement_visitor::for_down(forLoop_AST_node* forLoopNode)
// this is very complicated. So sets do_children=false, and does them itself in correct order.
{
    do_children = false;
    initiate_children( 3 );
    // a bit wierd in that we now do childern, and THEN we do down visitors in AST callback!
    //    ...oh well.

// first we write the initiation stmt.
    AST_visitor_base* initialStmt_child = get_child(0);
    forLoopNode->initial_statement->apply_visitor( initialStmt_child );

// we write looping bit
    write_start(forLoopNode,  forLoopNode->while_expression,  source_fout);


// now we write the block
    AST_visitor_base* block_child = get_child(1);
    forLoopNode->block_AST->apply_visitor( block_child );


// now we update
    AST_visitor_base* updateStmt_child = get_child(2);
    forLoopNode->update_statement->apply_visitor( updateStmt_child );

    statment_destructuble_finder update_stmnt_destruct;
    forLoopNode->update_statement->apply_visitor( &update_stmnt_destruct );
    update_stmnt_destruct.cleanup( source_fout );


// now we write exit
    source_fout->leave_scope();
    source_fout->out_strm() << source_fout->ln_strt << '}' << endl;

// destruct initial stmt
    statment_destructuble_finder initial_stmnt_destruct;
    forLoopNode->initial_statement->apply_visitor( &initial_stmnt_destruct );
    initial_stmnt_destruct.cleanup( source_fout );

// write loop controls
    end_loop_controls(forLoopNode, source_fout);


    // skip ups because this is weird enough
}



// individual statements
void source_statement_visitor::statement_up(statement_AST_node* statment)
{
    source_fout->out_strm() <<  endl;
}

void source_statement_visitor::expressionStatement_down(expression_statement_AST_node* expStmt)
{
    source_expression_visitor expr_vistr( source_fout );
    expStmt->expression->apply_visitor( &expr_vistr );

    // write exp
    source_fout->out_strm() << source_fout->ln_strt << (expr_vistr.final_expression->get_C_expression() ) << ";" << endl;

    expr_vistr.final_expression->write_cleanup( source_fout );
}



void source_statement_visitor::definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child)
{
    if( write_definitions )
    {
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, source_fout );
        source_fout->out_strm() << ";" << endl;
    }

    defStmt->var_type->resolved_type->initialize( defStmt->variable_symbol->C_name, source_fout );

    defStmt->var_type->resolved_type->write_default_constructor(defStmt->variable_symbol->C_name, source_fout);

    source_fout->out_strm() << endl;
}

void source_statement_visitor::definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child)
{
    if( write_definitions )
    {
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, source_fout );
        source_fout->out_strm() << ";" << endl;
    }

    defStmt->var_type->resolved_type->initialize( defStmt->variable_symbol->C_name, source_fout );


    // argument expressions
    source_expression_visitor arguments_visitor( source_fout );
    defStmt->argument_list->apply_visitor( &arguments_visitor );

    function_argument_acclimator func_args;
    apply_revisitor(defStmt->argument_list.get(), &arguments_visitor, &func_args  );

    // argument types
    auto argument_types = defStmt->argument_list->get_argument_types();


    // get parent expressions
    auto var_type = defStmt->var_type->resolved_type;
    auto var_C_name = defStmt->variable_symbol->C_name;
    auto variable_LHS_expression = make_shared<simple_C_expression>( var_C_name, var_type, true, false );

    // WRITE!!
    var_type->write_explicit_constructor( argument_types, variable_LHS_expression,
                        func_args.argument_expressions, source_fout);


    // cleanup
    variable_LHS_expression->write_cleanup( source_fout );
    for( auto arg_exp : func_args.argument_expressions)
    {
        arg_exp->write_cleanup( source_fout );
    }
}

void source_statement_visitor::definitionNassignment_up(definitionNassignment_statement_AST_node* defStmt,
                                    AST_visitor_base* varTypeRepr_child, AST_visitor_base* exp_child)
{

    if( write_definitions )
    {
        defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, source_fout );
        source_fout->out_strm() << ";" << endl;
    }

    defStmt->var_type->resolved_type->initialize( defStmt->variable_symbol->C_name, source_fout );


    // first handle the RHS
    source_expression_visitor arguments_visitor_PTR( source_fout );
    defStmt->expression->apply_visitor( &arguments_visitor_PTR );

    auto RHS_type = defStmt->expression->expression_return_type;
    auto RHS_c_exp = arguments_visitor_PTR.final_expression;


    // now the LHS
    auto LHS_name = defStmt->variable_symbol->C_name;
    auto LHS_type = defStmt->variable_symbol->var_type;
    auto variable_LHS_expression = make_shared<simple_C_expression>( LHS_name, LHS_type, true, false );


    //write!
    utf8_string TMP;
    if( LHS_type->has_explicit_copy_constructor( RHS_type, TMP) )
    {
        LHS_type->write_explicit_copy_constructor(variable_LHS_expression, RHS_c_exp, source_fout);
    }
    else
    {
        RHS_type->write_explicit_castTo(variable_LHS_expression, RHS_c_exp, source_fout);
    }

    // cleanup
    variable_LHS_expression->write_cleanup( source_fout );
    RHS_c_exp->write_cleanup( source_fout );

}



void source_statement_visitor::assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child)
{
    // RHS
    source_expression_visitor expr_vistr( source_fout );
    assignStmt->expression->apply_visitor( &expr_vistr );
    auto RHS_exp = expr_vistr.final_expression;

    // LHS
    source_LHSreference_visitor LHS_vistr( source_fout, RHS_exp);
    assignStmt->LHS->apply_visitor( &LHS_vistr );
    source_fout->out_strm() << endl;

    // cleanup
    RHS_exp->write_cleanup( source_fout );
    //LHS_vistr.final_expression->write_cleanup( source_fout );
}

void source_statement_visitor::autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child)
{
    // define and initialize
    auto LHS_type = autoStmt->variable_symbol->var_type;
    auto LHS_name = autoStmt->variable_symbol->C_name;

    if( write_definitions )
    {
        LHS_type->C_definition_name( LHS_name, source_fout );
        source_fout->out_strm() << ";" << endl;

    } /// not sure why this isn't a generic definition type???

    LHS_type->initialize( LHS_name, source_fout );

    auto variable_LHS_expression = make_shared<simple_C_expression>( LHS_name, LHS_type, true, false );


    // handle RHS
    source_expression_visitor expr_vistr( source_fout );
    autoStmt->expression->apply_visitor( &expr_vistr );
    auto RHS_c_exp = expr_vistr.final_expression;
    auto RHS_type = autoStmt->expression->expression_return_type;


    //write!
    utf8_string TMP;
    if( LHS_type->has_explicit_copy_constructor( RHS_type, TMP) )
    {
        LHS_type->write_explicit_copy_constructor(variable_LHS_expression, RHS_c_exp, source_fout);
    }
    else
    {
        RHS_type->write_explicit_castTo(variable_LHS_expression, RHS_c_exp, source_fout);
    }

    // cleanup
    variable_LHS_expression->write_cleanup( source_fout );
    RHS_c_exp->write_cleanup( source_fout );

}

void source_statement_visitor::returnStatement_down(return_statement_AST_node* returnStmt)
{
// TODO: return statement tech needs to be improved with up-visitor technology
    // RHS
    source_expression_visitor expr_vistr( source_fout );
    returnStmt->expression->apply_visitor( &expr_vistr );
    auto RHS_C_exp = expr_vistr.final_expression;
    auto expression_type = returnStmt->expression->expression_return_type;


    // LHS (to return)
    auto return_type = returnStmt->callable_to_escape->return_type;

    utf8_string argument_vname = "__cy__retTMP_" + source_fout->get_unique_string();
    return_type->C_definition_name(argument_vname, source_fout);
    source_fout->out_strm() << ';' << endl;
    return_type->initialize(argument_vname, source_fout);
    auto ret_var = make_shared<simple_C_expression>(argument_vname, return_type, true, false);
        // not owned, as this will be returned, and the caller actually owns this memory?? Thus not an owned_name


    // do cast!
    if( return_type->has_implicit_copy_constructor( expression_type ) )
    {
        return_type->write_implicit_copy_constructor(ret_var, RHS_C_exp, source_fout);
    }
    else if(  expression_type->can_implicit_castTo( return_type ) )
    {
        expression_type->write_implicit_castTo(ret_var, RHS_C_exp, source_fout);
    }
    else
    {
        throw gen_exception("this should never be reached. In source_statement_visitor::returnStatement_down");
    }

    // cleanup expression
    RHS_C_exp->write_cleanup( source_fout );


    // call destructors on active variables
    // first, find variables to destruct
    list<varName_ptr> variables_to_destruct;
    sym_table_base* current_symbol_table = returnStmt->symbol_table;
    sym_table_base* outer_symbol_table = returnStmt->callable_to_escape->inner_symbol_table.get();
    while( true )
    {

        for(auto& x : current_symbol_table->variable_table)
        {
            auto current_var_name = x.second;
            if( current_var_name->loc.strictly_LT( returnStmt->loc ) )
            {
                variables_to_destruct.push_back( current_var_name );
            }
        }

        if( current_symbol_table == outer_symbol_table)
        {
            break;
        }

        auto __current_symbol_table__ = dynamic_cast<local_sym_table*>( current_symbol_table );
        current_symbol_table = __current_symbol_table__->parent_table;
    }
    // then..... DESTROY!!
    for( auto& var : variables_to_destruct )
    {
        var->var_type->write_destructor( var->C_name, source_fout );
    }

    // write return
    source_fout->out_strm() << source_fout->ln_strt << "return " << argument_vname << ";" << endl;
}

void source_statement_visitor::constructElement_up(constructElement_AST_node* constructBlock, AST_visitor_base* exp_child, AST_visitor_base* argList_child)
{

    // argument expressions
    source_expression_visitor arguments_visitor( source_fout );
    constructBlock->argument_list->apply_visitor( &arguments_visitor );

    function_argument_acclimator func_args;
    apply_revisitor(constructBlock->argument_list.get(), &arguments_visitor, &func_args  );

    // argument types
    auto argument_types = constructBlock->argument_list->get_argument_types();



    // get LHS expression
    source_expression_visitor expr_vistr( source_fout );
    constructBlock->expression->apply_visitor( &expr_vistr );
    auto LHS_type = constructBlock->expression->expression_return_type;
    auto LHS_exp = expr_vistr.final_expression;



    // WRITE!!
    LHS_type->write_explicit_constructor( argument_types, LHS_exp,
                        func_args.argument_expressions, source_fout);



    // cleanup
    LHS_exp->write_cleanup( source_fout );
    for( auto arg_exp : func_args.argument_expressions)
    {
        arg_exp->write_cleanup( source_fout );
    }
}

void source_statement_visitor::loopCntrlStatement_up(loopCntrl_statement_AST_node* cntrlStmt)
{

    if( cntrlStmt->depth>0 and cntrlStmt->var_control_name.get_length()==0 ) // should never be tripped I think. var_control_name Probably set by loop_down
    {
        cntrlStmt->var_control_name = "__cy__loopCntrl__" + source_fout->get_unique_string();
    }

    auto top_block = cntrlStmt->top_loop->current_loop->block_AST;
    bool top_is_terminal = cntrlStmt->depth == 0;

    // cleanup top block here
    cleanup_block(top_block.get(), source_fout, cntrlStmt->loc.end);

    // set control statement
    if( top_is_terminal )
    {
        if( cntrlStmt->type == loopCntrl_statement_AST_node::break_t )
        {
            source_fout->out_strm() << source_fout->ln_strt << "break;" << endl;
        }
        else
        {
            source_fout->out_strm() << source_fout->ln_strt << "continue;" << endl;
        }
    }
    else
    {
        source_fout->out_strm() << source_fout->ln_strt << cntrlStmt->var_control_name << " = 1;" << endl;
        source_fout->out_strm() << source_fout->ln_strt << "break;" << endl;
    }

}







// writes the module setup function
class source_moduleExpresion_visitor : public AST_visitorTree
{
// essentially skims top-level AST for simple expressions, and writes them to the setup function.
// uses source_statement_visitor to actually do the writing.
// does not write functions, classes, or complex thingies

public:
    Csource_out_ptr source_fout;
    bool do_children; //true for module (first node applied), false for next level. Lower levels not done
    bool do_writing; // default is true. Set on way down to false if nesisary

    source_moduleExpresion_visitor(Csource_out_ptr source_fout) :
        source_fout(source_fout)
    {
        do_children = true;
        do_writing = true;
    }

    source_moduleExpresion_visitor(bool _do_children, Csource_out_ptr _source_fout) :
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
        source_fout->out_strm() << source_fout->ln_strt << "void " << module->module_name << "__init__(void){" << endl;
        source_fout->enter_scope();
    }

    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children) override
    {
    // we do NOT call destructors, as that would destroy all global memory before program even starts.
    // need to make function to call on program exit.

        /// destructors ///
        // annoying we have to write this twice?
        // this skims all top-level nodes, looking variables
//        class destructuble_finder : public AST_visitor_NoChildren
//        {
//        public:
//            list< varName_ptr > names_to_destruct;
//
//            void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt)
//            {
//                names_to_destruct.push_back( autoStmt->variable_symbol );
//            }
//
//            void definitionStmt_down(definition_statement_AST_node* defStmt)
//            {
//                names_to_destruct.push_back( defStmt->variable_symbol );
//            }
//
//            void definitionNconstructionStmt_down(definitionNconstruction_statement_AST_node* defStmt)
//            {
//                names_to_destruct.push_back( defStmt->variable_symbol );
//            }
//
//        };
//
//        // apply it
//        destructuble_finder finder;
//
//        for( auto &AST_node : module->module_contents )
//        {
//            AST_node->apply_visitor( &finder );
//        }
//
//        // now destruct it all!
//        for( auto var_to_destruct : finder.names_to_destruct )
//        {
//            var_to_destruct->var_type->write_destructor( var_to_destruct->C_name, source_fout );
//        }


        /// END THE FUNCTION
        source_fout->out_strm() << source_fout->ln_strt << "}" << endl;
        source_fout->leave_scope();
    }

//    void definitionStmt_down(definition_statement_AST_node* defStmt) override
//    {
//        defStmt->var_type->resolved_type->write_default_constructor(defStmt->variable_symbol->C_name, source_fout);
//
//        do_writing = false; // DO NOT WRITE DEFS!
//    }
//
//    void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt) override
//    {
//        source_expression_visitor expr_vistr( source_fout );
//        autoStmt->expression->apply_visitor( &expr_vistr );
//
//        autoStmt->variable_symbol->var_type->write_assignment( autoStmt->expression->expression_return_type.get(),
//                            autoStmt->variable_symbol->C_name, expr_vistr.C_expression_code, source_fout );
//        source_fout << endl;
//
//        do_writing = false; // 'cause its weird!!
//    }

    void statement_down(statement_AST_node* statment) override
    {
        if(do_writing)
        {
            source_statement_visitor statement_writer( source_fout, false );
            statment->apply_visitor( &statement_writer );
        }
    }
};



// __init__ methods need to search for which class members are constructed
// this horid thing "helps" with that search
// this is one of the worst peices of code I've written......  and I've written some doozies!
class member_constructor_finder : public AST_visitorTree
{
public:
    // first we have a list of construct_names, which we accumulate accross this visitor tree
    list< utf8_string > constructed_names;
    utf8_string search_name;

    member_constructor_finder() :
        search_name( "self" )
        {
        }

    member_constructor_finder(utf8_string& _search_name) :
        search_name( _search_name )
        {
        }



    shared_ptr< AST_visitor_base > make_child(int number) override
    {
        return make_shared<member_constructor_finder>( );
    }

    void ASTnode_up(AST_node* ASTnode)
    {
        for( auto child : children)
        {
            auto cast_child = dynamic_pointer_cast<member_constructor_finder>( child );

            for(auto &new_construct_name : cast_child->constructed_names)
            {
                constructed_names.emplace_back(new_construct_name);
            }
        }
    }


    // if we find a constructElement, we check if it matches the right structor, then append name to the constructed_names list
    void constructElement_down(constructElement_AST_node* constructBlock)
    {
        // this checks the first expression is an "accessor" expression of a "self" variable
        class helper_one : public AST_visitor_NoChildren
        {
        public:
            utf8_string member_name;
            bool is_good;
            utf8_string& search_name;

            helper_one(utf8_string& _search_name) :
                search_name( _search_name )
            {
                is_good=false;
            }

            void accessorExp_down(accessor_expression_AST_node* accessorExp)
            {
                // this checks the inner bit is a var name of type "self"
                class helper_two : public AST_visitor_NoChildren
                {
                public:
                    bool is_good;
                    utf8_string& search_name;

                    helper_two(utf8_string& _search_name) :
                        search_name( _search_name )
                    {
                        is_good=false;
                    }

                     void varReferance_down(varReferance_expression_AST_node* varRefExp)
                     {
                        is_good = (varRefExp->var_name == search_name);
                     }
                };

                helper_two checkerA( search_name );
                accessorExp->expression->apply_visitor( &checkerA );

                if( checkerA.is_good )
                {
                    is_good = true;
                    member_name = accessorExp->name;
                }
            }
        };

        helper_one checkerB( search_name );
        constructBlock->expression->apply_visitor( &checkerB );

        if(checkerB.is_good)
        {
            constructed_names.emplace_back( checkerB.member_name );
        }
    }

};


class source_function_visitor : public AST_visitorTree
{
// looks for functions and methods! and writes their source using the above expression writer.
// probably needs to be VERY recursive due to nested functions (not presently implemented due to scoping problems)

public:
    Csource_out_ptr source_fout;

    source_function_visitor(Csource_out_ptr _source_fout) :
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
        source_fout->out_strm() << endl;
        source_fout->out_strm() << source_fout->ln_strt << "{" << endl;
        source_fout->enter_scope();

        /// invoke required params of inform_moved ///
        if( funcDef->paramList->required_list  )
        {
            for( auto& param : funcDef->paramList->required_list->param_list )
            {
                //param.var_type_ASTnode->resolved_type->inform_moved(param.variable_symbol->C_name,  source_fout);
                param.variable_symbol->var_type->inform_moved(param.variable_symbol->C_name,  source_fout);
                source_fout->out_strm() << endl;
            }
        }

        /// write stuff for default params here!!!   include inform moved!
        if( funcDef->paramList->defaulted_list )
        {
            auto default_params = funcDef->paramList->defaulted_list;
            int num_params = default_params->param_list.size();

            auto param_name_iter = default_params->param_list.begin();
            auto default_exp_iter =  default_params->parameter_defaults.begin();
            for( int i=0; i<num_params; i++ )
            {
                source_fout->out_strm() << source_fout->ln_strt << "if("<< param_name_iter->variable_symbol->definition_name<<"__use_default__"<<"){"<<endl;
                source_fout->enter_scope();

                // LHS
                auto LHS_var_type = param_name_iter->variable_symbol->var_type;
                auto var_name = param_name_iter->variable_symbol->C_name;
                auto LHS_var_Cexp = make_shared<simple_C_expression>( var_name, LHS_var_type, true, false );

                // defaulted RHS expr
                source_expression_visitor expr_vistr( source_fout );
                auto default_exp = *default_exp_iter;
                default_exp->apply_visitor( &expr_vistr );

                auto C_exp = expr_vistr.final_expression;
                auto exp_type = C_exp->cyth_type;

                // do construction!!
                if( LHS_var_type->has_implicit_copy_constructor( exp_type ) )
                {
                    LHS_var_type->write_implicit_copy_constructor( LHS_var_Cexp, C_exp, source_fout);
                }
                else if( exp_type->can_implicit_castTo(LHS_var_type) )
                {
                    exp_type->write_implicit_castTo( LHS_var_Cexp, C_exp, source_fout );
                }

                // cleanup
                LHS_var_Cexp->write_cleanup( source_fout );

                // or maybe just inform moved
                source_fout->leave_scope();
                source_fout->out_strm() << source_fout->ln_strt << "}"<<endl<< "else{";
                source_fout->enter_scope();

                LHS_var_type->inform_moved(var_name,  source_fout);

                source_fout->leave_scope();
                source_fout->out_strm() << source_fout->ln_strt << "}"<<endl;

                ++param_name_iter;
                ++default_exp_iter;
            }
        }

        // write block of statements
        source_statement_visitor statement_writer( source_fout );
        funcDef->block_AST->apply_visitor( &statement_writer );

// DESTRUCT LOCAL VARIABLES!?!?!?

        source_fout->leave_scope();
        source_fout->out_strm() << source_fout->ln_strt << "}" << endl << endl;
    }

    void methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child)
    {
        auto self_var = methodDef->funcType->self_ptr_name;

        methodDef->specific_overload->write_C_prototype( source_fout );
        source_fout->out_strm() << endl;
        source_fout->out_strm() << source_fout->ln_strt << "{" << endl;
        source_fout->enter_scope();

        /// write self pointer cast
        self_var->var_type->C_definition_name(self_var->C_name, source_fout);
        source_fout->out_strm() << " = (" ;
        utf8_string TMP("");
        self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
        source_fout->out_strm() << ")" << self_var->C_name << "_;" << endl;

        /// invoke required params of inform_moved ///
        if( methodDef->paramList->required_list  )
        {
            for( auto& param : methodDef->paramList->required_list->param_list )
            {
                param.variable_symbol->var_type->inform_moved(param.variable_symbol->C_name,  source_fout);
                source_fout->out_strm() << endl;
            }
        }

        /// write stuff for default params here!!!
        if( methodDef->paramList->defaulted_list )
        {
            auto default_params = methodDef->paramList->defaulted_list;
            int num_params = default_params->param_list.size();

            auto param_name_iter = default_params->param_list.begin();
            auto default_exp_iter =  default_params->parameter_defaults.begin();
            for( int i=0; i<num_params; i++ )
            {
                source_fout->out_strm() << source_fout->ln_strt << "if("<< param_name_iter->variable_symbol->definition_name<<"__use_default__"<<"){"<<endl;
                source_fout->enter_scope();

                // LHS
                auto LHS_var_type = param_name_iter->variable_symbol->var_type;
                auto var_name = param_name_iter->variable_symbol->C_name;
                auto LHS_var_Cexp = make_shared<simple_C_expression>( var_name, LHS_var_type, true, false );

                // defaulted RHS expr
                source_expression_visitor expr_vistr( source_fout );
                auto default_exp = *default_exp_iter;
                default_exp->apply_visitor( &expr_vistr );

                auto default_C_exp = expr_vistr.final_expression;
                auto exp_type = default_C_exp->cyth_type;

                // do construction!!
                if( LHS_var_type->has_implicit_copy_constructor( exp_type ) )
                {
                    LHS_var_type->write_implicit_copy_constructor( LHS_var_Cexp, default_C_exp, source_fout);
                }
                else if( exp_type->can_implicit_castTo(LHS_var_type) )
                {
                    exp_type->write_implicit_castTo( LHS_var_Cexp, default_C_exp, source_fout );
                }

                // cleanup
                LHS_var_Cexp->write_cleanup( source_fout );

                // or maybe just inform moved
                source_fout->leave_scope();
                source_fout->out_strm() << source_fout->ln_strt << "}"<<endl<< "else{";
                source_fout->enter_scope();

                LHS_var_type->inform_moved(var_name,  source_fout);

                source_fout->leave_scope();
                source_fout->out_strm() << source_fout->ln_strt << "}"<<endl;

                ++param_name_iter;
                ++default_exp_iter;
            }
        }


        // special stuff for special methods
        // __init__ and __exInit__
        if( (methodDef->funcType->type_of_method == MethodType::constructor) or (methodDef->funcType->type_of_method == MethodType::explicit_constructor) )
        {
            // step one, invoke the devil
            member_constructor_finder find_constructed_members;
            methodDef->block_AST->apply_visitor( &find_constructed_members );

            //step two, recover the "self" symbol table
            auto class_type = dynamic_pointer_cast<DefClassType>( methodDef->class_type );
            auto class_sym_table = class_type->class_symbol_table;

            //step three, search through all data members
            //   if not constructed, write default cosntructor here
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                // check if constructed
                bool is_constructed = false;
                for( auto &var_name : find_constructed_members.constructed_names )
                {
                    if( var_name == varName_ptr_pair.first )
                    {
                        is_constructed = true;
                        break;
                    }
                }

                // it not, construct it!
                if(not is_constructed)
                {
                    utf8_string var_name( varName_ptr_pair.first );
                    auto self_Cexp = make_shared<simple_C_expression>( self_var->C_name, self_var->var_type, true, false );
                    auto getter_Cexp = self_var->var_type->write_member_getref(self_Cexp ,var_name, source_fout);

                    auto var_type = varName_ptr_pair.second->var_type;
                    auto C_exp_str = getter_Cexp->get_C_expression();
                    var_type->write_default_constructor( C_exp_str, source_fout );

                    self_Cexp->write_cleanup(source_fout);
                    getter_Cexp->write_cleanup(source_fout);
                }
             }
        } // make sense?

        // __convert__ and __exConvert__
        if( (methodDef->funcType->type_of_method == MethodType::conversion) or (methodDef->funcType->type_of_method == MethodType::explicit_conversion) )
        {
            // check members need reconstructed.
            auto parameter_name = methodDef->specific_overload->parameters->required_parameters.front();
            auto parameter_type = parameter_name->var_type;

            if( parameter_type->type_of_type == varType::defined_class_t )
            {
                 // step one, invoke the devil
                member_constructor_finder find_constructed_members( parameter_name->definition_name );
                methodDef->block_AST->apply_visitor( &find_constructed_members );

                //step two, recover the "self" symbol table
                auto class_type = dynamic_pointer_cast<DefClassType>( parameter_type );
                auto class_sym_table = class_type->class_symbol_table;

                //step three, search through all data members
                //   if not constructed, write default cosntructor here
                for( auto &varName_ptr_pair : class_sym_table->variable_table )
                {
                    // check if constructed
                    bool is_constructed = false;
                    for( auto &var_name : find_constructed_members.constructed_names )
                    {
                        if( var_name == varName_ptr_pair.first )
                        {
                            is_constructed = true;
                            break;
                        }
                    }

                    // it not, construct it!
                    if(not is_constructed)
                    {
                        utf8_string var_name( varName_ptr_pair.first );
            // IS THIS CORRECT???
                        auto self_Cexp = make_shared<simple_C_expression>( parameter_name->C_name, parameter_name->var_type, true, false );
                        auto getter_Cexp = self_var->var_type->write_member_getref(self_Cexp, var_name, source_fout);

                        auto var_type = varName_ptr_pair.second->var_type;
                        auto C_exp_str = getter_Cexp->get_C_expression();
                        var_type->write_default_constructor( C_exp_str, source_fout );

                        self_Cexp->write_cleanup(source_fout);
                        getter_Cexp->write_cleanup(source_fout);
                    }
                 }
            }
        }

        // write block of statements
        source_statement_visitor statement_writer( source_fout );
        methodDef->block_AST->apply_visitor( &statement_writer );

// TODO: destruct local variables??


        // __del__
        if( methodDef->funcType->type_of_method == MethodType::destructor )
        {
            //auto parameter_name = methodDef->specific_overload->parameters->required_parameters.front();

            auto class_type = dynamic_pointer_cast<DefClassType>( methodDef->class_type );
            auto class_sym_table = class_type->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                utf8_string var_name( varName_ptr_pair.first );
                auto self_Cexp = make_shared<simple_C_expression>( self_var->C_name, self_var->var_type, true, false );
                auto getter_Cexp = self_var->var_type->write_member_getref(self_Cexp, var_name, source_fout);

                auto var_type = varName_ptr_pair.second->var_type;
                auto TMP = getter_Cexp->get_C_expression();
                var_type->write_destructor( TMP, source_fout );

                getter_Cexp->write_cleanup( source_fout );
            }
        }

        source_fout->leave_scope();
        source_fout->out_strm() << source_fout->ln_strt << "}" << endl << endl;
    }


    void ClassDef_up( class_AST_node* clss_node, list<AST_visitor_base*>& var_def_children,
                     list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child )
    {
        /// need to write out the defaulted methods ///
        auto self_var = clss_node->self_name;
        auto class_type = clss_node->type_ptr;

        // defaulted constructor
        if( clss_node->write_default_constructor )
        {
            clss_node->default_constructor_overload->write_C_prototype( source_fout );
            source_fout->out_strm() << endl;
            source_fout->out_strm() << source_fout->ln_strt << "{" << endl;
            source_fout->enter_scope();


            /// write self pointer cast
            self_var->var_type->C_definition_name(self_var->C_name, source_fout);
            source_fout->out_strm() << " = (" ;
            utf8_string TMP("");
            self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
            source_fout->out_strm() << ")" << self_var->C_name << "_;" << endl;

            /// then initiate the members
            auto class_sym_table = class_type->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                if( varName_ptr_pair.first == "self" )
                {   continue;  }


                utf8_string var_name( varName_ptr_pair.first );
                auto self_Cexp = make_shared<simple_C_expression>( self_var->C_name, self_var->var_type, true, false );
                auto getter_Cexp = self_var->var_type->write_member_getref(self_Cexp, var_name, source_fout);

                auto var_type = varName_ptr_pair.second->var_type;
                auto C_exp_str = getter_Cexp->get_C_expression();
                var_type->write_default_constructor( C_exp_str, source_fout );

                self_Cexp->write_cleanup(source_fout);
                getter_Cexp->write_cleanup(source_fout);

            }
            source_fout->leave_scope();
            source_fout->out_strm() << source_fout->ln_strt << "}" << endl << endl;
        }

        // defaulted destructor
        if( clss_node->write_default_deconstructor )
        {
            clss_node->default_destructor_overload->write_C_prototype( source_fout );
            source_fout->out_strm() << endl;
            source_fout->out_strm() << source_fout->ln_strt << "{" << endl;
            source_fout->enter_scope();


            /// write self pointer cast
            self_var->var_type->C_definition_name(self_var->C_name, source_fout);
            source_fout->out_strm() << " = (" ;
            utf8_string TMP("");
            self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
            source_fout->out_strm() << ")" << self_var->C_name << "_;" << endl;

            /// and destruct all the other things
            auto class_sym_table = class_type->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                if( varName_ptr_pair.first == "self" )
                {   continue;  }

                utf8_string var_name( varName_ptr_pair.first );
                auto self_Cexp = make_shared<simple_C_expression>( self_var->C_name, self_var->var_type, true, false );
                auto getter_Cexp = self_var->var_type->write_member_getref(self_Cexp, var_name, source_fout);

                auto var_type = varName_ptr_pair.second->var_type;
                auto TMP = getter_Cexp->get_C_expression();
                var_type->write_destructor( TMP, source_fout );

                getter_Cexp->write_cleanup( source_fout );
            }


            source_fout->leave_scope();
            source_fout->out_strm() << source_fout->ln_strt << "}" << endl << endl;
        }

        //defaulted copy constructor
        if( clss_node->write_selfCopy_constructor )
        {
            clss_node->default_CopyConstructor_overload->write_C_prototype( source_fout );
            source_fout->out_strm() << endl;
            source_fout->out_strm() << source_fout->ln_strt << "{" << endl;
            source_fout->enter_scope();


            /// write self pointer cast
            self_var->var_type->C_definition_name(self_var->C_name, source_fout);
            source_fout->out_strm() << " = (" ;
            utf8_string TMP("");
            self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
            source_fout->out_strm() << ")" << self_var->C_name << "_;" << endl;


            /// inform RHS moved /// I HOPE THIS IS RIGHT!
            utf8_string RHS_name("__cy__arg_RHS");
            class_type->inform_moved(RHS_name,  source_fout);
            source_fout->out_strm() << endl;


            /// copy other variables.
            auto class_sym_table = class_type->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                if( varName_ptr_pair.first == "self" )
                {   continue;  }


                auto TYPE = varName_ptr_pair.second->var_type;
                if( TYPE->has_implicit_copy_constructor( TYPE ))
                {
                    utf8_string var_name( varName_ptr_pair.first );

                    // LHS get ref
                    auto self_Cexp = make_shared<simple_C_expression>( self_var->C_name, self_var->var_type, true, false );
                    self_Cexp->can_be_referenced = true;
                    auto LHS_getter_Cexp = self_var->var_type->write_member_getref(self_Cexp, var_name, source_fout);

                    // RHS
                    auto RHS_Cexp = make_shared<simple_C_expression>( "__cy__arg_RHS", self_var->var_type, true, false  );
                    RHS_Cexp->can_be_referenced = true;
                    auto RHS_getter_Cexp = self_var->var_type->write_member_getter(RHS_Cexp, var_name, source_fout);

                    // write the copy constructor!
                    TYPE->write_implicit_copy_constructor( LHS_getter_Cexp, RHS_getter_Cexp, source_fout);

                    // cleanup
                    self_Cexp->write_cleanup( source_fout );
                    LHS_getter_Cexp->write_cleanup( source_fout );
                    RHS_Cexp->write_cleanup( source_fout );
                    RHS_getter_Cexp->write_cleanup( source_fout );

//                    utf8_string name = varName_ptr_pair.first ;
//                    auto Cexp = self_var->var_type->write_member_getref( self_var->C_name,  name, source_fout  );
//                    auto Cexp_str = Cexp->get_C_expression();
//
//                    utf8_string RHS = "(__cy__arg_RHS->"+varName_ptr_pair.second->C_name;
//                    RHS += ")"; // I hope this works!
//
//                    varReferance_expression_AST_node RHS_ast(name, clss_node->loc );
//                    RHS_ast.variable_symbol = varName_ptr_pair.second;
//                    RHS_ast.expression_return_type = TYPE;
//                    RHS_ast.symbol_table = clss_node->symbol_table;
//
//                    TYPE->write_implicit_copy_constructor(TYPE.get(), &RHS_ast,
//                                Cexp_str, RHS, source_fout);
//                    source_fout << endl;
//                    Cexp->write_cleanup();
                }
            }



            source_fout->leave_scope();
            source_fout->out_strm() << "}" << endl << endl;
        }


        // defaulted assignment operators //
        for(auto &overload_varName_pair : clss_node->assignments_to_default)
        {
            auto overload = overload_varName_pair.first;
            auto RHS_var = overload_varName_pair.second;
            bool tmp = false;
            auto RHS_type_refed = RHS_var->var_type->is_reference_type( tmp );
            if( not tmp or not RHS_type_refed )
            {
                throw gen_exception("error 1 in ClassDef_up. This should not be reached." );
            }

            overload->write_C_prototype( source_fout );
            source_fout->out_strm() << endl;
            source_fout->out_strm() << source_fout->ln_strt << "{" << endl;
            source_fout->enter_scope();


            /// write self pointer cast
            self_var->var_type->C_definition_name(self_var->C_name, source_fout);
            source_fout->out_strm() << " = (" ;
            utf8_string TMP("");
            self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
            source_fout->out_strm() << ")" << self_var->C_name << "_;" << endl;

            /// inform RHS moved /// I HOPE THIS IS RIGHT!
            class_type->inform_moved(RHS_var->C_name,  source_fout);
            source_fout->out_strm() << endl;


            /// destruct-in-place
   // AM HERE. THIS IS NOT RIGHT?

            utf8_string derefed_self_Cname = "*"+self_var->var_type->get_pointer(class_type, self_var->C_name, source_fout);
            class_type->write_destructor(derefed_self_Cname, source_fout, true );

            //utf8_string derefed_self_Cname = "(*(" + self_var->C_name + "))"; // I hope this is right
            //class_type->write_destructor(derefed_self_Cname, source_fout, true );


            /// now reconstruct
            //utf8_string derefed_RHS_Cname = "(*(" + RHS_var->C_name + "))"; // I hope this is right TOOO!!!

//            if( class_type->has_implicit_copy_constructor( RHS_type_refed.get() ) )
//            {
//                auto RHS_node = make_shared<varReferance_expression_AST_node>( derefed_RHS_Cname, clss_node->loc );
//                RHS_node->has_output_ownership = false;
//                RHS_node->c_exp_can_be_referenced = true;
//                RHS_node->variable_symbol = make_shared<varName>();
//                RHS_node->variable_symbol->var_type = RHS_type_refed->shared_from_this();
//                RHS_node->variable_symbol->C_name = derefed_RHS_Cname;
//                RHS_node->variable_symbol->loc = clss_node->loc ;
//                RHS_node->variable_symbol->is_ordered = false;
//                RHS_node->expression_return_type = RHS_node->variable_symbol->var_type;
//                RHS_node->symbol_table = clss_node->inner_symbol_table.get();
//
//                class_type->write_implicit_copy_constructor( RHS_type_refed.get(), RHS_node.get(),  derefed_self_Cname,
//                                     derefed_RHS_Cname, source_fout );
//            }

            if( class_type->has_implicit_copy_constructor( RHS_var->var_type ) )
            {
                auto LHS_exp = make_shared< simple_C_expression >(  derefed_self_Cname, class_type, true, false);

                auto RHS_exp = make_shared< simple_C_expression >(  RHS_var->C_name, RHS_var->var_type, true, false );

                class_type->write_implicit_copy_constructor( LHS_exp, RHS_exp, source_fout);
            }
            else
            {
                throw gen_exception("error 2 in ClassDef_up. No copy constructor. This should not be reached." );
            }


            source_fout->leave_scope();
            source_fout->out_strm() << source_fout->ln_strt << "}" << endl << endl;
        }
    }
};

//// the main functions! ////

void write_module_to_C(module_AST_ptr module)
{
    //// first we write the header ///
    string header_fname = module->C_header_fname;
    string source_fname = module->C_source_fname;

    //ofstream C_header_file(header_fname);
    auto C_header_file = make_shared<output_Csource_file>( header_fname );

//header_fname = "DELME_" + header_fname;
//source_fname = "DELME_" + source_fname;

    // imports //
    header_writeImports_visitor import_writer(C_header_file, header_fname);
    module->apply_visitor( &import_writer );

    // definitions //
    header_writeDefinitions_visitor definitions_writer(C_header_file, header_fname);
    module->apply_visitor( &definitions_writer );

    //// now we write the source files ////
    //ofstream C_source_file(source_fname);
    auto C_source_file = make_shared<output_Csource_file>( source_fname );

    // preamble, anything that needs to come before main body
    source_writePreamble_visitor preamble_writer(C_source_file, header_fname);
    module->apply_visitor( &preamble_writer );

    // main body
    // functions
    source_function_visitor function_writer(C_source_file);
    module->apply_visitor( &function_writer );

    // module setup (top level expressions, call other module setups)
    source_moduleExpresion_visitor initilizer_writer(C_source_file);
    module->apply_visitor( &initilizer_writer );
}




bool recursive_init_writer(module_AST_ptr module, module_manager* mod_manager, list<string>& names_done, Csource_out_ptr output)
{
    names_done.emplace_back( module->module_name );

    for( auto& mod_name : module->imported_cyth_modules )
    {

        auto new_module = mod_manager->get_module( mod_name );
        bool do_module = find( names_done.begin(), names_done.end(), new_module-> module_name) == names_done.end();
        if( do_module )
        {

            if( not new_module)
            {
                cout << "ERROR IN WRITING MAIN: CANNOT FIND MODULE " << mod_name << " this should not be reached." << endl;
                return false;
            }

            bool goodness = recursive_init_writer(new_module,  mod_manager,  names_done, output);
            if( not goodness)
            {
                return false;
            }
        }
    }

    output->out_strm() << output->ln_strt << module->module_name << "__init__();" << endl;

    return true;
}

bool write_mainFunc_to_C(module_AST_ptr module, module_manager* mod_manager)
{
    string source_main_fname( module->C_source_fname + "_main.c" );
    module->C_mainSource_fname = source_main_fname;


    auto C_source_main_file = make_shared<output_Csource_file>( source_main_fname );

    C_source_main_file->out_strm() << C_source_main_file->ln_strt << "#include \"" << module->C_header_fname << "\"" <<endl;
    C_source_main_file->out_strm() << C_source_main_file->ln_strt << "void main(void){" <<endl;


    // write prep functions for all other modules here!!
    list<string> names_done;
    bool we_good = recursive_init_writer( module, mod_manager, names_done, C_source_main_file );


    C_source_main_file->out_strm() << C_source_main_file->ln_strt << module->main_func_name << "();" << endl;
    C_source_main_file->out_strm() << C_source_main_file->ln_strt << '}' << endl;

    return we_good;
}

