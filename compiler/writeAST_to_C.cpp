
#include "writeAST_to_C.hpp"
#include "AST_visitor.hpp"
#include <random>
#include <sstream>

using namespace csu;
using namespace std;


////  HEADER WRITERS ////
// writes import statements into header
class header_writeImports_visitor : public AST_visitorTree
{
public:
    bool do_children;
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

    bool apply_to_children(){ return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number)
    {
        return make_shared<header_writeImports_visitor>( false, header_fout, header_fout_name ); // if we are here, then we are on module.
        // The modules children should not apply the visitor, as only top-level statements can be imports
    }

    void module_down(module_AST_node* module)
    {
        default_random_engine generator;
        uniform_int_distribution<int> distribution(10000000, 99999999);

        string h_gaurd_ID =  to_string( distribution(generator) );
        header_fout << "#ifndef " << module->module_name << "_" << h_gaurd_ID << endl;
        header_fout << "#define " << module->module_name << "_" << h_gaurd_ID << endl;
    }

    void cImports_down(import_C_AST_node* ASTnode)
    {
        // hilariously, we don't do anything here yet
    }

};

// writes definitions of functions, variables, and structs into header
// only visits top AST nodes... with exceptions (mainly methods)...
class header_writeDefinitions_visitor : public AST_visitorTree
{
public:
    bool do_children;
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

    bool apply_to_children(){ return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number)
    {
        return make_shared<header_writeDefinitions_visitor>( false, header_fout, header_fout_name ); // if we are here, then we are on module.
        // The modules children should not apply the visitor, as only top-level statements can be have definitions
    }



    void funcDef_down(function_AST_node* funcDef)
    {
        header_fout << "void " << funcDef->specific_overload->C_name << "(void);" <<endl;
    }

    void definitionStmt_down(definition_statement_AST_node* defStmt)
    {
        header_fout << "extern " << defStmt->var_type->resolved_type->C_name << " " << defStmt->variable_symbol->C_name << ";" << endl;
    }



    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children)
    {
        header_fout << "#endif"  <<  endl;
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

    bool apply_to_children(){ return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number)
    {
        return make_shared<source_writePreamble_visitor>( false, source_fout ); // if we are here, then we are on module.
        // The modules children should not apply the visitor, so far, only top-level statements are need for preamble!
    }


    void module_down(module_AST_node* module)
    {
        source_fout << "#include \"" << header_fname << "\"" << endl;
        source_fout << endl;

        // these should be included from a library.
        source_fout << "#define __Ctype_add_macro__(X, Y) \\" << endl;
        source_fout << "({ typeof (X) __cy_TMP_x__ = (X); \\" << endl;
        source_fout << " typeof (Y) __cy_TMP_y__ = (Y); \\" << endl;
        source_fout << " __cy_TMP_x__ + __cy_TMP_y__; })" << endl;
        source_fout << endl;

        source_fout << "#define __Ctype_assignment_macro__(X, Y) \\" << endl;
        source_fout << "({ typeof (Y) __cy_TMP_y__ = (Y); \\" << endl;
        source_fout << "*X = __cy_TMP_y__; })" << endl;
        source_fout << endl;

    }

    void definitionStmt_down(definition_statement_AST_node* defStmt)
    {
        // should only be run on top-level statements, which presently, is all this visitor does.
        source_fout << defStmt->var_type->resolved_type->C_name << " " << defStmt->variable_symbol->C_name << ";" << endl;

        source_fout << endl;
    }
};

//
class source_statement_visitor : public AST_visitorTree
{
/// write out straight-forward statements. does not write functions, classes, or those complex thingies.

public:
    ofstream& source_fout;
    utf8_string expression_C_name;

    source_statement_visitor(ofstream& _source_fout) :
        source_fout(_source_fout)
    {
    }

    shared_ptr< AST_visitor_base > make_child(int number)
    {
        return make_shared<source_statement_visitor>( source_fout );
    }

    void statement_up(statement_AST_node* statment)
    {
        source_fout <<  endl;
    }

