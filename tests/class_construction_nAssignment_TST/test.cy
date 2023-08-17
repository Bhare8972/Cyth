cimport int as cint

from 'basic_builtin' import print_int
from 'basic_builtin' import print_char

def newline()
    cint tst = 10
    print_char( tst )
    
def print_semicolon()
    cint tst = 59
    print_char( tst )

cint intID = 0
cint one = 1

def incr_intID()
    intID = intID + one
    
def print_intID(cint ID)
    cint tst = 32
    print_char( tst ) # space
    tst = 73
    print_char( tst ) # I
    #tst = 68
    #print_char( tst ) # D

    print_int( ID )

class TSTint
    cint __val__
    cint been_destructified
    cint ID

    def __init__()
        self.__val__ = 0
        self.been_destructified = 0
        
        
        cint tst = 63 
        print_char( tst ) ## ?
        tst = 65
        print_char( tst ) ## A
        
        self.ID = intID
        incr_intID()
        print_intID(self.ID)
        newline()

    def __init__( cint v )
        self.been_destructified = 0
        self.__val__ = v
        
        cint tst = 63 
        print_char( tst ) ## ?
        tst = 66
        print_char( tst ) ## B
        
        self.ID = intID
        incr_intID()
        print_intID(self.ID)
        newline()

    def __init__( TSTint v )
        self.been_destructified = 0
        self.__val__ = v.__val__
        
        cint tst = 63 
        print_char( tst ) ## ?
        tst = 67
        print_char( tst ) ## C
        
        self.ID = intID
        incr_intID()
        print_intID(self.ID)
        newline()

    def print()
        
        cint tst = 80 
        print_char( tst ) # P
        print_int( self.__val__ )
        print_int( self.been_destructified )
        print_intID(self.ID)
        newline()
        

    def __del__()
        cint tst = 68 
        print_char( tst ) # D
        print_int( self.__val__ )
        print_int( self.been_destructified )
        self.been_destructified = 1
        print_intID(self.ID)
        newline()

def make_int()
    cint tst = 64
    print_char( tst ) # @
    TSTint out
    return out

class two_ints
    TSTint A
    TSTint B

    #def __init__(cint _A)
    #    
    #    construct
    #        self.A( _A )

    def __init__(cint _A, cint _B)
        
        construct
            self.A( _A )
            self.B( _B )

    def __init__(TSTint _A)
        construct
            self.A( _A)

    def print()
        self.A.print()
        cint tst = 32
        print_char( tst ) ## space
        self.B.print()

class big_int
    TSTint A
    def __exInit__(TSTint _A )
        construct
            self.A( _A)

    def __convert__(TSTint _A )
        cint tst = 0
        construct
            _A.__val__( self.A.__val__ )
            _A.been_destructified( tst )
            _A.ID( intID )

        tst = 63 
        print_char( tst ) ## ?
        tst = 69
        print_char( tst ) ## E
        
        incr_intID()
        print_intID( _A.ID )
        newline()


def __main__()

    TSTint blarg
    blarg = make_int()
    blarg.print()
    
    newline()    

    cint tst = 6
    TSTint glarg( tst )
    glarg.print()
    newline()

    two_ints YO
    YO.print()
    newline() 

    tst = 8
    two_ints YO1( tst )
    YO1.print()
    newline() 

    tst = 3
    cint tst2 = 12 
    two_ints YO2(tst, tst2)
    YO2.print()
    newline()

    two_ints YO3( glarg )
    YO3.print()
    newline() 

    two_ints YO4( YO2 )
    YO4.print()
    newline()

    YO = YO4
    YO4.print()
    YO.print()
    newline()

    glarg = blarg
    glarg.print()
    newline()


    tst = 26 
    auto BIG_glarg = big_int( tst )
    glarg = BIG_glarg
    newline()
    
    print_semicolon()
    newline()
### it's all bit a silly isn't it?


