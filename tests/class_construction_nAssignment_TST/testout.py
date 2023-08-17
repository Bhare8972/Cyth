#!/usr/bin/env python3


from os import path

if __name__ == "__main__":
    test_directory = path.dirname(path.abspath(__file__))
    data = open(test_directory+"/OUT",'r').read()
    
    n_ints = 0
    for char in data:
        if char == "?":
            n_ints += 1
        elif char == "D":
            n_ints -= 1
    print("num ints (should be 0):", n_ints)