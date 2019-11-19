/*
Copyright 2019 Brian Hare

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file defines the base class for abstract syntax tree visitors, which are essentially operators on the AST
*/

#include "AST_visitor.hpp"
using namespace csu;
using namespace std;


AST_visitorTree::AST_visitorTree()
{
    parent = 0;
    children_initiated = false;
}

AST_visitorTree::AST_visitorTree(AST_visitor_base* _parent)
{
    parent = _parent;
    children_initiated = false;
}

void AST_visitorTree::initiate_children(int number)
{
    number_children = number;

    if( not children_initiated )
    {
        children.reserve(number);

        for(int i=0; i<number; i++)
        {
            children.push_back( make_child(i) );
        }

        children_initiated = true;
    }
}

