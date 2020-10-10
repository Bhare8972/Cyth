
#include "writeAST_to_C.hpp"
#include "AST_visitor.hpp"
#include <random>
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


    virtual void CythImports_down(import_cyth_AST_node* ASTnode) override
    {
        if( ASTnode->import_module_Cheader_fname.get_length() != 0 )
        {
            header_fout << "#include \"" << ASTnode->import_module_Cheader_fname << "\""<<endl;
        }
    }

    void ClassDef_down( class_AST_node* class_node)
    {
        //do_children = true; // so we can handle the structures and methods inside a structure
        // ??

        header_fout << "struct "<< class_node->type_ptr->C_name << ";" << endl;
    }

};


// for defining the data members inside classes
// mostly for variables. doesn't handle methods or other classes ...
class Class_DataDefiner : public AST_visitorTree
{
public:
    ofstream& fout;
    bool do_children;

    Class_DataDefiner(ofstream& _fout, bool chillins=true) :
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

    void definitionNconstructionStmt_down(definitionNconstruction_statement_AST_node* defStmt) override
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
    void ClassDef_down( class_AST_node* class_node) override
    {
        do_children = true; // so we can handle the structures and methods inside a structure
        // anouther, more correct approach, would be to use anouther visiter
        auto class_type = class_node->type_ptr;
        //auto class_symbols = class_type->class_symbol_table;


    // first we write ancilary structs/classes //
        // starting with the vtable //
        header_fout << "struct "<< class_type->vtableType_cname << "{" << endl;

        // Write all virtual methods that don't override and are defined in this class
        DefClassType::methodOverloadIter methoditer = class_type->methodOverloadIter_begin( false );
        DefClassType::methodOverloadIter methoditer_end = class_type->methodOverloadIter_end();
        for( ; methoditer!=methoditer_end; ++methoditer )
        {
            auto overload = methoditer.overload_get();

            if( overload->is_virtual and overload->overriden_method==nullptr )// virtual and does not overload
            {
                // first, define the offset
                header_fout << "long " << overload->c_reference << "_offset;" << endl;

                // then the function pointer
                stringstream OUT;
                OUT << "(*" <<  overload->c_reference << ")(";

                //overload->self_ptr_name->var_type->C_definition_name(overload->self_ptr_name->C_name, OUT);
                OUT << "void* " << overload->self_ptr_name->C_name << "_";
                if( overload->parameters->total_size() > 0 )
                {
                    OUT << ",";
                }

                overload->parameters->write_to_C(OUT, false);
                OUT << ")";

                utf8_string name = OUT.str();
                overload->return_type->C_definition_name(name, header_fout);
                header_fout<<';'<<endl;

            }
        }

        header_fout << endl;
        header_fout << "};" << endl;




    // define the global vtable variable
        header_fout << "extern struct " << class_type->vtableType_cname << " " <<  class_type->global_vtableVar_cname << ";" << endl;
    // and parent tables
        for( unsigned int parent_i=0;  parent_i< (class_type->full_inheritance_tree.size()); ++parent_i)
        {
            auto parent_type = class_type->full_inheritance_tree[ parent_i ];
            auto vtable_name = class_type->global_parentVtable_cnames[ parent_i ];
            header_fout << "extern struct " << parent_type->vtableType_cname << " " <<  vtable_name << ";" << endl;
        }




    // now we define the actual class struct

        header_fout << "struct "<< class_type->C_name << "{" << endl;

        //write vtable
        header_fout << "struct " << class_type->vtableType_cname << " *__cy_vtable;" << endl;

        // write parents
        for( auto &parent_name : class_type->parent_class_names)
        {
            parent_name->var_type->C_definition_name(parent_name->C_name, header_fout);
            header_fout<<';'<<endl;
        }

        //write members
        Class_DataDefiner internal_visitor(header_fout);
        class_node->apply_visitor( &internal_visitor );


        header_fout << endl;
        header_fout << "};" << endl;





    // write automated functions
        // write default constructor?
        if(class_node->write_default_constructor)
        {
            class_node->default_constructor_overload->write_C_prototype( header_fout );
            header_fout << ';'<<endl;
        }
        // write default destructor?
        if(class_node->write_default_deconstructor)
        {
            class_node->default_destructor_overload->write_C_prototype( header_fout );
            header_fout << ';'<<endl;
        }
        // default copy constructor?
        if( class_node->write_selfCopy_constructor )
        {
            class_node->default_CopyConstructor_overload->write_C_prototype( header_fout );
            header_fout << ';'<<endl;
        }

    }

    void methodDef_down(method_AST_node* methodDef) override
    {
        methodDef->specific_overload->write_C_prototype( header_fout );
        header_fout << ';'<<endl;
    }

};



