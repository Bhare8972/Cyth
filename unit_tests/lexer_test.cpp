#include "catch.hpp"
#include "lexer.hpp"

#include <stdio.h>
using namespace std;
using namespace csu;

class lex_int
{
public:
    int to_return;

    lex_int(int _to_return)
    {
        to_return=_to_return;
    }

    virtual int operator()(const utf8_string& data, const location_span& loc, lexer<int>* lex)
    {
        return to_return;
    }
};

TEST_CASE( "test lexer", "[lexer]" )
{
//note:
//not a real unit test, becouse UTF8, regex, and ring_buffer do not have any unit tests

    lexer_generator<int> gen("./lexer_table_test");

    gen.set_EOF_action(lex_int(-1));

    gen.add_nonreturn_pattern(R"(" ")"); //no return on spaces
    gen.add_nonreturn_pattern(R"(\n)"); //no return on new lines
    gen.add_pattern("q+", lex_int(1)); //return a 1 on list of 1
    //on comments, enter lex state 1 and return a 2
    gen.add_pattern(R"(\\\\)", LEX_FUNC(int){ lex->set_state(1); return 2; }); //note the need for a raw C++ string, with four slashes needed to accept two slashes in input text
    gen.add_multi_patterns({"[0-9]+([.][0-9]*)?","[.][0-9]+"}, lex_int(3)); //on float numbers, return a 3

    gen.increment_state(); //setup state 1, which is parsing comments
    //but on a new line, switch back to previous state, but keep lexing
    gen.add_pattern("\\n", LEX_FUNC(int){ lex->set_state(0); lex->continue_lexing(true); return 0; } );
    gen.add_nonreturn_pattern("."); //ignore most charectors
    //note the order of the last two patterns. THe second can accept teh first, so the first is first so that it is not overridden by the second

    //setup a fake program source to read from
    stringstream src;
    src<<"qq qqq   qqqqq"<<endl;
    src<<"   .5654 3423.432 \\\\ comments"<<endl;

    auto lex=gen.get_lexer(); //note that this will make a file
    lex.set_input(src);

    //lex!
    //first four tokens are just qs
    INFO("first test")
    REQUIRE(lex()==1);
    INFO("second test")
    REQUIRE(lex()==1);
    INFO("third test")
    REQUIRE(lex()==1);
    INFO("fourth test")
    REQUIRE(lex()==3);
    INFO("fifth test")
    REQUIRE(lex()==3);

    INFO("sixth test")
    REQUIRE(lex()==2); //we get a comment
    INFO("seventh test")
    REQUIRE(lex()==-1); //and an EOF

    gen.load_from_file(); //force the lexer generator to get new lexer info from the file
    auto new_lex=gen.get_lexer();

    stringstream new_src;
    new_src<<"qq qqq   qqqqq"<<endl;
    new_src<<"   .5654 3423.432 \\\\ comments"<<endl;
    new_lex.set_input(new_src);

    INFO("first B test")
    REQUIRE(new_lex()==1);
    INFO("second B test")
    REQUIRE(new_lex()==1);
    INFO("third B test")
    REQUIRE(new_lex()==1);
    INFO("fourth B test")
    REQUIRE(new_lex()==3);
    INFO("fifth B test")
    REQUIRE(new_lex()==3);
    INFO("sixth B test")
    REQUIRE(new_lex()==2); //we get a comment
    INFO("seventh B test")
    REQUIRE(new_lex()==-1); //and an EOF

    remove("./lexer_table_test");//delete our lexer file.
}
