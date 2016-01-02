#include "catch.hpp"
#include "type_erasure.hpp"
using namespace std;
using namespace csu;

TEST_CASE( "test type_erasure dyn_holder", "[dyn_holder]" ) 
{
    
    int first_var_value=5;
    dyn_holder var(first_var_value);
    
    REQUIRE(var.empty()==false);
    
    int cast_to;
    var.cast(cast_to);
    REQUIRE(cast_to==first_var_value);
    
    shared_ptr<float> second_var_value(new float(10));
    dyn_holder second_var(second_var_value);
    
    auto a=second_var.cast<float>();
    REQUIRE(a.get()==second_var_value.get());
    REQUIRE( (*a)==(*second_var_value) );
    REQUIRE( var.compatible(second_var_value)==false );
    
    REQUIRE_THROWS_AS(var.cast<float>(), bad_type_erasure_cast );
}

//a special test class to test the dyn_method
class dyn_method_tester
{
    int my_val;
public:
    dyn_method_tester()
    {
        my_val=0;
    }

    int return_five() //0 arguments
    {
        return 5;
    }
    
    double add_five(int A) //one argument
    {
        return A+5.0;
    }
    
    double add(int A, double B)//two arguments
    {
        return A+B;
    }
    
    void setter(int input)
    {
        my_val=input;
    }
};

TEST_CASE( "test type_erasure dyn_method", "[dyn_method][dyn_holder]" ) 
{
    
    dyn_method_tester TST_CLS;
    
    dyn_method zero_arg(TST_CLS, &dyn_method_tester::return_five);
    auto ret_data=zero_arg();
    
    REQUIRE(*ret_data.cast<int>()==5);
    REQUIRE_THROWS_AS( zero_arg(10), wrong_num_args );
    REQUIRE_NOTHROW( zero_arg(list<dyn_holder>()) );
    
    
    
    dyn_method one_arg(TST_CLS, &dyn_method_tester::add_five);
    auto ret_data2=one_arg(6);
    
    REQUIRE(*ret_data2.cast<double>() == Approx( 11.0 ));
    REQUIRE_THROWS_AS( one_arg(6.0),  bad_type_erasure_cast);
    
    REQUIRE_THROWS_AS( one_arg(), wrong_num_args );
    
    list<dyn_holder> an_argument;
    an_argument.emplace_back(6);
    REQUIRE_NOTHROW( one_arg(an_argument) );
    
    
    
    dyn_method two_args(TST_CLS, &dyn_method_tester::add);
    
    
    REQUIRE_THROWS_AS( two_args(), wrong_num_args );
    REQUIRE_THROWS_AS( two_args(1.0), wrong_num_args );
    
    list<dyn_holder> two_arguments;
    two_arguments.emplace_back(6);
    double num=10.0; //keeping carefull controll of the type
    two_arguments.emplace_back(num);
    
    ret_data2=two_args(two_arguments);
    REQUIRE( *ret_data2.cast<double>() == Approx(16.0) );
    REQUIRE_NOTHROW( two_args(6,10.0) );
}

//a special test class to test the dyn_func
class dyn_func_tester
{
    int my_val;
public:
    dyn_func_tester(int _val)
    {
        my_val=_val;
    }

    int operator()(int a, int b)
    {
        return a+b-my_val;
    }
};

int func_tester_func(double a)
{
    return int(a+3);
}

TEST_CASE( "test type_erasure dyn_func", "[dyn_func][dyn_holder]" ) 
{
    dyn_func tster(func_tester_func);
    auto ret_data=tster(3.0);
    REQUIRE(*ret_data.cast<int>()==6);
    
    dyn_func_tester TST_CLS(3);
    dyn_func tster2( TST_CLS, &dyn_func_tester::operator());
    ret_data=tster2(5,3);
    REQUIRE(*ret_data.cast<int>()==5);
    REQUIRE_THROWS_AS( tster(), wrong_num_args );
    
    dyn_func tster3( [](int a)->float{return a+1;} );
}