//// SOURCE WRITERS ////
// writes the preamble. This includes import of header, simple functions, global variables, etc...
// initializes the vtables
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
    }

    void typed_VarDef_down(Typed_VarDefinition* vardef) override // hope this works??
    {
        vardef->var_type->resolved_type->C_definition_name( vardef->variable_symbol->C_name,  source_fout  );
        source_fout<< ";" << endl << endl;
    }

    void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt) override
    {
        autoStmt->variable_symbol->var_type->C_definition_name( autoStmt->variable_symbol->C_name, source_fout );
        source_fout << ";" << endl << endl;
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
            source_fout << "struct "<< parent_class_type->vtableType_cname << " " << vtable_name << " = {" << endl;

            // now we loop over every vtabled method and its override
            for( auto &connection_pair : connection_map )
            {
                auto &parent_method = connection_pair.second.methodOverload_to_override;
                auto &overriding_method = connection_pair.second.methodOverload_that_overrides;
                auto &overriding_class = connection_pair.second.class_that_overrides;

                // there are three options for the offset

                utf8_string offset("0");
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
                        utf8_string TMP;
                        stringstream defClass_out;
                        child_class->C_definition_name(TMP, defClass_out);
                        offset = offset + "+offsetof(" + defClass_out.str() + ", " + parent_varname->C_name + ")";
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

                        offset = offset + "+offsetof(" + child_class->C_name + ", " + previous_parent->C_name + ")";

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

                        offset = offset + "-offsetof(" + child_class->C_name + ", " + previous_parent->C_name + ")";

                        previous_parent = child_class;
                    }


                }
                // write to the file
                source_fout << " ." << parent_method->c_reference << "_offset =" << offset << "," <<endl;
                source_fout << " ." << parent_method->c_reference << " =" <<  overriding_method->c_reference << ","<< endl;
            }
            source_fout << "};" << endl;
        }


        // now we set our own vtable
        source_fout << "struct "<< class_node->type_ptr->vtableType_cname << " " << class_node->type_ptr->global_vtableVar_cname << " = {" << endl;

        auto methoditer = class_type->methodOverloadIter_begin( false );
        auto methoditer_end = class_type->methodOverloadIter_end();
        for( ; methoditer!=methoditer_end; ++methoditer )
        {
            auto overload = methoditer.overload_get();
            if( overload->is_virtual and overload->overriden_method==nullptr) // IE, is part of this vtable
            {
                source_fout << " ." << overload->c_reference << "_offset = 0," <<endl;
                source_fout << " ." << overload->c_reference << " =" <<  overload->c_reference << ","<< endl;
            }
        }

        source_fout << "};" << endl;
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
    varref->writer = make_shared<simple_expression_writer>( varref->variable_symbol->C_name );

}

void source_LHSreference_visitor::source_LHS_child::LHS_accessor_up(LHS_accessor_AST_node* LHSaccess, AST_visitor_base* LHSref_visitor)
{
    //auto LHS_child = dynamic_cast<source_LHS_child*>( LHSref_visitor );
    //utf8_string& child_LHS_Cref = LHS_child->LHS_C_code;

    //LHS_C_code = LHSaccess->reference_type->write_member_getref( child_LHS_Cref, LHSaccess->name, source_fout );


    auto child_LHS_Cref = LHSaccess->LHS_exp->writer->get_C_expression();
    LHSaccess->writer = LHSaccess->reference_type->write_member_getref( child_LHS_Cref, LHSaccess->name, source_fout );
}


source_LHSreference_visitor::source_LHSreference_visitor(ofstream& _source_fout, expression_AST_node* _RHS_AST_node, utf8_string& RHS_exp, varType_ptr _RHS_type) :
    source_fout(_source_fout)
{
    RHS_C_code = RHS_exp;
    RHS_type = _RHS_type;
    RHS_AST_node = _RHS_AST_node;
}

// note this will NEVER call 'up' visitors!

void source_LHSreference_visitor::LHS_varRef_down(LHS_varReference* varref)
{
    // note we are still assuming that var refs are always simple. This may change.
    source_LHS_child LHS( source_fout );
    varref->apply_visitor( &LHS );

    if( varref->reference_type->get_has_assignment(RHS_type.get())  )
    {
        //varref->reference_type->write_assignment( RHS_type.get(), RHS_AST_node, LHS.LHS_C_code , RHS_C_code, source_fout );
        auto Cexp = varref->writer->get_C_expression();
        varref->reference_type->write_assignment( RHS_type.get(), RHS_AST_node, Cexp , RHS_C_code, source_fout );
    }
    else if( RHS_type->get_has_assignTo( varref->reference_type.get() ) )
    {
        auto Cexp = varref->writer->get_C_expression();
        RHS_type->write_assignTo(varref->reference_type.get(),  RHS_AST_node, Cexp , RHS_C_code, source_fout );
    }
    else // trust!
    {
        throw gen_exception( "error in source_LHSreference_visitor::LHS_varRef_down. This should never be reached" );
        //auto Cexp = varref->writer->get_C_expression();
        //RHS_type->write_implicit_castTo( varref->reference_type.get(),  RHS_AST_node, Cexp , RHS_C_code, source_fout );
    }
}

