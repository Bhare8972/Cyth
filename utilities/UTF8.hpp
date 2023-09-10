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



#include <stdint.h>
#include <string>
#include <iostream>
#include <initializer_list>

#ifndef UTF8_STRING_150829021957
#define UTF8_STRING_150829021957

typedef unsigned int uint;

namespace  csu{

class utf8_string; //for use later

class code_point
//this class defines a single UTF-8 code-point (charector)
{
private:
    uint8_t* code_units;
public:

    friend std::ostream& operator<<(std::ostream& os, const code_point& dt);
    friend std::istream& operator>>(std::istream& is, code_point& dt);
    friend bool operator<(const code_point& lhs, const code_point& rhs);
    friend bool operator>(const code_point& lhs, const code_point& rhs);

    code_point();

    code_point( const code_point& rhs);

    code_point( const uint32_t _UTF32);
    //create a UTF-8 codepoint from UTF 32. Not 100% sure this is correct. will throw general exception if the number is larger than 1FFFFF

    code_point( std::istream& input_stream);
    //read a code_point from a stream, ignoring formating

    code_point( const std::initializer_list<uint8_t> units);
    //create a UTF-8 codepoint from exact bits desired. This is for making 'invalid' codepoints
    //be very carefull with this function. Can EASILY create undefined behavior (see code_point::NUnits)

    ~code_point();

    bool operator == (const code_point& rhs) const;
    //equals opperator return false;

    bool operator == (const utf8_string& rhs) const;
    //equals opperator, compare to a UTF8_string

    code_point& operator=(const code_point& other) ;
    // copy assignment

    code_point& operator=(code_point&& other) ;
    // move assignment

    const uint8_t operator[](uint8_t idx) const ;
    //accsess a code-point in the code-unit. throws a gen_exception if idx is out of range

    int from_str(std::string input);
    //construct new code point from the first UTF-8 point in a CPP string
    //is not a constructor, becouse we want to return number of units consumed

    int NUnits() const;
    //return number of code-units in this code-point
    //returns '4' if first code point is not between 0x0 and 0xF4

    static int NUnits(uint8_t start_char);
    //number of code-units in the code-point starting with start_char
    //returns '4' if first code point is not between 0x0 and 0xF4

    bool in(const utf8_string& data ) const;
    //check if this charector is in data

    uint32_t to_UTF32() const;
    //return this charector encoded as UTF32

    bool is_empty() const;

    void put(std::ostream& out_stream);
    //put this code_point into a stream

}; //end code_point

std::ostream& operator<<(std::ostream& os, const code_point& CP);
std::istream& operator>>(std::istream& is, code_point& dt);

bool operator<(const code_point& lhs, const code_point& rhs);

bool operator>(const code_point& lhs, const code_point& rhs);

class utf8_string
//a string class that implements UTF-8
{
private:
    size_t length;
    size_t capacity;
    code_point* points;

    friend std::ostream& operator<<(std::ostream& os, const utf8_string& dt);
    friend utf8_string operator+(const utf8_string& LHS, const utf8_string& RHS);
    friend bool operator< (const utf8_string& lhs, const utf8_string& rhs);


public:
    typedef code_point* iterator;

    //constructors
    utf8_string(const utf8_string& RHS);

    utf8_string(size_t _capacity=0);

    utf8_string(const std::string& input);

    utf8_string(const char* input);

    utf8_string(code_point& input);

    ~utf8_string();

    //to and from cpp_string
    void from_cpp_string(const std::string& input);
    //replaces content with content in input. Throws gen_exception if input is not UTF-8

    std::string to_cpp_string();
    //return a utf-8 cpp string with the same content as this string

    //size operations
    size_t get_capacity();

    size_t get_length() const;

    void resize(size_t _capacity);
    //resizes string to _max_length. Discarding points if _max_length is smaller than current length

    void reserve(size_t _capacity);
    //resizes string to at least_max_length.

    //element acsses
    code_point& operator[] (size_t pos);
    //accsess code_point at pos. Throws gen_exception if pos is beyond length of string

    utf8_string slice(size_t start, size_t stop);
    //returns a UTF8 string with codepoints between start and stop, include start. Throws gen_exception if stop is less than start
    // or stop is larger than string


    const code_point& operator[] (size_t pos) const;
    //accsess code_point at pos. Throws gen_exception if pos is beyond length of string

    //iterators
    iterator begin();
    iterator end();

    //modifiers
    utf8_string& operator+= (const std::string& str);
    utf8_string& operator+= (const utf8_string& data);
    utf8_string& operator+= (const char* data);

    void append(const std::string& str);
    void append(const char* str);
    void append(const utf8_string& data);
    void append(const code_point& data);
    void append(const uint32_t data);

    //overloaded operators
    //equals opperator
    bool operator == (const utf8_string& rhs) const;
    bool operator == (const char* rhs) const;

    bool operator != (const char* rhs) const;

    utf8_string& operator=(const utf8_string& other);
    // copy assignment

    utf8_string& operator=(utf8_string&& other);
    // move assignment



};//end utf8_string

std::ostream& operator<<(std::ostream& os, const utf8_string& str);
utf8_string operator+(const utf8_string& LHS, const utf8_string& RHS);
bool operator< (const utf8_string& lhs, const utf8_string& rhs);

} //end csu namespace

#endif
