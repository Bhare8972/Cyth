/*

Copyright 2020 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file defines C-expressions and how they write to file and cleanup
*/

#ifndef C_EXPRESSIONS_201110211200
#define C_EXPRESSIONS_201110211200


#include <memory>
#include <list>

#include "UTF8.hpp"
#include "c_source_writer.hpp"

//TODO: require and check get_C_expression is called exactly once??
//TODO: has a location?

class varType; // you will learn these horrors elsewhere. For now, be content you only know their name.

// make a more generallized expression that doesn't have a C representation!. It throws error if you call get_C_expression

class C_expression
{
    std::list< std::shared_ptr<C_expression> > child_expressions;

public:
    bool has_output_ownership; // if true(default), then cleanup will need to call destructor.
    //If this is true, outer expressions are allowed to steal memory, call __moved__, and set this to be false.
    // this seems to combine two concepts: if this exp can be the RHS of a C copy (=) and if this expression requires a destructor
    // it is current undefined what to do if these two concepts are not both false or both true.
    //    best to avoid this situation
    // write cleanup will write a error if can_be_referenced is true but has_output_ownership is false

    bool can_be_referenced;

    std::shared_ptr<varType> cyth_type; // cyth type this C-expression represents??

    virtual csu::utf8_string get_C_expression()=0; // this may be called multiple times
    virtual void cleanup_this( Csource_out_ptr source_fout ){} // overwrite this to define how expression is cleaned-up

    void write_cleanup( Csource_out_ptr source_fout );  // this is called once, after which get_C_expression is not valid
    void add_cleanup_child( std::shared_ptr<C_expression> child );
};
typedef std::shared_ptr<C_expression> C_expression_ptr;

class simple_C_expression : public C_expression
// c-expression that doesn't call destructor. everything set externally.
{
public:
    csu::utf8_string exp;

// nothing is set
    //simple_C_expression(){}

    simple_C_expression( csu::utf8_string _exp, std::shared_ptr<varType> _type,
                        bool _can_be_referenced, bool _has_output_ownership);

    csu::utf8_string get_C_expression(){ return exp; }
};


class owned_name : public C_expression
// c-expression that owns a name to be destructed
{
    csu::utf8_string name;
public:
    owned_name(std::shared_ptr<varType> __cyth_type, csu::utf8_string __name);

    csu::utf8_string get_C_expression(){ return name; }
    void cleanup_this( Csource_out_ptr source_fout );
};

#endif
