

def newline()
    10.print_char()
    
def space()
    32.print_char()

def __main__()

     ## todo: should probably test RHS comparisons... but theyr probably fine!

    int A = 1
    int B = 2
    
    bool C = A < B
    C.print()
    newline()
    
    C = A==B
    C.print()
    newline()
    
    ( A<B ).print()
    newline()
    
    
    ( A>B ).print()
    newline()
    
    ( A>=B ).print()
    newline()
    
    ( A<=B ).print()
    newline()
    
    C = true
    bool D = false
    
    (C and C).print()
    space()
    (C and D).print()
    space()
    (D and C).print()
    space()
    (D and D).print()
    newline()
    
    (C or C).print()
    space()
    (C or D).print()
    space()
    (D or C).print()
    space()
    (D or D).print()
    newline()
    
    
    
    
    
    
    
    
