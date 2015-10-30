#include "catch.hpp"
#include "lexer.hpp"
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
        cout<<"RETURNING: "<<to_return<<endl;
        return to_return;
    }
};

TEST_CASE( "test lexer", "[lexer]" )
{
    lexer_generator<int> gen("./lexer_table");

    gen.set_EOF_action(lex_int(-1));

    gen.add_nonreturn_pattern("\" \""); //no return on spaces
    gen.add_nonreturn_pattern("\\n"); //no return on new lines
    gen.add_pattern("q+", lex_int(1)); //return a 1 on list of 1
    //on comments, enter lex state 1 and return a 2
    gen.add_pattern("\\\\", [](const utf8_string& data, const location_span& loc, lexer<int>* lex){ lex->set_state(1); return 2; });
    gen.add_multi_patterns({"[0-9]+([.][0-9]*)?","[.][0-9]*"}, lex_int(3)); //on float numbers, return a 3

    gen.increment_state(); //setup state 1, which is parsing comments
    gen.add_nonreturn_pattern("."); //ignore most charectors
    //but on a new line, switch back to previous state, but keep lexing
    gen.add_pattern("\\n", [](const utf8_string& data, const location_span& loc, lexer<int>* lex){ lex->set_state(0); cout<<"Yo"<<endl; lex->continue_lexing(true); return 0; });

    //setup a fake program source to read from
    stringstream src;
    src<<"qq qqq   qqqqq"<<endl;
    src<<"   qq \\\\ comments"<<endl;

    auto lex=gen.get_lexer();
    lex.set_input(src);

    lex.print_machine();
    //lex!
    //first four tokens are just qs
    INFO("first test")
    REQUIRE(lex()==1);
    INFO("second test")
    REQUIRE(lex()==1);
    INFO("third test")
    REQUIRE(lex()==1);
    INFO("fourth test")
    REQUIRE(lex()==1);

    INFO("fifth test")
    REQUIRE(lex()==2); //we get a comment
    INFO("sixth test")
    REQUIRE(lex()==-1); //and an EOF
}
