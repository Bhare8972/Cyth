#!/usr/bin/env python3
import argparse
from os import mkdir, listdir, path
import subprocess

if __name__ == "__main__":
    
    test_directory = path.dirname(path.abspath(__file__))
    tests = [ f for f in listdir(test_directory) if f.endswith("_TST") ]
    
    parser = argparse.ArgumentParser(description='Run cyth tests.')
    parser.add_argument("--compOut", action='store_true', help="print compiler output")
    parser.add_argument( "--cyth", action="store", required=True, help="location of the cyth compiler" )
    parser.add_argument( "--out", action="store", required=True, help="location where to output tests" )
    parser.add_argument( "--lib", action="store", required=True, help="location of the cyth library to use" )
    parser.add_argument( "--test", action="append", choices=tests, help="specific test to run (can be multiple), is optional" )
    args = parser.parse_args()

    cyth_command = args.cyth
    out_directory = args.out
    cyth_lib = args.lib

    print_compiler_output = args.compOut
    
    to_run = tests
    if args.test is not None:
        to_run = args.test
        
    for running_tst in to_run:
        test_dir = out_directory + "/" + running_tst
        working_test_dir = test_dir + "/tmp"
        
        if not path.exists( test_dir ):
            mkdir(test_dir)
        if not path.exists( working_test_dir ):
            mkdir(working_test_dir)
            
        #command =  cyth_command + " -i " + test_directory+'/'+running_tst+"/test.cy --out_loc " + test_dir + " --inter_loc " + working_test_dir
        command = [ cyth_command, "-l "+cyth_lib, "-i "+test_directory+'/'+running_tst+"/test.cy", "--out_loc "+test_dir,  "--inter_loc "+working_test_dir]
        print( "running test:", running_tst  )
        print("   cyth cmd:", ' '.join(command))
        
        out = None
        try:
            out = subprocess.run( command, capture_output=True, text=True )
        except Exception as e:
            print("subprocess compile error! cmd:", ' '.join(command))
            print( e)
            if out:
                print('out:', out)
            continue
        	
        # print( out.stdout )

        if out.returncode == 0:
            if print_compiler_output:
                print( out.stdout )
                print('/n')
            print("  compiled succsesfully")
        else:
            print("  ERROR compiling test")
            print('cmd:', ' '.join(command) )
            print("stdout:")
            print( out.stdout )
            print("stderr:")
            print( out.stderr )
            print(":\n")
            continue
        
        run_command = test_dir + "/test.cy.out"

        print('running program. cmd:', run_command)
        out = subprocess.run( [run_command], capture_output=True, text=True )

        print('testing result')
        expected_text = open( test_directory+'/'+running_tst+"/OUT" ).read()
        
        if expected_text !=  out.stdout:
            print("ERROR! test failed given output:")
            print()
            print('~'+out.stdout+'~')
            print()



            shortest_L = min(len(expected_text), len(out.stdout) )
            found = False
            for i in range(shortest_L):
                expec_char = expected_text[i]
                got_char = out.stdout[i]
                print(expec_char, end='')
                if expec_char != got_char:
                    print('~'+got_char+'~')
                    found = True
                    break
            if not found:
                print()
                print('End diff e:~'+expected_text[i+1:]+'~g:'+out.stdout[i+1:]+'~')
                print('e len:', len(expected_text[i+1:]), 'g len:', len(out.stdout[i+1:]))

            
        else:
            print("  test succsesful!")
            print()
        