void source_LHSreference_visitor::LHS_accessor_down(LHS_accessor_AST_node* LHSaccess)
{
    source_LHS_child LHS( source_fout );
    auto LHS_EXP = LHSaccess->LHS_exp;
    LHS_EXP->apply_visitor( &LHS );

    // note we need the type of the child, not the final return type!!!
    auto child_type = LHSaccess->LHS_exp->reference_type;
    auto C_exp = LHS_EXP->writer->get_C_expression();
    child_type->write_member_setter(RHS_AST_node,  C_exp, LHSaccess->name, RHS_type.get(), RHS_C_code, source_fout );
}

/// LHS expression_cleanup_visitor ///
void LHSexpression_cleanup_visitor::LHSReference_down(LHS_reference_AST_node* LHS_ref)
{
    if( LHS_ref->writer )
    {
        LHS_ref->writer->write_cleanup();
    }
}





/// source_expression_visitor ///

shared_ptr< AST_visitor_base > source_expression_visitor::make_child(int number)
{
    return make_shared<source_expression_visitor>( source_fout );
}

void source_expression_visitor::intLiteral_up(intLiteral_expression_AST_node* intLitExp)
{
    stringstream exp;
    exp << intLitExp->literal;

    //intLitExp->C_exp = exp.str();
    intLitExp->writer = make_shared<simple_expression_writer>( exp.str() );
    intLitExp->c_exp_can_be_referenced = false;
}

void source_expression_visitor::binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
{
    // define addition
    //source_expression_visitor* LHS_visitor = dynamic_cast<source_expression_visitor*>(LHS_exp_visitor);
    //source_expression_visitor* RHS_visitor = dynamic_cast<source_expression_visitor*>(RHS_exp_visitor);
    auto LHS_ast = binOprExp->left_operand;
    auto RHS_ast = binOprExp->right_operand;


    auto LHS_Cexp = LHS_ast->writer->get_C_expression();
    auto RHS_Cexp = RHS_ast->writer->get_C_expression();

    if( binOprExp->expression_return_type->can_be_defined() )
    {
        utf8_string var_name = "__cy__expTMP_" + binOprExp->symbol_table->get_unique_string();

        binOprExp->expression_return_type->C_definition_name( var_name, source_fout );
        source_fout << ';' << endl;
        binOprExp->expression_return_type->initialize( var_name, source_fout );

        auto exp = LHS_ast->expression_return_type->write_LHSaddition(LHS_ast, LHS_Cexp, RHS_ast, RHS_Cexp,
                                                source_fout);

        source_fout << var_name << '=' << exp << ';' << endl;
        exp->write_cleanup();
// NOTE MOVE
        binOprExp->writer = make_shared<simple_expression_writer>( var_name );
        binOprExp->c_exp_can_be_referenced = true;
    }
    else
    {
        binOprExp->writer = LHS_ast->expression_return_type->write_LHSaddition(LHS_ast, LHS_Cexp, RHS_ast, RHS_Cexp,
                                                source_fout);
        binOprExp->c_exp_can_be_referenced = false;
    }

        //binOprExp->writer = LHS_ast->expression_return_type->write_LHSaddition(LHS_ast, LHS_Cexp, RHS_ast, RHS_Cexp,
                                                //source_fout);
        //binOprExp->c_exp_can_be_referenced = false;
}

void source_expression_visitor::varReferance_up(varReferance_expression_AST_node* varRefExp)
{
    varRefExp->writer = make_shared<simple_expression_writer>( varRefExp->variable_symbol->C_name );
    varRefExp->c_exp_can_be_referenced = true;
}

void source_expression_visitor::ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor)
{
    //source_expression_visitor* EXP_visitor_PTR = dynamic_cast<source_expression_visitor*>(expChild_visitor);
    //C_expression_code = EXP_visitor_PTR->C_expression_code;
    parenGroupExp->writer = make_shared<simple_expression_writer>( parenGroupExp->expression->writer->get_C_expression() );
    parenGroupExp->c_exp_can_be_referenced = parenGroupExp->expression->c_exp_can_be_referenced;
}

void source_expression_visitor::accessorExp_up(accessor_expression_AST_node* accessorExp, AST_visitor_base* expChild_visitor)
{
    auto T = accessorExp->expression->expression_return_type;
//    source_expression_visitor* EXP_visitor_PTR = dynamic_cast<source_expression_visitor*>(expChild_visitor);
//
    auto child_exp = accessorExp->expression->writer->get_C_expression();
    accessorExp->writer = T->write_member_getter( child_exp, accessorExp->name, source_fout);


    //accessorExp->writer = make_shared<simple_expression_writer>( C_expression_code );
    accessorExp->c_exp_can_be_referenced = true;
}

