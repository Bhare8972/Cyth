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


This is a set of classes and functions for use of UTF-8.  Will need to be exanded to covert to and from UTF-16 and UTF-32*/


#include "UTF8.hpp"
#include "gen_ex.h"

using namespace csu;


//need to convert to and from UTF-32
static const uint32_t firstByteMark[4] = { 0x00, 0xC0, 0xE0, 0xF0};
static const uint32_t offsetsFromUTF8[4] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL};

//begin code_point methods
code_point::code_point()
{
    code_units=NULL;
}

code_point::code_point( const code_point& rhs)
{
    if(rhs.code_units==0)
    {
        code_units=0;
        return;
    }

    int l=rhs.NUnits();
    code_units=new uint8_t[l];
    for(int x=0; x<l; x++)
    {
        code_units[x]=rhs[x];
    }
}

code_point::code_point( const uint32_t _UTF32)
//create a UTF-8 codepoint from UTF 32. Not 100% sure this is correct. will throw general exception if the number is larger than 1FFFFF
{
    auto UTF32=_UTF32;
    code_units=0;

    unsigned short bytesToWrite = 0;
    const uint32_t byteMask = 0xBF;
    const uint32_t byteMark = 0x80;

    //get number of code units
    if(UTF32 < 0x80)
    {
        bytesToWrite = 1;
    }
    else if (UTF32 < 0x800)
    {
        bytesToWrite = 2;
    }
    else if (UTF32 < 0x10000)
    {
        bytesToWrite = 3;
    }
    else if (UTF32 <= 0x200000)
    {
        bytesToWrite = 4;
    }
    else
    {
        throw gen_exception("32 bit number is too large to convert to UTF 8");
        return;
    }
    code_units=new uint8_t[bytesToWrite];

    uint8_t* target=code_units + bytesToWrite;

    //insert data into the UTF-8 code units
    switch (bytesToWrite)
    { /* note: everything falls through. */
        case 4: *(--target) = static_cast<uint8_t>((UTF32 | byteMark) & byteMask); UTF32 >>= 6;
        case 3: *(--target) = static_cast<uint8_t>((UTF32 | byteMark) & byteMask); UTF32 >>= 6;
        case 2: *(--target) = static_cast<uint8_t>((UTF32 | byteMark) & byteMask); UTF32 >>= 6;
        case 1: *(--target) = static_cast<uint8_t> (UTF32 | firstByteMark[bytesToWrite-1]);
    }
}

code_point::code_point(std::istream& input_stream)
//read a codepoint from an input stream. Assume stream is UTF8
{
    code_units=0;

    int first_char;
    first_char=input_stream.get();
    if(first_char==EOF)
    {
        //we have reached end-of-file.
        //leave codepoint empty, hope someone catches it
        return;
    }

    int NUNITS=NUnits(first_char);
    code_units=new uint8_t[NUNITS];
    code_units[0]=first_char;

    int a=1;
    switch(NUNITS)
    {
        case 4: code_units[a]=input_stream.get(); a++;
        case 3: code_units[a]=input_stream.get(); a++;
        case 2: code_units[a]=input_stream.get(); a++;
    }
}

code_point::code_point( const std::initializer_list<uint8_t> units)
{
    code_units=new uint8_t[units.size()];
    auto init_iter=units.begin();
    for(uint x=0; x<units.size(); x++)
    {
        code_units[x]=*init_iter;
        init_iter++;
    }
}

code_point::~code_point()
{
    if(code_units)
        delete[] code_units;
}

bool code_point::operator== (const code_point& rhs) const
//equals opperator
{
    int l=NUnits();
    if(l==rhs.NUnits())
    {
        for(int i=0; i<l; i++)
        {
            if(rhs[i]!=code_units[i])
                return false;
        }
        return true;
    }
    return false;
}

bool code_point::operator== (const utf8_string& rhs) const
//equals opperator, compare to a UTF8_string
{
    if( rhs.get_length() != 1)
    {
        return false;
    }

    const code_point& rhs_pnt=rhs[0];

    int l=NUnits();
    if(l==rhs_pnt.NUnits())
    {
        for(int i=0; i<l; i++)
        {
            if(rhs_pnt[i]!=code_units[i])
                return false;
        }
        return true;
    }
    return false;
}

