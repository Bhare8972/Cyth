/*
Copyright 2015 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


This is a special set of classes designed to hold ANY type.
Inspired by any.cpp by Christipher Diggins.
*/

#include <memory>
#include <exception>
#include <list>
#include <type_traits>
#include <tuple>

namespace csu{ //cyth standard utillities namespace

//need to add more info in this exception
class bad_type_erasure_cast : public std::exception
//a generalized exception class. Does not take charge of deleteing char*
{
public:

    virtual const char* what() const throw()
    {
        return "bad type_erasure cast";
    }
};

class method_not_implemented : public std::exception
//a generalized exception class. Does not take charge of deleteing char*
{
public:

    virtual const char* what() const throw()
    {
        return "method calling is not implemented";
    }
};

//TODO: dynamic_cast and ostream support
class dyn_holder
//dynamic holder class. Designed to hold any type.
//unlike CDiggens any, however, it is not optimized for small types, but for classes
{
protected:

    class base_policy //base_policy is a wrapper around typed_base_policy
    {
    public:
        virtual dyn_holder call_func(void* cls)=0; //call function with zero arguments
        virtual dyn_holder call_func(void* cls, dyn_holder arguments)=0; //call function with one argument
        virtual dyn_holder call_func(void* cls, std::list<dyn_holder>& arguments)=0; //call function with many arguments
        virtual base_policy* get_policy()=0;

        virtual ~base_policy(){} //virtual destructor
    };

    template<typename T>
    class typed_policy : public base_policy //this class actually holds our type information. Mostly needed for casting
    {
    public:
        virtual dyn_holder call_func(void* cls) //call function with zero arguments
        { throw method_not_implemented(); }
        virtual dyn_holder call_func(void* cls, dyn_holder arguments) //call function with one argument
        { throw method_not_implemented(); }
        virtual dyn_holder call_func(void* cls, std::list<dyn_holder>& arguments) //call function with many arguments
        { throw method_not_implemented(); }

        virtual base_policy* get_policy()
        {
            return dyn_holder::get_policy<T>();
        }

        virtual ~typed_policy(){}
    };

    template<typename T>
    static base_policy* get_policy()
    {
        static typed_policy<T> policy;
        return &policy;
    }

    class empty_holder //signifies that the class is empty
    {
    };

    std::shared_ptr<void> data;
    base_policy* type_information;

public:

    //constructors
    dyn_holder()
    {
        data=0;
        type_information=get_policy<empty_holder>();
    }

    template<typename T>
    dyn_holder(const T& new_data)
    {
        auto new_data2=std::make_shared<T>(new_data);
        data=std::static_pointer_cast<void>(new_data2);
        type_information=get_policy<T>();
    }

    template<typename T>
    dyn_holder(std::shared_ptr<T> new_data)
    {
        data=std::static_pointer_cast<void>(new_data);
        type_information=get_policy<T>();
    }

    dyn_holder(const dyn_holder& new_data )
    {
        data=new_data.data;
        type_information=new_data.type_information;
    }

    //virtual destructor for future overloading
    virtual ~dyn_holder() {}

    //assignment opperators
    template<typename T>
    dyn_holder& operator=(const T& new_data)
    {
        auto new_data2=std::make_shared<T>(new_data);
        data=std::static_pointer_cast<void>(new_data2);
        type_information=get_policy<T>();
        return *this;
    }

    template<typename T>
    dyn_holder& operator=(std::shared_ptr<T> new_data)
    {
        data=std::static_pointer_cast<void>(new_data);
        type_information=get_policy<T>();
        return *this;
    }

    dyn_holder& operator=(const dyn_holder& new_data )
    {
        data=new_data.data;
        type_information=new_data.type_information;
        return *this;
    }

    //the hard bit: going back!
    template<typename T>
    std::shared_ptr<T> cast() const
    {
        if(get_policy<T>() != type_information->get_policy())
        {
            throw bad_type_erasure_cast();
        }
        return std::static_pointer_cast<T>(data);
    }

    template<typename T>
    void cast(std::shared_ptr<T>& out) const
    {
        out=cast<T>();
    }

    template<typename T>
    void cast(T& out) const
    {
        std::shared_ptr<T> R=cast<T>();
        out=*R;
    }

    //utilities
    bool empty() const
    {
        return data==0;
    }

    void reset()
    {
        data=0;
        type_information=get_policy<empty_holder>();
    }

    bool compatible(const dyn_holder& RHS) const
    {
        return type_information->get_policy()==RHS.type_information->get_policy();
    }
};

class wrong_num_args : public std::exception
//a generalized exception class. Does not take charge of deleteing char*
{
public:

    virtual const char* what() const throw()
    {
        return "wrong number of arguments for method";
    }
};


template<typename ... following_types>
class type_holder
//this struct is simple for keeping a hold of multiple types
{
public:
};

template<typename first_type, typename ... following_types>
class type_holder<first_type, following_types...> : type_holder<following_types...>
//this struct is simple for keeping a hold of multiple types
{
public:
    type_holder<following_types ...> get_following() {  return type_holder<following_types...>();}
};

//special structs for unwraping function arguments
template<int N>
struct apply_wrap
{
    template<typename method_class, typename method_return, typename... method_args, typename unpackedArg, typename... ArgsToUnpack, typename... UnpackedArgs>
    static dyn_holder deravel(void* cls, method_return (method_class::*func)(method_args...), type_holder<unpackedArg, ArgsToUnpack...> t, std::list<dyn_holder>::iterator next_arg,
                                 std::list<dyn_holder>::iterator end_args, UnpackedArgs... args )
    {
        if(next_arg==end_args) throw wrong_num_args();

        unpackedArg new_arg;
        next_arg->cast(new_arg);

        ++next_arg;

        return apply_wrap<N-1>::deravel(cls, func, t.get_following(), next_arg, end_args, args... , new_arg);
    }
};

template<>
struct apply_wrap<1>
{
    template<typename method_class, typename method_return, typename...method_args, typename unpackedArg, typename... UnpackedArgs>
    static dyn_holder deravel(void* cls, method_return (method_class::*func)(method_args...), type_holder<unpackedArg> t, std::list<dyn_holder>::iterator next_arg,
                                 std::list<dyn_holder>::iterator end_args, UnpackedArgs... args )
    {
        if(next_arg==end_args) throw wrong_num_args();

        unpackedArg new_arg;
        next_arg->cast(new_arg);

        ++next_arg;

        if(next_arg !=end_args) throw wrong_num_args();

        method_class* A=static_cast<method_class*>(cls);
        method_return R=(A->*func)(args..., new_arg);
        return dyn_holder(R);
    }
};

template<>
struct apply_wrap<0>
{
    template<typename method_class, typename method_return, typename...method_args, typename... ArgsToUnpack>
    static dyn_holder deravel(void* cls, method_return (method_class::*func)(method_args...), type_holder<ArgsToUnpack...>& t, std::list<dyn_holder>::iterator next_arg,
                                 std::list<dyn_holder>::iterator end_args)
    {

        if(next_arg !=end_args) throw wrong_num_args();

        method_class* A=static_cast<method_class*>(cls);
        method_return R=(A->*func)();
        return dyn_holder(R);
    }
};


//two extra functions to turn a template pack into a list of dyn_holders
template<typename first_argument_type, typename ... argument_types>
void varargs_to_list(std::list<dyn_holder>& out_list, first_argument_type first_argument, argument_types...arguments)
{
    out_list.push_back(dyn_holder(first_argument));
    varargs_to_list(out_list, arguments...);
}

template<typename first_argument_type>
void varargs_to_list(std::list<dyn_holder>& out_list, first_argument_type first_argument)
{
    out_list.push_back(dyn_holder(first_argument));
}

class dyn_method : public dyn_holder
//this class expands upon dyn_holder, by adding a callible method
{
protected:

    //method policy with return type. This probably doesn't work for no return
    template<typename classT, typename retT, typename...argsT>
    class method_policy : public base_policy
    {
    public:
        retT (classT::*func)(argsT...);

        method_policy(retT (classT::*_func)(argsT...))
        {
            func=_func;
        }

        virtual ~method_policy(){}

        virtual dyn_holder call_func(void* cls) //call function with zero arguments
        {
            return call_deravel_zero<argsT...>(cls);
        }


        template<typename first_arg, typename ... other_args> //one or more arguments to a zero argument function
        dyn_holder call_deravel_zero(void* cls)
        {
            throw wrong_num_args();
        }


        template<typename... other_args, typename std::enable_if<sizeof...(other_args) == 0, bool>::type=0 >
        dyn_holder call_deravel_zero(void* cls) //zero arguments to a zero argument function
        {
            classT* A=static_cast<classT*>(cls);
            retT R=(A->*func)();
            return dyn_holder(R);
        }

        virtual dyn_holder call_func(void* cls, dyn_holder argument) //call function with one argument
        {
            return call_deravel_single<argsT...>(cls, argument);
        }

        //one argument to a one-argument function
        template<typename argT, typename... extra_args, typename std::enable_if<sizeof...(extra_args) == 0, bool>::type=0>
        dyn_holder call_deravel_single(void* cls, dyn_holder argument)
        {
            if(sizeof...(extra_args) !=0) //only given one argument, when need more.
            {
                throw wrong_num_args();
            }

            argT new_arg;
            argument.cast(new_arg);

            classT* A=static_cast<classT*>(cls);
            retT R=(A->*func)(new_arg);
            return dyn_holder(R);
        }

        //one argument to a zero-argument function, or multi-arumetn funtion
        template<typename ... extra_args, typename std::enable_if<sizeof...(extra_args) != 1, bool>::type=0 >
        dyn_holder call_deravel_single(void* cls, dyn_holder argument)
        {
            throw wrong_num_args(); //given one argument, when need 0
        }

        virtual dyn_holder call_func(void* cls, std::list<dyn_holder>& arguments) //call function with many arguments
        {
            //return call_deravel<argsT...>(cls, arguments.begin(), arguments.end());
            type_holder<argsT...> t;
            return apply_wrap<sizeof...(argsT)>::deravel(cls, func, t, arguments.begin(), arguments.end());
        }

        virtual base_policy* get_policy() //we need this to be able to maintain consistancy with dyn_holder
        {
            return dyn_holder::get_policy<classT>();
        }
    };

public:

    dyn_method()
    {
        data=0;
        type_information=0;
    }

    template<typename classT, typename retT, typename...argsT>
    dyn_method( const classT& new_data, retT (classT::*_func)(argsT...) )
    {
        //std::shared_ptr<classT> new_data2( new classT(new_data) );
        auto new_data2=std::make_shared<classT>(new_data);
        data=std::static_pointer_cast<void>(new_data2);
        type_information=new method_policy<classT, retT, argsT...>(_func);
    }

    ~dyn_method()
    {
        if(type_information) delete type_information;
        data=0;
    }

    dyn_holder operator()() //call method with no arguments
    {
        return type_information->call_func(data.get());
    }

    template<typename argument_type>
    dyn_holder operator()(argument_type argument) //call method with one argument
    {
        return type_information->call_func(data.get(), dyn_holder(argument));
    }

    dyn_holder operator()(dyn_holder argument) //call method with one argument
    {
        return type_information->call_func(data.get(), argument);
    }

    dyn_holder operator()(std::list<dyn_holder> arguments) //call method with multiple arguments
    {
        return type_information->call_func(data.get(), arguments);
    }

    template<typename...argument_types>
    dyn_holder operator()(argument_types...args) //call method with multiple arguments
    //a special function to pack arguments into a list
    {
        std::list<dyn_holder> arguments;
        varargs_to_list(arguments, args...);
        return type_information->call_func(data.get(), arguments);
    }

};

/* where to go from here:
combine dyn_holder and dyn_method 
* extend dyn_method to work with non-method functions, and with functions and methods that don't have return types
* extend both of them to support basic operators. (or error otherwise, using enable_if)
* create a new, extended, type that holds a dictionary to multiple methods, creating a python-like type that can hold C++ classes
*/
}//end namespace
