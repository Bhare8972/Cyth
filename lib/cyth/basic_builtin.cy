cimport int as cint
from 'cyth_builtin.h' cimport print_int as C_print_int
from 'cyth_builtin.h' cimport print_char as C_print_char
!!!compileNlink cyth_builtin.c

def print_int( cint i )
    C_print_int( i )

def print_char(cint i)
    C_print_char( i )
