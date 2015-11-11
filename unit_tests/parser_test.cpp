#include "catch.hpp"
#include "parser.hpp"

#include <stdio.h>
using namespace std;
using namespace csu;


TEST_CASE( "test parser", "[parser, lexer]" )
{
    parser_generator("./parse_table_test.txt", "./lex_table_test.txt");
    
    remove("./parse_table_test.txt");//delete our lexer file.
    remove("./lex_table_test.txt");//delete our parser file.
}