code_point& code_point::operator=(const code_point& other)
// copy assignment
{
    if (this != &other) // self-assignment check expected
    {
        int l=other.NUnits();
        if(l==0)
        {
            if(code_units)
                delete[] code_units;
            code_units=0;
            return *this;
        }

        if (NUnits() != l)/* storage cannot be reused (e.g. different sizes) */
        {
            if(code_units)
                delete[] code_units;
            code_units = new uint8_t[l];
        }

        for( int i=0; i<l; i++ )
        {
            code_units[i]=other[i];
        }

    }
    return *this;
}

code_point& code_point::operator=(code_point&& other)
// move assignment
{
    if(code_units)
        delete[] code_units;        // delete this storage
    code_units = other.code_units;  // move
    other.code_units = NULL; // leave moved-from in valid state
    return *this;
}

const uint8_t code_point::operator[](uint8_t idx) const
//accsess a code-point in the code-unit. throws a gen_exception if idx is out of range
{
    if(idx>=NUnits())
    {
        throw gen_exception("index out of range");
    }
    return code_units[idx];
}

int code_point::from_str(std::string input)
//construct new code point from the first UTF-8 point in a CPP string
//is not a constructor, becouse we want to return number of units consumed
{
    int l=code_point::NUnits(input[0]);
    if(l==0)
        return 0;
    if( l!=NUnits() )
    {
        delete[] code_units;
        code_units=new uint8_t[l];
    }
    for(int x=0; x<l; x++)
    {
        code_units[x]=input[x];
    }
    return l;
}

int code_point::NUnits() const
//return number of code-units in this code-point
{
    if( code_units==0)
        return 0;
    else if( code_units[0]<=0x7f )
        return 1;
    else if( code_units[0]>=0xC2 and code_units[0]<=0xDF)
        return 2;
    else if( code_units[0]>=0xE0 and code_units[0]<=0xEF)
        return 3;
    else if( code_units[0]>=0xF0 and code_units[0]<=0xF4)
        return 4;

    return 4;
}

int code_point::NUnits(uint8_t start_char)
//number of code-units in the code-point starting with start_char
{
    if( start_char<=0x7f )
        return 1;
    else if( start_char>=0xC2 and start_char<=0xDF)
        return 2;
    else if( start_char>=0xE0 and start_char<=0xEF)
        return 3;
    else if( start_char>=0xF0 and start_char<=0xF4)
        return 4;

    return 4;
}

bool code_point::in(const utf8_string& data ) const
//check if this charector is in data
{
    uint L=data.get_length();
    for(uint i=0; i<L; i++)
    {
        if((*this)==data[i])
        {
            return true;
        }
    }
    return false;
}

uint32_t code_point::to_UTF32() const
//convert this charector to UTF32. throw gen_exception if it is empty
{
    uint32_t ret=0;
    int N_units=NUnits();
    if(N_units==0)
    {
        throw gen_exception("cannot get UTF 8 value of empty code point");
    }

    uint8_t* source=code_units;
    //all cases fall through, on purpose.
    switch(N_units)
    {
        case 4: ret += *source++; ret <<= 6;
        case 3: ret += *source++; ret <<= 6;
        case 2: ret += *source++; ret <<= 6;
        case 1: ret += *source;
    }
    ret -= offsetsFromUTF8[N_units-1];
    return ret;
}

bool code_point::is_empty() const
{
    return code_units==0;
}

void code_point::put(std::ostream& out_stream)
{
    int L=NUnits();
    if(L==0)
    {
        throw gen_exception("cannot send an empty code_point to a stream");
    }
    for(int i=0; i<L; i++)
    {
        out_stream.put(code_units[i]);
    }
}

//end code_point methods
//begin code_point friend functions

std::ostream& csu::operator<<(std::ostream& os, const code_point& dt)
//throws gen_exception if code_point is empty
{
    int L=dt.NUnits();
    if(L==0)
    {
        throw gen_exception("cannot send an empty code_point to a stream");
    }
    for(int i=0; i<L; i++)
    {
        os<<dt.code_units[i];
    }
    return os;
}

std::istream& csu::operator>>(std::istream& is, code_point& dt)
{
    if(dt.code_units!=0)
    {
        delete[] dt.code_units;
        dt.code_units=0;
    }

    uint8_t first_char;
    is>>first_char;
    int NUNITS=dt.NUnits(first_char);
    dt.code_units=new uint8_t[NUNITS];
    dt.code_units[0]=first_char;
    int a=1;
    switch(NUNITS)
    {
        case 4: is>>dt.code_units[a]; a++;
        case 3: is>>dt.code_units[a]; a++;
        case 2: is>>dt.code_units[a]; a++;
    }
    return is;
}

