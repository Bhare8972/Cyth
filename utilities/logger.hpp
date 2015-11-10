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

logger class utility
*/

#ifndef LOGGER_151017125247
#define LOGGER_151017125247

#include "gen_ex.h"

#include <iostream>
#include <list>
#include <sstream>
#include <string>


namespace csu{ //cyth standard utilities namespace

//basic utility classes
class logger
{
private:
    std::list<std::string> data;
    friend std::ostream& operator<<(std::ostream& os, const logger& dt);
    
public:
    logger(){}
    
    void operator()()
    {
        data.push_back("\n");
    }
    
    template<typename T, typename... Ts>
    void operator()(T input, Ts... inputs)
    {
        std::stringstream tmp;
        tmp<<input;
        (*this)(inputs...);
    }
    //adds each bit of data to our log
    
    void write(std::string fname);
    //write the data to a file
};
std::ostream& operator<<(std::ostream& os, const logger& str);

} //end csu namespace


#endif