void source_expression_visitor::functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child)
{

    vector< utf8_string > argument_C_expressions;

    auto num_args = funcCall->argument_list->total_size();
    argument_C_expressions.reserve( num_args );
    for( int i=0; i<num_args; i++)
    {
        argument_C_expressions.push_back( funcCall->argument_list->expression_from_index( i )->writer->get_C_expression() );
    }


    if( funcCall->expression_return_type->can_be_defined() and funcCall->expression_return_type->type_of_type!=varType::empty ) //empty is because of the void type
    {
        auto var_name = "__cy__expTMP_" + funcCall->symbol_table->get_unique_string();
        funcCall->expression_return_type->C_definition_name( var_name, source_fout );
        source_fout << ';' << endl;
        funcCall->expression_return_type->initialize( var_name, source_fout );

        auto writer = funcCall->expression->expression_return_type->write_call( funcCall->argument_list.get(),
                           funcCall->expression->writer,  argument_C_expressions,  source_fout );

        source_fout << var_name << '=' << (writer->get_C_expression()) << ';' << endl;

// NOTE MOVE
        writer->write_cleanup();

        funcCall->writer = make_shared<simple_expression_writer>( var_name );
        funcCall->c_exp_can_be_referenced = true;
    }
    else
    {

    auto writer = funcCall->expression->expression_return_type->write_call( funcCall->argument_list.get(),
            funcCall->expression->writer,  argument_C_expressions,  source_fout );

    funcCall->writer = writer;
    funcCall->c_exp_can_be_referenced = false;

    }
}

/// expression_cleanup_visitor ///
void expression_cleanup_visitor::expression_down(expression_AST_node* expression)
{
    expression->writer->write_cleanup();
}

void expression_cleanup_visitor::binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
{
    if( binOprExp->has_output_ownership and  binOprExp->c_exp_can_be_referenced )
    {

        auto name = binOprExp->writer->get_C_expression();
        binOprExp->expression_return_type->write_destructor(name, source_fout, not binOprExp->expression_return_type->is_static_type());
    }
}

void expression_cleanup_visitor::functionCall_Exp_up(functionCall_expression_AST_node* funcCall, AST_visitor_base* expression_child, AST_visitor_base* arguments_child)
{
    if( funcCall->has_output_ownership and  funcCall->c_exp_can_be_referenced )
    {
        auto name = funcCall->writer->get_C_expression();
        funcCall->expression_return_type->write_destructor(name, source_fout, not funcCall->expression_return_type->is_static_type());
    }
}


//// some revisitors

//// child_expression_accumulator ////

//void child_expression_accumulator::callArguments_up(call_argument_list* callArgs, AST_visitor_base* unArgs_child, AST_visitor_base* namedArgs)
//{
//    child_expression_accumulator*  unArgs_child_casted = nullptr;
//    child_expression_accumulator*  namedArgs_casted = nullptr;
//
//    int T = 0;
//    if( unArgs_child )
//    {
//        unArgs_child_casted = dynamic_cast<child_expression_accumulator*>( unArgs_child );
//        T += unArgs_child_casted->children_Ccodes.size();
//    }
//    if( namedArgs )
//    {
//        namedArgs_casted = dynamic_cast<child_expression_accumulator*>( namedArgs );
//        T += namedArgs_casted->children_Ccodes.size();
//    }
//
//    children_Ccodes.reserve( T );
//
//    if( unArgs_child_casted )
//    {
//        for(auto str_pnt : unArgs_child_casted->children_Ccodes )
//        {
//            children_Ccodes.push_back( str_pnt );
//        }
//    }
//    if( namedArgs_casted )
//    {
//        for(auto str_pnt : namedArgs_casted->children_Ccodes )
//        {
//            children_Ccodes.push_back( str_pnt );
//        }
//    }
//}
//
//void child_expression_accumulator::baseArguments_up(call_argument_list::base_arguments_T* argList, std::list<AST_visitor_base*>& visitor_children)
//{
//    children_Ccodes.reserve( visitor_children.size() );
//    for( auto ptr : visitor_children )
//    {
//        auto src_exp_ptr = dynamic_cast<child_expression_accumulator*>( ptr )->twin;
//
//        children_Ccodes.push_back( &(src_exp_ptr->C_expression_code) );
//    }
//}




/// source_statement_visitor ///

source_statement_visitor::source_statement_visitor(ofstream& _source_fout, bool _write_definitions) :
    source_fout(_source_fout)
{
    do_children = true;
    write_definitions = _write_definitions;
}

shared_ptr< AST_visitor_base > source_statement_visitor::make_child(int number)
{
    return make_shared<source_statement_visitor>( source_fout );
}


//// things that cause this visitor to stop ////
void source_statement_visitor::funcDef_down(function_AST_node* funcDef)
{
    do_children = false;
}

//// "normal" things ////

void source_statement_visitor::block_up(block_AST_node* block, std::list<AST_visitor_base*>& visitor_children)
{
    /// destructors ///
    // this skims all top-level nodes, looking variables
    class destructuble_finder : public AST_visitor_NoChildren
    {
    public:
        list< varName_ptr > names_to_destruct;

        void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt)
        {
            names_to_destruct.push_back( autoStmt->variable_symbol );
        }

        void definitionStmt_down(definition_statement_AST_node* defStmt)
        {
            names_to_destruct.push_back( defStmt->variable_symbol );
        }

        void definitionNconstructionStmt_down(definitionNconstruction_statement_AST_node* defStmt)
        {
            names_to_destruct.push_back( defStmt->variable_symbol );
        }

    };

    // apply it
    destructuble_finder finder;

    for( auto &AST_node : block->contents )
    {
        AST_node->apply_visitor( &finder );
    }

    // now destruct it all!
    for( auto var_to_destruct : finder.names_to_destruct )
    {
        var_to_destruct->var_type->write_destructor( var_to_destruct->C_name, source_fout, not var_to_destruct->var_type->is_static_type() );
    }

}

