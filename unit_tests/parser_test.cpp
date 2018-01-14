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
    parser_generator par_gen("./parse_table_test", "./lex_table_with_parser_test");

    //// define the grammer ////

    // get info from par_gen //
    auto EOF_term=par_gen.get_EOF_terminal();
    auto ERROR_term=par_gen.get_ERROR_terminal();
    auto lex_gen=par_gen.get_lexer_generator();

    //define terminals
    auto INT_term=par_gen.new_terminal("integer");

    //set terminal patterns
    INT_term->add_pattern("[0-9]+");

    //define non-terminals
    auto NUM_LIST_nonterm= par_gen.new_nonterminal("int_list"); //start non-terminal
    auto NUM_nonterm= par_gen.new_nonterminal("number");
    auto ADDITION_nonterm = par_gen.new_nonterminal("addition");
    auto MULTIPLICATION_nonterm= par_gen.new_nonterminal("multiplication");

    //define non-term productions
    NUM_LIST_nonterm->add_production({  })      .set_action< int >( return_one );  //EPSILON
    NUM_LIST_nonterm->add_production({ NUM_LIST_nonterm, NUM_nonterm }).set_action< int >( return_one );
    NUM_LIST_nonterm->add_production({ NUM_LIST_nonterm, ERROR_term }).set_action< int >( return_one );

    NUM_nonterm->add_production({ INT_term })              .set_action< int >( return_one );
    NUM_nonterm->add_production({ ADDITION_nonterm })      .set_action< int >( return_one );
    NUM_nonterm->add_production({ MULTIPLICATION_nonterm }).set_action< int >( return_one );

    MULTIPLICATION_nonterm->add_production({NUM_nonterm, R"( "*" )", NUM_nonterm}).set_associativity("LEFT").set_precedence().set_action< int >( return_one );
    ADDITION_nonterm->add_production({NUM_nonterm, R"( "+" )", NUM_nonterm}).set_associativity("RIGHT").set_precedence().set_action< int >( return_one );

    //set other lexer actions
    lex_gen->add_nonreturn_pattern("\" \"");//to eat spaces

    ////get the parser ////
    auto parse=par_gen.get_parser();

    //// make a test src ///
    stringstream src;
    src<<" 32 78+23*4 0889";
    parse->reset_input(src);

    //// RUN ////
    parse->parse(true);

    //// run again to test loading from file ////
    //std::cout<<"--RESETING--"<<std::endl;
    par_gen.reset_table();
    parse=par_gen.get_parser();
    src.str( "" );
    src.clear();
    src<<" 27*6 3+4 54";
    parse->reset_input(src);
    parse->parse(true);

    remove("./parse_table_test");//delete our lexer file.
    remove("./lex_table_with_parser_test");//delete our parser file.

    //probably need to add some kind of test to make sure that it is actually working.....
}
