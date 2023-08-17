cimport int as cint
cimport float as cfloat

from 'basic_builtin' import print_int
from 'basic_builtin' import print_char

def newline()
    cint tst = 10
    print_char( tst )

class number

    def __init__()
        cint tst = 110
        print_char( tst ) ## n
        tst = 117
        print_char( tst ) ## u
        tst = 109
        print_char( tst ) ## m
        newline()

    def do_blarg()
        cint tst = 98
        print_char( tst ) ## b
        tst = 108
        print_char( tst ) ## l
        tst = 97
        print_char( tst ) ## a
        tst = 114
        print_char( tst ) ## r
        tst = 103
        print_char( tst ) ## g
        newline()

    def do_blarg(cint A)
        cint tst = 98
        print_char( tst ) ## b
        tst = 108
        print_char( tst ) ## l
        tst = 97
        print_char( tst ) ## a
        tst = 114
        print_char( tst ) ## r
        tst = 103
        print_char( tst ) ## g
        print_int( A ) ## number
        newline()


    def __del__()
        cint tst = 110
        print_char( tst ) ## n
        tst = 117
        print_char( tst ) ## u
        tst = 109
        print_char( tst ) ## m
        tst = 100
        print_char( tst ) ## d
        tst = 101
        print_char( tst ) ## e
        tst = 108
        print_char( tst ) ## l
        newline()

class TST_int( number )
    cint __A__

    def __init__()
        cint tst = 105
        print_char( tst ) ## i
        tst = 110
        print_char( tst ) ## n
        tst = 116
        print_char( tst ) ## t
        newline()


    def do_blarg()
        cint tst = 68
        print_char( tst ) ## D
        tst = 79
        print_char( tst ) ## O
        self.number.do_blarg() 

    def __del__()
        cint tst = 105
        print_char( tst ) ## i
        tst = 110
        print_char( tst ) ## n
        tst = 116
        print_char( tst ) ## t
        tst = 100
        print_char( tst ) ## d
        tst = 101
        print_char( tst ) ## e
        tst = 108
        print_char( tst ) ## l
        newline()

def __main__()

    TST_int A
    A.do_blarg()
    cint B = 5
    A.do_blarg(B)


