
#ifndef general_exception
#define general_exception

#include <exception>
#include <sstream>
#include <string>

class gen_exception : public std::exception
//a generalized exception class. Does not take charge of deleteing char*
{
private:

    template< typename T, typename...Ts>
    void add_msg(std::stringstream& stream, T msg_head, Ts... msg)
    {
        stream<<msg_head;
        add_msg(stream, msg...);
    }

    void add_msg(std::stringstream& stream)
    {
        msg=stream.str();
    }

public:
    std::string msg;

    template< typename... msgT>
    gen_exception(msgT... msg)
    {
        std::stringstream stream;
        add_msg(stream, msg...);
    }

    virtual const char* what() const throw()
    {
        return msg.c_str();
    }
};


#endif