bool csu::operator<(const code_point& lhs, const code_point& rhs)
{
    uint8_t lhs_l=lhs.NUnits();
    uint8_t rhs_l=rhs.NUnits();

    if(lhs_l<rhs_l)
        return true;
    else if(lhs_l>rhs_l)
        return false;

    for(uint8_t i=0; i<lhs_l; i++)
    {
        if(lhs.code_units[i]<rhs.code_units[i])
        {
            return true;
        }
        else if(lhs.code_units[i]>rhs.code_units[i])
        {
            return false;
        }
    }
    return false; //they are equal
}

bool csu::operator>(const code_point& lhs, const code_point& rhs)
{
    uint8_t lhs_l=lhs.NUnits();
    uint8_t rhs_l=rhs.NUnits();

    if(lhs_l<rhs_l)
        return false;
    else if(lhs_l>rhs_l)
        return true;

    for(uint8_t i=0; i<lhs_l; i++)
    {
        if(lhs.code_units[i]<rhs.code_units[i])
        {
            return false;
        }
        else if(lhs.code_units[i]>rhs.code_units[i])
        {
            return true;
        }
    }
    return false; //they are equal
}

//begin utf8_string


//constructors
utf8_string::utf8_string(const utf8_string& RHS)
{
    length=RHS.length;
    capacity=RHS.length;
    points=new code_point[RHS.length];

    for( int i=0; i<RHS.length; i++ )
    {
        points[i]=RHS[i];
    }
}

utf8_string::utf8_string(uint8_t _capacity)
{
    length=0;
    capacity=0;
    points=0;
    if( _capacity>0)
        resize(_capacity);
}

utf8_string::utf8_string(const std::string& input)
{
    length=0;
    capacity=0;
    points=0;
    from_cpp_string(input);
}

utf8_string::utf8_string(const char* input)
{
    std::string str_in(input);
    length=0;
    capacity=0;
    points=0;
    from_cpp_string(str_in);
}

utf8_string::utf8_string(code_point& input)
{
    length=1;
    capacity=1;
    points=new code_point[1];

    points[0]=input;
}

utf8_string::~utf8_string()
{
    if( points )
        delete[] points;
    points=0;
}

//to and from cpp_string
void utf8_string::from_cpp_string(const std::string& input)
//replaces content with content in input. Throws gen_exception if input is not UTF-8
{
    uint8_t position=0;
    uint8_t new_length=0;
    while( position<input.length() )
    {
        uint8_t NUnits=code_point::NUnits(input.substr(position)[0]);
        if(NUnits==0)
            throw gen_exception("input is not UTF-8. B");
        if( NUnits+position>input.length() )
            throw gen_exception("input is not UTF-8. A");
        position+=NUnits;
        new_length+=1;
    }

    reserve(new_length);
    length=new_length;

    position=0;

    for(uint8_t i=0; i<length and position<input.length(); i++)
    {
        position+=points[i].from_str(input.substr(position));
    }
}

std::string utf8_string::to_cpp_string()
//return a utf-8 cpp string with the same content as this string
{
    uint8_t cpp_length=0;
    for(uint8_t i=0; i<length; i++)
    {
        cpp_length+=points[i].NUnits();
    }

    std::string ret(cpp_length, ' ');

    uint8_t position=0;
    for(uint8_t i=0; i<length; i++)
    {
        uint8_t NUnits=points[i].NUnits();

        switch (NUnits)
        {
            case 4:
            {
                ret[position+3]=points[i][3];
            }
            case 3:
            {
                ret[position+2]=points[i][2];
            }
            case 2:
            {
                ret[position+1]=points[i][1];
            }
            case 1:
            {
                ret[position]=points[i][0];
            }
        }
        position+=NUnits;
    }
    return ret;
}

//size operations
uint8_t utf8_string::get_capacity()
{
    return capacity;
}

uint utf8_string::get_length() const
{
    return length;
}

void utf8_string::resize(uint8_t _capacity)
//resizes string to _max_length. Discarding points if _max_length is smaller than current length
{
    if(_capacity==capacity)
        return;

    code_point* new_points=new code_point[_capacity];
    uint8_t L=_capacity;
    if(length<_capacity)
        L=length;

    for(uint8_t i=0; i<L; i++)
    {
        new_points[i]=points[i];
    }
    length=L;
    capacity=_capacity;
    if( points )
        delete[] points;
    points=new_points;
}

