
def newline()
    10.print_char()

## NOTE! bool_int_long init is not being called

def __main__()

    int A_int( 5 )
    int B_int( 10 )
    A_int.print()
    newline()
    
    long A_long( 1 )
    A_int = int(A_long)
    A_int.print()
    newline()
    
    
    A_int = true
    A_int.print()
    A_int = false
    A_int.print()
    newline()
    
    A_int = 6
    A_int.print()
    newline()
    
    int C_int = B_int - A_int
    C_int.print()
    newline()
    
    C_int = A_int + B_int
    C_int.print()
    newline()
    
    long B_long = A_long % A_int
    B_long.print()
    newline()
    
    B_long = A_int / A_long
    B_long.print()
    newline()
    
    long C_long = A_long*B_long
    C_long.print()
    newline()
    
    C_long = (A_long + A_int)*B_int - B_long
    C_long.print()
    newline()
    
    
    
    
