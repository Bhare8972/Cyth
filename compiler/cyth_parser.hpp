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

This file defines the Cyth syntax and produces the lexer
*/

#ifndef CYTH_PARSER_170211203515
#define CYTH_PARSER_170211203515

#include <list>

#include "cyth_AST.hpp"
#include "parser.hpp"

class cython_lexer : public csu::lexer<csu::token_data>
{
private:
    int STMT_id;
    int NEW_BLOCK_id;
    int END_BLOCK_id;
    int EOF_id;

    std::list<long> block_lengths;
    bool expecting_block;

public:
    cython_lexer(lex_func_t _EOF_action, std::shared_ptr< std::vector< std::shared_ptr<csu::DFA_state> > > _state_table,
                std::shared_ptr< std::vector< lex_func_t > > _actions, std::shared_ptr< std::vector< unsigned int> > _lexer_states);

    void set_identifiers(int _STMT_id, int _NEW_BLOCK_id, int _END_BLOCK_id, int _EOF_id);

    void expect_block();

    csu::token_data lex_eatnewline(csu::utf8_string& data, csu::location_span& loc);
    csu::token_data lex_newline(csu::utf8_string& data, csu::location_span& loc);
    csu::token_data lex_EOF( csu::location_span& loc );
};

//need to make this a global. No idea how not too.

class make_cyth_parser
{
private:
    csu::parser_generator cyth_parser_generator;

    int STMT_id;
    int NEW_BLOCK_id;
    int END_BLOCK_id;
    int EOF_id;

public:
    make_cyth_parser(bool do_file_IO=true);

    std::shared_ptr<csu::parser> get_parser(bool do_file_IO=true);
};

#endif
