#include "catch.hpp"
#include "parser.hpp"

#include <stdio.h>
using namespace std;
using namespace csu;

int return_one(parser_function_class::data_T& data)
{
    return 1;
}

TEST_CASE( "test parser", "[parser, lexer]" )
{
    parser_generator par_gen("./parse_table_test.txt", "./lex_table_test.txt");

    //// define the grammer ////

    // get info from par_gen //
    auto EOF_term=par_gen.get_EOF_terminal();
    auto lex_gen=par_gen.get_lexer_generator();

    //define terminals
    auto INT_term=par_gen.new_terminal("integer");

    //set terminal patterns
    INT_term->add_pattern("[0-9]*");

    //define non-terminals
    //auto FINAL_nonterm = par_gen.new_nonterminal("final"); //since this is first, it will be the starting non-term
    auto INT_LIST_nonterm= par_gen.new_nonterminal("int_list");

    //define non-term productions
    //FINAL_nonterm->add_production({ INT_LIST_nonterm, EOF_term }).set_action< int >( return_one );

    INT_LIST_nonterm->add_production({ INT_term }).set_action< int >( return_one );
    INT_LIST_nonterm->add_production({ INT_LIST_nonterm,INT_term }).set_action< int >( return_one );

    //set other lexer actions
    lex_gen->add_nonreturn_pattern("\" \"");//to eat spaces

    ////get the parser ////
    auto parse=par_gen.get_parser();

    I AM HERE. TEST PARSER
    IMPROVE PARSER AND GENERATOR
    ADD UNIT TESTS

    remove("./parse_table_test.txt");//delete our lexer file.
    remove("./lex_table_test.txt");//delete our parser file.
}
