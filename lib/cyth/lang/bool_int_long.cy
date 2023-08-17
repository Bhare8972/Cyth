cimport int as cint
cimport long as clong
from 'cyth_builtin.h' cimport BYTE as C_BYTE

from 'cyth_builtin.h' cimport print_int as C_print_int
from 'cyth_builtin.h' cimport print_char as C_print_char
!!!compileNlink cyth_builtin.c

class bool 
    C_BYTE __val__

    def __init__()
        self.__val__ = 0

    def __init__(cint v) ## this is vary wierd, but useful
        self.__val__ = v

    def __init__(bool v) ## this makes a default __assign__
        self.__val__ = v.__val__
        
    def __toC__()
        return self.__val__
        
  ## some temp functions
    def print()
        if self.__val__
            cint t = 116
            C_print_char( t )
            t = 114
            C_print_char( t )
            t = 117
            C_print_char( t )
            t = 101
            C_print_char( t )
        else
            cint t = 102
            C_print_char( t )
            t = 97
            C_print_char( t )
            t = 108
            C_print_char( t )
            t = 115
            C_print_char( t )
            t = 101
            C_print_char( t )


bool true
true.__val__ = 1
bool false


class int
    cint __val__

    def __init__()
        self.__val__ = 0

  ### copy constructors. Will make default __assign__
    def __init__( cint v ) 
        self.__val__ = v

    def __init__( int v ) 
        self.__val__ = v.__val__

    def __init__( bool v ) 
        self.__val__ = v.__val__
        
  ## assignment
        
    def __assignTo__( cint v)
        v = self.__val__
    
  ## conversion
    def __convert__( cint v)
        v = self.__val__
        
    def __toC__()
        return self.__val__


  ## binary operators
    #def __lPow__(int RHS)
    #    cint tmp = self.__val__ ** RHS.__val__
    #    return int(tmp)
        
    def __lMul__(int RHS)
        cint tmp = self.__val__ * RHS.__val__
        return int(tmp)
        
    def __lDiv__(int RHS)
        cint tmp = self.__val__ / RHS.__val__
        return int(tmp)
        
    def __lMod__(int RHS)
        cint tmp = self.__val__ % RHS.__val__
        return int(tmp)
        
    def __lAdd__(int RHS)
        cint tmp = self.__val__ + RHS.__val__
        return int(tmp)
        
    def __lSub__(int RHS)
        cint tmp = self.__val__ - RHS.__val__
        return int(tmp)
        
    def __cmp__(int RHS)
        cint R
        if self.__val__ < RHS.__val__
            R = 0-1
        elif self.__val__ == RHS.__val__
            R = 0
        else
            R = 1
        return int(R)

  ## some temp functions
    def print()
        C_print_int( self.__val__ )
       
    def print_char()
        C_print_char( self.__val__ )
        



class long
    clong __val__

    def __init__()
        self.__val__ = 0

  ### copy constructors. Will make default __assign__
    def __init__( clong v ) 
        self.__val__ = v
        
    def __init__( long v ) 
        self.__val__ = v.__val__

    def __init__( int v ) 
        self.__val__ = v.__val__

    def __init__( bool v ) 
        self.__val__ = v.__val__
        
  ## assignment
        
    def __assignTo__( clong v)
        v = self.__val__
    
  ## conversion
    def __convert__( clong v)
        v = self.__val__
        
    def __exConvert__( cint v)
        v = self.__val__
        
    def __exConvert__( int v)
        v.__val__ = self.__val__
        
    def __toC__()
        return self.__val__
        
  ## binary operators
    ## with long
    #def __lPow__(long RHS)
     #   clong tmp = self.__val__ ** RHS.__val__
        #return long(tmp)
        
    def __lMul__(long RHS)
        clong tmp = self.__val__ * RHS.__val__
        return long(tmp)
        
    def __lDiv__(long RHS)
        clong tmp = self.__val__ / RHS.__val__
        return long(tmp)
        
    def __lMod__(long RHS)
        clong tmp = self.__val__ % RHS.__val__
        return long(tmp)
        
    def __lAdd__(long RHS)
        clong tmp = self.__val__ + RHS.__val__
        return long(tmp)
        
    def __lSub__(long RHS)
        clong tmp = self.__val__ - RHS.__val__
        return long(tmp)
        
    # LHS with int
    #def __lPow__(int RHS)
     #   cint tmp = self.__val__ ** RHS.__val__
        #return int(tmp)
        
    def __lMul__(int RHS)
        cint tmp = self.__val__ * RHS.__val__
        return int(tmp)
        
    def __lDiv__(int RHS)
        cint tmp = self.__val__ / RHS.__val__
        return int(tmp)
        
    def __lMod__(int RHS)
        cint tmp = self.__val__ % RHS.__val__
        return int(tmp)
        
    def __lAdd__(int RHS)
        cint tmp = self.__val__ + RHS.__val__
        return int(tmp)
        
    def __lSub__(int RHS)
        cint tmp = self.__val__ - RHS.__val__
        return int(tmp)
        
    # RHS with int
   # def __rPow__(int LHS)
     #   cint tmp = LHS.__val__ ** self.__val__
        #return int(tmp)
        
    def __rMul__(int LHS)
        cint tmp = LHS.__val__ * self.__val__
        return int(tmp)
        
    def __rDiv__(int LHS)
        cint tmp = LHS.__val__ / self.__val__
        return int(tmp)
        
    def __rMod__(int LHS)
        cint tmp = LHS.__val__ % self.__val__
        return int(tmp)
        
    def __rAdd__(int LHS)
        cint tmp = LHS.__val__ + self.__val__
        return int(tmp)
        
    def __rSub__(int LHS)
        cint tmp = LHS.__val__ - self.__val__
        return int(tmp)

  ## some temp functions
    def print()
        C_print_int( self.__val__ )
        


 
 
        
