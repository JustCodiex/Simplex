# Simplex Method
This is a basic C implementation of the simplex algorithm. This algorithm can solve linear programming problems in standard/cannonical form:
$$
\text{max} \ c^Tx \\
\text{s.t.} \ Ax \leq b
$$
Which can be useful for solving several optimisation problems such as profit maximisation under certain constraints and even finding the optimal diet plan.

## Running the program
The program can be executed by compiling the `simplex.c` file using any C-compiler. The executable can then be executed through a terminal with a list of files to run. For example:
```
simplex "basic.txt" "prob1.txt"
```
The input program must be of the format:
```txt
variables N
{max,min} C_1, C_2, ... C_N
constraints M
A[1,1], A[1, 2], ... A[1, N] {<=,>=,=} B_1
                 ...
A[M,1], A[M, 2], ... A[M, N] {<=,>=,=} B_M 
```
See the [basic.txt](basic.txt) file for a complete example. Running the basic problem results in the output:
```
Solving problem: basic.txt
Initial Dictionary:
                          x1        x2        x3
    Zeta =  0.0000 +  5.0000 +  4.0000 +  3.0000
      w1 =  5.0000 -  2.0000 -  3.0000 -  1.0000
      w2 = 11.0000 -  4.0000 -  1.0000 -  2.0000
      w3 =  8.0000 -  3.0000 -  4.0000 -  2.0000

--- Now Solving ---
x1 entering and w1 leaving:

                          w1        x2        x3
    Zeta = 12.5000 -  2.5000 -  3.5000 +  0.5000
      x1 =  2.5000 -  0.5000 -  1.5000 -  0.5000
      w2 =  1.0000 +  2.0000 +  5.0000 - -0.0000
      w3 =  0.5000 +  1.5000 +  0.5000 -  0.5000 

x3 entering and w3 leaving:

                          w1        x2        w3
    Zeta = 13.0000 -  1.0000 -  3.0000 -  1.0000
      x1 =  2.0000 -  2.0000 -  2.0000 +  1.0000
      w2 =  1.0000 +  2.0000 +  5.0000 -  0.0000
      x3 =  1.0000 +  3.0000 +  1.0000 -  2.0000

--- Simplex Terminating (Success) ---

Maximum Value: 13.000000
Variables: x1 = 2.0000, w2 = 1.0000, x3 = 1.0000
```