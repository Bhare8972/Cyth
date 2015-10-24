
#ifndef general_exception
#define general_exception

#include <exception>

class gen_exception : public std::exception
//a generalized exception class. Does not take charge of deleteing char*
{
public:
    const char* msg;
    
    gen_exception(const char* _msg)
    {
        msg=_msg;
    }
    
    virtual const char* what() const throw()
    {
        return msg;
    }
};


#endif
