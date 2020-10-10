
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

This file defines AST visitors that are used elsewhere.
This is different than basic_AST_visitors, which are desinged to "process" the AST.
*/

#ifndef USEFUL_VISITORS_200509163100
#define USEFUL_VISITORS_200509163100

#include "UTF8.hpp"
#include "AST_visitor.hpp"
#include "cyth_AST.hpp"

class expression_canbe_referenced : public AST_visitorTree
// this visitor is aplied to exressions to find if that expresion can be referenced
{
private:
    bool do_children; // default is false. Set to true on way down if needed.
public:
    bool can_be_referenced; // default is false.
    // expression only needs to be refined if can_be_referenced can be true

    expression_canbe_referenced()
    {
        do_children = false;
        can_be_referenced = false;
    }

    bool apply_to_children() override
        { return do_children; }

    std::shared_ptr< AST_visitor_base > make_child(int number)
    {
        return std::make_shared<expression_canbe_referenced>();
    }



    void varReferance_down(varReferance_expression_AST_node* varRefExp) override
        { can_be_referenced = true; }

    void accessorExp_down(accessor_expression_AST_node* accessorExp) override
        { can_be_referenced = true; }


    void ParenExpGrouping_down(ParenGrouped_expression_AST_node* parenGroupExp) override
        { do_children = true; }

    void ParenExpGrouping_up(ParenGrouped_expression_AST_node* parenGroupExp, AST_visitor_base* expChild_visitor) override
        {
            auto exp_vstr_pnt = dynamic_cast<expression_canbe_referenced*>( expChild_visitor );
            can_be_referenced = exp_vstr_pnt->can_be_referenced;
        }

};



#endif // USEFUL_VISITORS_200509163100