void source_statement_visitor::statement_up(statement_AST_node* statment)
{
    source_fout <<  endl;
}

void source_statement_visitor::expressionStatement_down(expression_statement_AST_node* expStmt)
{
    source_expression_visitor expr_vistr( source_fout );
    expStmt->expression->apply_visitor( &expr_vistr );

    // write exp
    source_fout << (expStmt->expression->writer->get_C_expression() ) << ";" << endl;

    expression_cleanup_visitor cleanup( source_fout );
    expStmt->expression->apply_visitor( &cleanup );

}



void source_statement_visitor::typed_VarDef_up(Typed_VarDefinition* var_def, AST_visitor_base* var_type)
{

    if( write_definitions )
    {
        var_def->var_type->resolved_type->C_definition_name( var_def->variable_symbol->C_name, source_fout );
        source_fout << ";" << endl;
    }

    var_def->var_type->resolved_type->initialize( var_def->variable_symbol->C_name, source_fout );
}

void source_statement_visitor::definitionStmt_up(definition_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child)
{
    //defStmt->var_type->resolved_type->C_definition_name( defStmt->variable_symbol->C_name, source_fout );
    //source_fout << ";" << endl;

    defStmt->var_type->resolved_type->write_default_constructor(defStmt->variable_symbol->C_name, source_fout);

    source_fout<< endl;
}

void source_statement_visitor::definitionNconstruction_up(definitionNconstruction_statement_AST_node* defStmt, AST_visitor_base* varTypeRepr_child, AST_visitor_base* argList_child)
{
    source_expression_visitor arguments_visitor_PTR( source_fout );
    defStmt->argument_list->apply_visitor( &arguments_visitor_PTR );



    vector< utf8_string > argument_C_expressions;

    auto num_args = defStmt->argument_list->total_size();
    argument_C_expressions.reserve( num_args );
    for( int i=0; i<num_args; i++)
    {
        argument_C_expressions.push_back( defStmt->argument_list->expression_from_index( i )->writer->get_C_expression() );
    }



    defStmt->var_type->resolved_type->write_explicit_constructor(defStmt->argument_list.get(), defStmt->variable_symbol->C_name,
                                                    argument_C_expressions, source_fout);

    expression_cleanup_visitor cleanup( source_fout );
    defStmt->argument_list->apply_visitor( &cleanup );
}



void source_statement_visitor::assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* LHS_reference_child, AST_visitor_base* expression_child)
{
    source_expression_visitor expr_vistr( source_fout );
    assignStmt->expression->apply_visitor( &expr_vistr );

    auto exp = assignStmt->expression->writer->get_C_expression();
    source_LHSreference_visitor LHS_vistr( source_fout, assignStmt->expression.get(), exp, assignStmt->expression->expression_return_type);
    assignStmt->LHS->apply_visitor( &LHS_vistr );
    source_fout << endl;


    expression_cleanup_visitor cleanup( source_fout );
    assignStmt->expression->apply_visitor( &cleanup );


    LHSexpression_cleanup_visitor LHScleanup( source_fout );
    assignStmt->LHS->apply_visitor( &LHScleanup );
}

void source_statement_visitor::autoDefStmt_up(auto_definition_statement_AST_node* autoStmt, AST_visitor_base* expression_child)
{
    if( write_definitions )
    {
        autoStmt->variable_symbol->var_type->C_definition_name( autoStmt->variable_symbol->C_name, source_fout );
        source_fout << ";" << endl;

    } /// not sure why this isn't a generic definition type???

    autoStmt->variable_symbol->var_type->initialize( autoStmt->variable_symbol->C_name, source_fout );

    source_expression_visitor expr_vistr( source_fout );
    autoStmt->expression->apply_visitor( &expr_vistr );


//
//    auto exp = autoStmt->expression->writer->get_C_expression();
//    autoStmt->variable_symbol->var_type->write_implicit_copy_constructor( autoStmt->expression->expression_return_type.get(),
//                       autoStmt->expression.get(), autoStmt->variable_symbol->C_name, exp, source_fout );
//
//                       write_implicit_copy_constructor(varType* RHS_type, expression_AST_node* RHS_AST_node, csu::utf8_string& LHS,
//                        csu::utf8_string& RHS_exp, std::ostream& output);




    auto unnamed_args = make_shared<call_argument_list::unnamed_arguments_T>(autoStmt->loc);
    unnamed_args->add_argument( autoStmt->expression, autoStmt->expression->loc );
    call_argument_list argument_nodes(autoStmt->loc, unnamed_args, nullptr );
    argument_nodes.symbol_table = autoStmt->symbol_table;

    vector<csu::utf8_string> args;
    args.emplace_back( autoStmt->expression->writer->get_C_expression() );

    autoStmt->variable_symbol->var_type->write_explicit_constructor( &argument_nodes,  autoStmt->variable_symbol->C_name,
                        args, source_fout);

    source_fout << endl;


    expression_cleanup_visitor cleanup( source_fout );
    autoStmt->expression->apply_visitor( &cleanup );
}

