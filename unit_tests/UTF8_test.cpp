#include "catch.hpp"
#include "UTF8.hpp"
using namespace std;
using namespace csu;

TEST_CASE( "UTF8 string", "[utf8_string]" ) {
    utf8_string a("yo¤");
    REQUIRE(a=="yo¤");
}
