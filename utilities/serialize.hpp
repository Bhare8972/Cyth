
#ifndef SERIALIZE_151222235315
#define SERIALIZE_151222235315

#include <fstream>
#include <string>


template<typename T>
void binary_write(std::ostream& output, T to_write)
//write a bit of numerical data to a file.
{
    output.write((char*)&to_write,sizeof(to_write));
}

template<typename T>
void binary_read(std::istream& input, T& to_read)
//read a bit of numerical data from a file.
//type must match when it was written, or will be inconsisitant
{
    input.read((char*)&to_read,sizeof(to_read));
}


// for CPP string
void binary_write(std::ostream& output, std::string& input)
{
    uint L = input.size();
    binary_write(output, L );
    for( char I : input)
    {
        binary_write(output, I);
    }
}

std::string binary_read(std::istream& input, std::string& out)
{
    uint L;
    binary_read(input, L);
    out.resize( L );

    for(int i=0; i<L; i++)
    {
        char I;
        binary_read(input, I);
        out[i] = I;
    }
    return out;
}

#endif