void source_statement_visitor::returnStatement_down(return_statement_AST_node* returnStmt)
{
    source_expression_visitor expr_vistr( source_fout );
    returnStmt->expression->apply_visitor( &expr_vistr );
    auto ret_exp = returnStmt->expression->writer->get_C_expression();

    // check if we need casting
    // this can be optimized if we need no casting

    auto return_type = returnStmt->callable_to_escape->return_type;
    auto expression_type = returnStmt->expression->expression_return_type;

    utf8_string argument_vname = "__cy__retTMP_" + returnStmt->symbol_table->get_unique_string();
    return_type->C_definition_name(argument_vname, source_fout);
    source_fout << ';'<<endl;
    return_type->initialize(argument_vname, source_fout);

    if( return_type->has_implicit_copy_constructor( expression_type.get() ) )
    {
        return_type->write_implicit_copy_constructor(expression_type.get(), returnStmt->expression.get(), argument_vname,
                                                ret_exp, source_fout);
    }
    else if(  expression_type->can_implicit_castTo( return_type.get() ) )
    {
        expression_type->write_implicit_castTo(return_type.get(), returnStmt->expression.get(), argument_vname,
                     ret_exp , source_fout);
    }
    else
    {
        throw gen_exception("this should never be reached. In source_statement_visitor::returnStatement_down");
    }

    // cleanup expression
    expression_cleanup_visitor cleanup( source_fout );
    returnStmt->expression->apply_visitor( &cleanup );


    // call destructors on active variables
    // first, find variables to destruct
    list<varName_ptr> variables_to_destruct;
    sym_table_base* current_symbol_table = returnStmt->symbol_table;
    sym_table_base* outer_symbol_table = returnStmt->callable_to_escape->symbol_table;
    while( true )
    {
        if( current_symbol_table == outer_symbol_table)
        {
            break;
        }

        for(auto& x : current_symbol_table->variable_table)
        {
            auto current_var_name = x.second;
            if( current_var_name->loc.strictly_LT( returnStmt->loc ) )
            {
                variables_to_destruct.push_back( current_var_name );
            }
        }

        auto __current_symbol_table__ = dynamic_cast<local_sym_table*>( current_symbol_table );
        current_symbol_table = __current_symbol_table__->parent_table;
    }
    // then..... DESTROY!!
    for( auto& var : variables_to_destruct )
    {
        var->var_type->write_destructor( var->C_name, source_fout, not var->var_type->is_static_type() );
    }

    // write return
    source_fout << "return " << argument_vname << ";" << endl;
}

void source_statement_visitor::constructElement_up(constructElement_AST_node* constructBlock, AST_visitor_base* exp_child, AST_visitor_base* argList_child)
{
    source_expression_visitor expr_vistr( source_fout );
    constructBlock->expression->apply_visitor( &expr_vistr );

    source_expression_visitor arguments_visitor_PTR( source_fout );
    constructBlock->argument_list->apply_visitor( &arguments_visitor_PTR );



    vector< utf8_string > argument_C_expressions;
    auto num_args = constructBlock->argument_list->total_size();
    argument_C_expressions.reserve( num_args );
    for( int i=0; i<num_args; i++)
    {
        auto arg_exp =  constructBlock->argument_list->expression_from_index( i )->writer->get_C_expression();
        argument_C_expressions.push_back( arg_exp );
    }

    auto exp = constructBlock->expression->writer->get_C_expression();
    constructBlock->expression->expression_return_type->write_explicit_constructor(constructBlock->argument_list.get(),
            exp, argument_C_expressions, source_fout);

    expression_cleanup_visitor cleanup( source_fout );
    constructBlock->expression->apply_visitor( &cleanup );

    expression_cleanup_visitor args_cleanup( source_fout );
    constructBlock->argument_list->apply_visitor( &args_cleanup );
}





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
        /// destructors ///
        // annoying we have to write this twice?
        // this skims all top-level nodes, looking variables
        class destructuble_finder : public AST_visitor_NoChildren
        {
        public:
            list< varName_ptr > names_to_destruct;

            void autoDefStmt_down(auto_definition_statement_AST_node* autoStmt)
            {
                names_to_destruct.push_back( autoStmt->variable_symbol );
            }

            void definitionStmt_down(definition_statement_AST_node* defStmt)
            {
                names_to_destruct.push_back( defStmt->variable_symbol );
            }

            void definitionNconstructionStmt_down(definitionNconstruction_statement_AST_node* defStmt)
            {
                names_to_destruct.push_back( defStmt->variable_symbol );
            }

        };

        // apply it
        destructuble_finder finder;

        for( auto &AST_node : module->module_contents )
        {
            AST_node->apply_visitor( &finder );
        }

        // now destruct it all!
        for( auto var_to_destruct : finder.names_to_destruct )
        {
            var_to_destruct->var_type->write_destructor( var_to_destruct->C_name, source_fout, not var_to_destruct->var_type->is_static_type() );
        }


        /// END THE FUNCTION
        source_fout << "}" << endl;
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
                auto default_exp = *default_exp_iter;
                default_exp->apply_visitor( &expr_vistr );

                auto C_exp = default_exp->writer->get_C_expression();

                param_name_iter->var_type_ASTnode->resolved_type->write_implicit_copy_constructor(default_exp->expression_return_type.get(), default_exp.get(),
                            param_name_iter->variable_symbol->C_name, C_exp, source_fout );

                //param_name_iter->var_type_ASTnode->resolved_type->write_assignment( default_exp->expression_return_type.get(), default_exp.get(),
                //            param_name_iter->variable_symbol->C_name, C_exp, source_fout );


                expression_cleanup_visitor cleanup( source_fout );
                default_exp->apply_visitor( &cleanup );

                source_fout << "}"<<endl;

            }
        }
