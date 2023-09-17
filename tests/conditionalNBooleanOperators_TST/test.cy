

def newline()
    10.print_char()
    
def space()
    32.print_char()

def printYes()
    121.print_char()
    101.print_char()
    115.print_char()
    
def printNo()
    110.print_char()
    111.print_char()
    
def TrueYes()
    printYes()
    return true
    
def TrueNo()
    printNo()
    return true
    
def FalseYes()
    printYes()
    return false

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
    
    ## test boolean operators
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
    
    ## test short-circuiting
    FalseYes() and TrueNo() ## test and does short circuit
    newline()
    TrueYes() and FalseYes() ## test and does not short circuit
    newline()
    
    TrueYes() or TrueNo()  ## test or does short circuit
    newline()
    FalseYes() or FalseYes()  ## test or does not short circuit
    newline()
    
    

