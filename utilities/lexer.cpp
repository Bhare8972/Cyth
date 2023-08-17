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


this file is a set of utilities and a lexer, designed for lexing text files in UTF-8
*/

#include "lexer.hpp"

using namespace std;
using namespace csu;

//location
location::location()
{
    line = 1;
    column = 0;
}

location::location(utf8_string& _file_name)
{
    file_name = _file_name;
    line = 1;
    column = 0;
}

location::location(const location& RHS)
{
    file_name = RHS.file_name;
    line = RHS.line;
    column=  RHS.column;
}

location_span location::update(utf8_string& input)
{
    location_span ret;
    ret.start = *this;
    for(auto& char_ : input)
    {
        if(char_=="\n")
        {
            line++;
            column = 0;
        }
        else
        {
            column++;
        }
    }
    ret.end = *this;
    return ret;
}

void location::backup(utf8_string& input)
{

    for( int i = input.get_length()-1; i>=0; i-- )
    {
        auto& char_ = input[i];
        if(char_=="\n")
        {
            line--;
            column = 0;
        }
        else if( column>0 )
        {
            column--;
        }
    }

}

location& location::operator=(const location& other)
{
    line = other.line;
    column = other.column;
    file_name = other.file_name;
    return *this;
}

bool location::operator<(const location& LHS) const
{
    return (line<LHS.line) or (line==LHS.line and column<LHS.column);
}

bool location::operator==(const location& LHS) const
{
    return (line==LHS.line) and (column==LHS.column);
}

bool location::operator<=(const location& LHS) const
{
    return ((*this)<LHS) or ((*this)==LHS);
}

bool location::operator>(const location& LHS) const
{
    return not ((*this)<=LHS);
}

bool location::operator>=(const location& LHS) const
{
    return not ((*this)<LHS);;
}

ostream& csu::operator<<(ostream& os, const location& dt)
{
    os << "line " << dt.line << ":" << dt.column << '(' << dt.file_name << ')';
    return os;
}

//location_span
location_span::location_span(const location& _start, const location& _end)
{
    start=_start;
    end=_end;
}

location_span::location_span(const location_span& RHS)
{
    start=RHS.start;
    end=RHS.end;
}

location_span::location_span(){}

location_span csu::operator+( const location_span& LHS, const location_span& RHS)
{
    location start = LHS.start;
    if( LHS.start > RHS.start  )
    {
        start = RHS.start;
    }

    location end = RHS.end;
    if(LHS.end > RHS.end)
    {
        end = LHS.end;
    }

    return location_span(start, end);
}

location_span csu::operator+( location_span& LHS, location_span& RHS)
{
    location start = LHS.start;
    if( LHS.start > RHS.start  )
    {
        start = RHS.start;
    }

    location end = RHS.end;
    if(LHS.end > RHS.end)
    {
        end = LHS.end;
    }

    return location_span(start, end);
}


bool location_span::is_comparible(const location_span& LHS)
{
    bool RHS_in_one_file = start.file_name == end.file_name;
    bool LHS_in_one_file = LHS.start.file_name == LHS.end.file_name;
    if( RHS_in_one_file and LHS_in_one_file )
    {
        if( start.file_name==LHS.start.file_name )
        {
            return true;
        }
    }
    return false;
}

bool location_span::strictly_GT(const location_span& LHS)
{
    return start>LHS.end;
}

bool location_span::strictly_LT(const location_span& LHS)
{
    return end<LHS.start;
}

ostream& csu::operator<<(ostream& os, const location_span& dt)
{
    os << "from " << dt.start << " to " << dt.end;
    return os;
}

//ring buffer
ring_buffer::ring_buffer( istream& fin_) ://:fin(fin_.rdbuf())
    fin(fin_)
{
    has_loaded_EOF=false;
    has_read_EOF=false;
    length_read=0;
    length_loaded=0;
    n_nodes=1;

    ring_buffer_node* new_node=new ring_buffer_node();
    new_node->next=new_node;
    new_node->previous=new_node;

    start_node=new_node;
    end_node=new_node;
    empty_node=new_node;

    add_nodes(2000);
    load_data();
}

ring_buffer::~ring_buffer()
{
    auto current_node=start_node;
    current_node->previous->next = 0;
    while(current_node->next != 0)
    {
        auto next_node = current_node->next;
        delete current_node;
        current_node = next_node;
    }
}

