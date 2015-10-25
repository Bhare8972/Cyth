#include "catch.hpp"
#include "lexer.hpp"
using namespace std;
using namespace csu;

TEST_CASE( "test lexer", "[lexer]" ) 
{
    lexer_generator<int> gen("./lexer_table");
}
