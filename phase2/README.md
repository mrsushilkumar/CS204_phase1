# CS204 Phase2
## Input file
* In the input.mc file we have to put machine code for the instructions we want to execute.    
* We have tested the machine code for the following programs :- 
    * Finding nth fibonacci number
    * Performing bubble sort on an array
    * Finding sum of the elements of an array 

## Output files
* In the 'output.txt' file we have displayed the values of all the registers and data memory. 
* In the 'stats.txt' file, we have displayed the values for several metrics including the total number of cycles, total instructions executed, CPI (cycles per instruction), number of data transfer instructions (load and store) executed, number of ALU instructions executed, number of control instructions executed, number of stalls, number of data hazards, and number of stalls due to control hazards.

## myRISCVSim.cpp file
In the myRISCVSim.cpp all the functions(fetch(),decode(),execute(),mem(),writeback(),handshake()) performed continuously untill instruction will not end.

## how to compile and run-

1-> save all the file in same folder (input.mc ,output.txt, myRISCVSim.cpp ).    
2-> which fuction you have to perform make .mc file and save it in input file.  
3-> open myRISCVSim.cpp file compile and run it.  
4-> after running of programm the resister value and value at memory address and all the other information will updated in output text file.  