void utf8_string::reserve(uint8_t _capacity)
//resizes string to at least_max_length.
{
    if(_capacity<=capacity)
        return;

    code_point* new_points=new code_point[_capacity];

    for(uint8_t i=0; i<length; i++)
    {
        new_points[i]=points[i];
    }
    capacity=_capacity;
    if( points )
        delete[] points;
    points=new_points;
}

//element acsses
code_point& utf8_string::operator[] (size_t pos)
//accsess code_point at pos. Throws gen_exception if pos is beyond length of string
{
    if( pos >= length )
        throw gen_exception("pos is larger than string length");
    return points[pos];
}

utf8_string utf8_string::slice(uint8_t start, uint8_t stop)
//returns a UTF8 string with codepoints between start and stop, include start. Throws gen_exception if stop is less than start
// or stop is larger than string
{
    if(stop<start)
    {
        throw gen_exception("stop is less than start");
    }
    else if(stop>length)
    {
        throw gen_exception("stop is beyond length of string");
    }

    uint8_t L=stop-start;
    utf8_string ret(L);
    for(uint8_t i=start; i<stop; i++)
    {
        ret.append(points[i]);
    }
    return ret;
}


const code_point& utf8_string::operator[] (size_t pos) const
//accsess code_point at pos. Throws gen_exception if pos is beyond length of string
{
    if(pos>=length)
        throw gen_exception("pos is larger than string length");
    return points[pos];
}

//iterators
utf8_string::iterator utf8_string::begin() { return &points[0]; }
utf8_string::iterator utf8_string::end() { return &points[length]; }

//modifiers
utf8_string& utf8_string::operator+= (const std::string& str)
{
    append(str);
    return *this;
}

utf8_string& utf8_string::operator+= (const utf8_string& data)
{
    append(data);
    return *this;
}

void utf8_string::append(const std::string& str)
{
    utf8_string data(str);
    append(data);
}

void utf8_string::append(const char* str)
{
    utf8_string data(str);
    append(data);
}

void utf8_string::append(const utf8_string& data)
{
    uint8_t new_size=length+data.length;
    reserve(new_size);
    for(int i=0; i<data.length; ++i)
    {
        points[i+length]=data.points[i];
    }
    length=new_size;
}

void utf8_string::append(const code_point& data)
{
    reserve(length+1);
    points[length]=data;
    length+=1;
}

void utf8_string::append(const uint32_t data)
{
    reserve(length+1);
    points[length]=code_point(data);
    length+=1;
}

//overloaded operators
bool utf8_string::operator == (const utf8_string& rhs) const
//equals opperator
{
    if(length==rhs.length)
    {
        for(uint8_t i=0; i<length; i++)
        {
            if(not (rhs[i]==points[i]) )
                return false;
        }
        return true;
    }
    return false;
}

utf8_string& utf8_string::operator=(const utf8_string& other)
// copy assignment
{
    if (this != &other) // self-assignment check expected
    {
        reserve(other.length);

        for( int i=0; i<other.length; i++ )
        {
            points[i]=other[i];
        }
        length=other.length;

    }
    return *this;
}

utf8_string& utf8_string::operator=(utf8_string&& other)
// move assignment
{
    if(points)
        delete[] points;        // delete this storage
    points = other.points;  // move
    length=other.length;
    capacity=other.capacity;
    other.points = 0; // leave moved-from in valid state
    other.length=0;
    other.capacity=0;
    return *this;
}

//end utf8_string methods
//begin uft8_friend methods


std::ostream& csu::operator<<(std::ostream& os, const utf8_string& str)
{
    for(int i=0; i<str.length; i++)
    {
        os<<str.points[i];
    }
    return os;
}

utf8_string csu::operator+(const utf8_string& LHS, const utf8_string& RHS)
{
    utf8_string ret=LHS;
    ret.append(RHS);
    return ret;
}

bool csu::operator< (const utf8_string& lhs, const utf8_string& rhs)
{
    if(lhs.length<rhs.length) return true;
    if(lhs.length>rhs.length) return false;

    for( int i=0; i<lhs.length; i++ )
    {
        if( lhs.points[i]<rhs.points[i] ) return true;
        if( lhs.points[i]>rhs.points[i] ) return false;
    }

    return true;
}

