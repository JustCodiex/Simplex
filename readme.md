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
See the [basic.txt](basic.txt) file for a complete example.