void ring_buffer::add_nodes(int n_nodes_)
//create extra empty nodes after the empty node
{
    ring_buffer_node* new_node=new ring_buffer_node();
    new_node->previous = empty_node;
    auto final_node = empty_node->next;
    empty_node->next=new_node;

    auto previous_node=new_node;
    for(int i=0; i<n_nodes_; i++)
    {
        new_node=new ring_buffer_node();
        previous_node->next = new_node;
        new_node->previous = previous_node;
        previous_node = new_node;
    }

    previous_node->next = final_node;
    final_node->previous = previous_node;
    n_nodes += n_nodes_;
}

void ring_buffer::load_data()
{
    if(has_loaded_EOF)
    {
        return;
    }
    if( not fin.good() )
    {
        has_loaded_EOF=true;
        return;
    }
    while( (empty_node->next != start_node) and fin.good())
    {
        auto A = code_point(fin);
        empty_node->charector = A;
        if(empty_node->charector.is_empty())
        {
            has_loaded_EOF=true;
        }
        else
        {
            empty_node=empty_node->next;
            length_loaded++;
        }
    }

    if( not fin.good())
    {
        has_loaded_EOF=true;
    }
}

code_point ring_buffer::next()
//return next charector, without 'reading' it
{
    load_data();
    if(end_node==empty_node)
    {
        return code_point();
    }
    return end_node->charector;
}

bool ring_buffer::next_is_EOF()
//return if the next codepoint is EOF (as next() won't necisarily say)
{
    if(end_node==empty_node)
    {
        load_data();
        if(end_node==empty_node and has_loaded_EOF)
        {
            return true;
        }
    }
    return false;
}

code_point ring_buffer::read()
//read and return next charector
{
    if(end_node==empty_node)
    {
        load_data();
        if(end_node==empty_node)
        {
            if(has_loaded_EOF)
            {
                has_read_EOF=true;
            }
            return code_point();
        }
    }

    auto next_char=next();
    end_node=end_node->next;
    length_read++;
    if(end_node==empty_node and has_loaded_EOF)
    {
        has_read_EOF=true;
    }
    return next_char;
}

utf8_string ring_buffer::get_string()
//return a string representing all read nodes
{
    utf8_string ret(length_read);
    auto node=start_node;
    while(node!=end_node)
    {
        ret.append(node->charector);
        node=node->next;
    }
    return ret;
}

utf8_string ring_buffer::reset_string()
//set start_node to end_node, and return string that was read
{
    auto ret=get_string();
    start_node=end_node;//->previous;
    length_loaded-=length_read;
    length_read=0;
    return ret;
}

void ring_buffer::backup(int N)
//backup end node N places
{
    for(int i=0; i<N; i++)
    {
        if(end_node==start_node)
        {
            start_node=start_node->previous;
            length_read=1;
            length_loaded++;
        }
        end_node=end_node->previous;
        length_read--;
    }
    has_read_EOF=false;
}

void ring_buffer::put(utf8_string& data)
//put data at end of buffer
{
    if(data.get_length()==0) return;

    if( (n_nodes-length_loaded)<= data.get_length())//check to see if we have enough space
    {
        add_nodes(data.get_length()-(n_nodes-length_loaded)+1);
    }

    for(auto& charector : data)
    {
        empty_node->charector=charector;
        empty_node = empty_node->next;
        length_loaded++;
    }

    has_read_EOF=false;
}

void ring_buffer::insert(utf8_string& data)
//place data before start_node, then move start_node backwards
// throws error if any chars read
{
    if(length_read != 0 )
    {
        throw gen_exception("RING BUFFER ERROR: INSEART CALLED WHEN READ !=0");
    }

    unsigned int L = data.get_length();
    if(L==0) return;

    if( (n_nodes-length_loaded)<= L)//check to see if we have enough space
    {
        add_nodes(L-(n_nodes-length_loaded)+1);
    }

    // backup start by L
    for( int i=0; i<L; i++)
    {
        start_node = start_node->previous;
    }
    end_node = start_node;

    // now inject the charectors
    auto working_node = start_node;
    for(auto& charector : data)
    {
        working_node->charector = charector;
        working_node = working_node->next;
        length_loaded++;
    }


}
/*
//place data before end_node
{
    if(data.get_length()==0) return;

    //create new nodes and insert charectors
    auto previous_node=end_node->previous;
    auto before_data=previous_node;
    for(auto& charector : data)
    {
        ring_buffer_node* new_node=new ring_buffer_node();
        new_node->charector=charector;
        previous_node->next=new_node;
        new_node->previous=previous_node;
        previous_node=new_node;
        length_loaded++;
    }
    previous_node->next=end_node;
    end_node->previous=previous_node;
    end_node=before_data->next;
    has_read_EOF=false;
}*/

void ring_buffer::reject()
//reject the read string. back end_node to start_node
{
    end_node=start_node;
    length_read=0;

    has_read_EOF=false;
}

