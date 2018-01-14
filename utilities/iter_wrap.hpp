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

This is a generic wrapper over any iterable container. Allows for psuedo-random indexing if we have bi-directional iterator.
Mainly for use with lists
*/

#ifndef ITER_WRAP_151128061735
#define ITER_WRAP_151128061735

#include <stdexcept>
#include <iterator>

//utilities for defining languege
namespace csu{ //cyth standard utilities namespace

template< typename container>
class iter_wrap
{
public:
    typedef typename container::iterator iterator;
    typedef decltype(*iterator()) return_type;

private:
    unsigned int location;
    iterator present_location_iter;
    iterator end_iter;
    int num_elements;

public:

    iter_wrap(container& cont)
    {
        location=0;
        present_location_iter=cont.begin();
        end_iter=cont.end();
    }

    iter_wrap(const iter_wrap<container>& IW)
    {
        location=IW.location;
        present_location_iter=IW.present_location_iter;
        end_iter=IW.end_iter;
    }

    iter_wrap(iterator beginning, unsigned int num_elements_)
    {
        location=0;
        num_elements=num_elements_;
        present_location_iter=beginning;
        for(unsigned int x=0; x<num_elements; x++) ++beginning;//increment beginning to end
        end_iter=beginning;
    }

    iter_wrap(unsigned int num_elements_, iterator ending)
    {
        num_elements=num_elements_;
        location=0;
        end_iter=ending;
        for(unsigned int x=0; x<num_elements; x++) --ending;//decrement ending to beginning
        present_location_iter=ending;
    }

    iter_wrap(const iterator& beginning, const iterator& ending)
    {
        location=0;
        end_iter=ending;
        present_location_iter=beginning;
        num_elements=distance(beginning, ending);

        if( num_elements!=0 and present_location_iter==end_iter)
        {
            throw std::out_of_range("I coded this part wrong. Please fix it!");
        }
    }

    iterator begin()
    //return iterator to beginning of range. Note that this will decrement the location all the way to the beginning
    {
        for(unsigned int x=0; x<location; ++x) --present_location_iter;
        location=0;
        return present_location_iter;
    }

    iterator end()
    {
        return end_iter;
    }

    int size()
    {
        return num_elements;
    }

    return_type& operator[](unsigned int i)
    //return a referance to an object at location i. Throws an std::out_of_range if i is not in range
    //this function is the primary point of this class
    {
    //NOTE: lower bound validity assured by i being unsigned

        //move to correct location
        if(location < i)
        {
            for(unsigned int x=0; x<(i-location); x++ )
            {
                if(present_location_iter == end_iter)
                {
                    throw std::out_of_range("index is past end of range") ;
                }
                ++present_location_iter;
            }
            location=i;
        }
        else if(location>i)
        {
           for(unsigned int x=0; x<(location-i); x++ )
            {
                --present_location_iter;
            }
            location=i;
        }

        if(present_location_iter == end_iter)
        {
            throw std::out_of_range("index is past end of range");
        }
        return *present_location_iter;
    }

    unsigned int get_location() const
    {
        return location;
    }

    iter_wrap<container>& operator=(const iter_wrap<container>& RHS)
    {
        if (this != &RHS) // self-assignment check expected
        {
            location=RHS.location;
            present_location_iter=RHS.present_location_iter;
            end_iter=RHS.end_iter;
        }
        return *this;
    }

};

////some helper functions for cutting items of end of list
//// works with any container that has a splice

template<typename container>
container pop_end_elements(container& cont, unsigned int num_elems)
{
    auto range_start = cont.end();

    for(unsigned int x=0; x<num_elems; x++) --range_start;

    container ret;
    ret.splice(ret.begin(), cont, range_start, cont.end());
    return ret;
}

}//end csu namespace
#endif