//TODO! inform_moved!!

        // write block of statements

        source_statement_visitor statement_writer( source_fout );
        funcDef->block_AST->apply_visitor( &statement_writer );
        source_fout << "}" << endl << endl;
    }

    void methodDef_up(method_AST_node* methodDef, AST_visitor_base* returnType_child, AST_visitor_base* paramList_child,
                              AST_visitor_base* methodBody_child)
    {
        auto self_var = methodDef->funcType->self_ptr_name;

        methodDef->specific_overload->write_C_prototype( source_fout );
        source_fout << endl;
        source_fout << "{" << endl;

        /// write self pointer cast
        self_var->var_type->C_definition_name(self_var->C_name, source_fout);
        source_fout << " = (" ;
        utf8_string TMP("");
        self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
        source_fout << ")" << self_var->C_name << "_;" << endl;


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
                auto default_exp = (*default_exp_iter);
                default_exp->apply_visitor( &expr_vistr );

                auto default_C_exp = default_exp->writer->get_C_expression();

                param_name_iter->var_type_ASTnode->resolved_type->write_implicit_copy_constructor( default_exp->expression_return_type.get(), default_exp.get(),
                            param_name_iter->variable_symbol->C_name, default_C_exp, source_fout );

                //param_name_iter->var_type_ASTnode->resolved_type->write_assignment( default_exp->expression_return_type.get(), default_exp.get(),
                 //           param_name_iter->variable_symbol->C_name, default_C_exp, source_fout );

                expression_cleanup_visitor cleanup( source_fout );
                default_exp->apply_visitor( &cleanup );

                source_fout << "}"<<endl;

            }
        }