    void assignmentStmt_up(assignment_statement_AST_node* assignStmt, AST_visitor_base* expression_child)
    {

        source_statement_visitor* expression_visitor_PTR = dynamic_cast<source_statement_visitor*>(expression_child);

        source_fout << assignStmt->operation_function_name << "( &" << assignStmt->variable_symbol->C_name << "," << expression_visitor_PTR->expression_C_name << ");"<<endl;
    }

    void functionCall_Stmt_up(functionCall_statement_AST_node* funcCall,  AST_visitor_base* expression_child)
    {
        if( funcCall->expression->expression_return_type->needs_to_be_resolved() )
        {
             // this is a static function
             source_fout << funcCall->specific_overload->C_name << "();" << endl;
        }
        else
        {
            // this is more of a function-pointer thing I think!
            source_statement_visitor* expression_visitor_PTR = dynamic_cast<source_statement_visitor*>(expression_child);
            source_fout << expression_visitor_PTR->expression_C_name << "();" << endl;
        }
    }

    void intLiteral_up(intLiteral_expression_AST_node* intLitExp)
    {
        expression_C_name = intLitExp->symbol_table->get_unique_variable_name();

        source_fout << "typeof(" << intLitExp->literal << ") " << expression_C_name << "=" <<  intLitExp->literal << ";" << endl;
    }

    void binOperator_up(binOperator_expression_AST_node* binOprExp, AST_visitor_base* LHS_exp_visitor, AST_visitor_base* RHS_exp_visitor)
    {
        expression_C_name = binOprExp->symbol_table->get_unique_variable_name();

        source_statement_visitor* LHS_exp_visitor_PTR = dynamic_cast<source_statement_visitor*>(LHS_exp_visitor);
        source_statement_visitor* RHS_exp_visitor_PTR = dynamic_cast<source_statement_visitor*>(RHS_exp_visitor);

        stringstream exp;
        exp << binOprExp->operation_function_name << "(" << LHS_exp_visitor_PTR->expression_C_name << "," << RHS_exp_visitor_PTR->expression_C_name << ")";
        string exp_str;
        exp >> exp_str;

        if( binOprExp->return_type_is_unnamed )
        {
            source_fout << "typeof(" << exp_str << ")";
        }
        else
        {
            source_fout << binOprExp->expression_return_type->C_name;
        }

        source_fout << " " << expression_C_name << "=" << exp_str << ";" << endl;
    }

    void varReferance_up(varReferance_expression_AST_node* varRefExp)
    {
        expression_C_name = varRefExp->variable_symbol->C_name; // the magic of compilers!
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

    source_moduleExpresion_visitor(ofstream& _source_fout) :
        source_fout(_source_fout)
    {
        do_children = true;
    }

    source_moduleExpresion_visitor(bool _do_children, ofstream& _source_fout) :
        source_fout(_source_fout)
    {
        do_children = _do_children;
    }

    bool apply_to_children(){ return do_children; }

    shared_ptr< AST_visitor_base > make_child(int number)
    {
        return make_shared<source_moduleExpresion_visitor>( false, source_fout );
    }


    void module_down(module_AST_node* module)
    {
        source_fout << "void " << module->module_name << "__init__(void){" << endl;
    }

    void module_up(module_AST_node* module, std::list<AST_visitor_base*>& visitor_children)
    {
        source_fout << "}" << endl;
    }

    void statement_down(statement_AST_node* statment)
    {
        source_statement_visitor statement_writer( source_fout );
        statment->apply_visitor( &statement_writer );
    }
};

class source_function_visitor : public AST_visitorTree
{
// looks for functions, and writes their source using the above expression writer.
// probably needs to be VERY recursive due to nested functions

public:
    ofstream& source_fout;

    source_function_visitor(ofstream& _source_fout) :
        source_fout(_source_fout)
    {
    }

    shared_ptr< AST_visitor_base > make_child(int number)
    {
        return make_shared<source_function_visitor>( source_fout );
    }

    void funcDef_up(function_AST_node* funcDef, AST_visitor_base* stmt_child)
    {
        source_fout << "void " << funcDef->specific_overload->C_name << "(void)" << endl;
        source_fout << "{" << endl;

        source_statement_visitor statement_writer( source_fout );
        funcDef->stmt->apply_visitor( &statement_writer );

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
}