//TODO! inform_moved!!


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
                    auto Cexp = self_var->var_type->write_member_getter( self_var->C_name, var_name  , source_fout  );

                    auto var_type = varName_ptr_pair.second->var_type;
                    auto C_exp_str = Cexp->get_C_expression();
                    var_type->write_default_constructor( C_exp_str, source_fout );
                    Cexp->write_cleanup();
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
                        auto Cexp = parameter_type->write_member_getter( parameter_name->C_name, var_name, source_fout  );

                        auto var_type = varName_ptr_pair.second->var_type;
                        auto Cexp_str = Cexp->get_C_expression();
                        var_type->write_default_constructor( Cexp_str, source_fout );
                        Cexp->write_cleanup();
                    }
                 }
            }
        }



        // write block of statements
        source_statement_visitor statement_writer( source_fout );
        methodDef->block_AST->apply_visitor( &statement_writer );



        // __del__
        if( methodDef->funcType->type_of_method == MethodType::destructor )
        {
            auto class_type = dynamic_pointer_cast<DefClassType>( methodDef->class_type );
            auto class_sym_table = class_type->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                utf8_string var_name( varName_ptr_pair.first );
                auto Cexp = self_var->var_type->write_member_getter( self_var->C_name, var_name  , source_fout  );
                auto Cexp_str = Cexp->get_C_expression();

                auto var_type = varName_ptr_pair.second->var_type;
                var_type->write_destructor( Cexp_str, source_fout, not var_type->is_static_type() );

                Cexp->write_cleanup();
            }
        }

        source_fout << "}" << endl << endl;
    }


    void ClassDef_up( class_AST_node* clss_node, std::list<AST_visitor_base*>& var_def_children,
                     std::list<AST_visitor_base*>& method_def_children, AST_visitor_base* inheritanceList_child )
    {
        /// need to write out the defaulted methods ///
        auto self_var = clss_node->self_name;

        // defaulted constructor
        if( clss_node->write_default_constructor )
        {
            clss_node->default_constructor_overload->write_C_prototype( source_fout );
            source_fout << endl;
            source_fout << "{" << endl;

            /// write self pointer cast
            self_var->var_type->C_definition_name(self_var->C_name, source_fout);
            source_fout << " = (" ;
            utf8_string TMP("");
            self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
            source_fout << ")" << self_var->C_name << "_;" << endl;

            /// then initiate the members
            auto class_sym_table = clss_node->type_ptr->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                if( varName_ptr_pair.first == "self" )
                {   continue;  }

                utf8_string var_name( varName_ptr_pair.first );
// TODO, getref????
                auto Cexp = self_var->var_type->write_member_getter( self_var->C_name, var_name  , source_fout  );
                auto Cexp_str = Cexp->get_C_expression();

                auto var_type = varName_ptr_pair.second->var_type;
                var_type->write_default_constructor( Cexp_str, source_fout );

                Cexp->write_cleanup();
            }
            source_fout << "}" << endl << endl;
        }

        // defaulted destructor
        if( clss_node->write_default_deconstructor )
        {
            clss_node->default_destructor_overload->write_C_prototype( source_fout );
            source_fout << endl;
            source_fout << "{" << endl;

            /// write self pointer cast
            self_var->var_type->C_definition_name(self_var->C_name, source_fout);
            source_fout << " = (" ;
            utf8_string TMP("");
            self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
            source_fout << ")" << self_var->C_name << "_;" << endl;

            /// and destruct all the other things
            auto class_sym_table = clss_node->type_ptr->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                if( varName_ptr_pair.first == "self" )
                {   continue;  }

                utf8_string var_name( varName_ptr_pair.first );
                auto Cexp = self_var->var_type->write_member_getref( self_var->C_name, var_name  , source_fout );
                auto Cexp_str = Cexp->get_C_expression();

                auto var_type = varName_ptr_pair.second->var_type;

                var_type->write_destructor( Cexp_str, source_fout, not var_type->is_static_type() );

                Cexp->write_cleanup();
            }


            source_fout << "}" << endl << endl;
        }

        //defaulted copy constructor
        if( clss_node->write_selfCopy_constructor )
        {
            clss_node->default_CopyConstructor_overload->write_C_prototype( source_fout );
            source_fout << endl;
            source_fout << "{" << endl;

            /// write self pointer cast
            self_var->var_type->C_definition_name(self_var->C_name, source_fout);
            source_fout << " = (" ;
            utf8_string TMP("");
            self_var->var_type->C_definition_name( TMP, source_fout); // hope this works!!
            source_fout << ")" << self_var->C_name << "_;" << endl;


//TODO! inform_moved!!

            /// copy other variables.
            auto class_sym_table = clss_node->type_ptr->class_symbol_table;
            for( auto &varName_ptr_pair : class_sym_table->variable_table )
            {
                if( varName_ptr_pair.first == "self" )
                {   continue;  }


                auto TYPE = varName_ptr_pair.second->var_type;
                if( TYPE->has_implicit_copy_constructor(TYPE.get()) )
                {
                    utf8_string name = varName_ptr_pair.first ;
                    auto Cexp = self_var->var_type->write_member_getref( self_var->C_name,  name, source_fout  );
                    auto Cexp_str = Cexp->get_C_expression();

                    utf8_string RHS = "(__cy__arg_RHS->"+varName_ptr_pair.second->C_name;
                    RHS += ")"; // I hope this works!

                    varReferance_expression_AST_node RHS_ast(name, clss_node->loc );
                    RHS_ast.variable_symbol = varName_ptr_pair.second;
                    RHS_ast.expression_return_type = TYPE;
                    RHS_ast.symbol_table = clss_node->symbol_table;

                    TYPE->write_implicit_copy_constructor(TYPE.get(), &RHS_ast,
                                Cexp_str, RHS, source_fout);
                    source_fout << endl;
                    Cexp->write_cleanup();
                }
            }

            source_fout << "}" << endl << endl;
        }
    }

};

//// the main function ////

void write_module_to_C(module_AST_ptr module)
{

    //// first we write the header ///
    string header_fname = module->file_name + ".h";
    string source_fname = module->file_name + ".c";
    ofstream C_header_file(header_fname);

    module->C_header_fname = header_fname;
    module->C_source_fname = source_fname;

//header_fname = "DELME_" + header_fname;
//source_fname = "DELME_" + source_fname;

    // imports //
    header_writeImports_visitor import_writer(C_header_file, header_fname);
    module->apply_visitor( &import_writer );

    // definitions //
    header_writeDefinitions_visitor definitions_writer(C_header_file, header_fname);
    module->apply_visitor( &definitions_writer );


    //// now we write the source files ////
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
        string source_main_fname( module->file_name + "_main.c" );
        module->C_mainSource_fname = source_main_fname;

//source_main_fname = "DELME_" + source_main_fname;

        ofstream C_source_main_file(source_main_fname);

        C_source_main_file << "#include \"" << header_fname << "\"" <<endl;
        C_source_main_file << "void main(void){" <<endl;
        // write prep functions for all other modules here!!
        C_source_main_file << module->module_name << "__init__();" << endl;
        C_source_main_file << module->main_func_name << "();" << endl;
        C_source_main_file << '}' << endl;
    }
}
