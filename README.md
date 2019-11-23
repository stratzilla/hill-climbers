# Hill Climbers
A very rudimentary multidimensional local search using hill climbing to find global minima over various functions. The algorithm is generalized to any dimensionality, although the source code has a dimensionality of `2` chosen for an expectation of common use.

# Compilation and Execution
Compile with the below:

`$ g++ hillclimb.cpp -pthread -std=c++0x -o hc`

And execute as below:

`$ ./hc <arg1> <arg2>`

Where `<arg1>` is the function to minimize (`[1..8]`) and `<arg2>` is how many hill climbers (threads) to utilize within the range `[1..8]`.

This program is multithreaded (up to eight threads) to expedite and improve finding the solution. This program also has no termination condition other than `SIGINT`: it will continue trying to find the global minimum until the user chooses to signal interrupt, at which point the best in run fitness and position is outputted to the user.

# Dependencies
- gcc/g++
- C++ (0x or higher, may need `-std=c++0x` flag)
- GNU/Linux

# Functions
The included minimizing functions are:

| `<arg1>` | Name       | Function | Bounds |
| -------- | ---------- | -------- | ------ |
| 1        | Egg Holder | <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20-%28x_%7Bi&plus;1%7D%20&plus;%2047%29%20%5Csin%7B%28%5Csqrt%7B%7Cx_%7Bi&plus;1%7D%20&plus;%20%5Cfrac%7Bx_i%7D%7B2%7D%20&plus;%2047%7C%7D%29%7D%20-%20x_i%20%5Csin%7B%28%5Csqrt%7B%7Cx_i%20-%20%28x_%7Bi&plus;1%7D%20&plus;%2047%29%7D%29%7D" /> | <img src="https://latex.codecogs.com/png.latex?%5B-512%2C%20512%5D" /> |
| 2        | Schwefel   | <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20418.9829d%20-%20%5Csum_%7Bi%3D1%7D%5Ed%20x_i%20%5Csin%7B%28%5Csqrt%7B%7Cx_i%7C%7D%7D%29" /> | <img src="https://latex.codecogs.com/png.latex?%5B-500%2C%20500%5D" /> |
| 3        | Rastrigin  | <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20%5Csum_%7Bi%3D1%7D%5Ed%20%5Bx_i%5E2%20-%2010%20%5Ccos%282%5Cpi%20x_i%29%5D" /> | <img src="https://latex.codecogs.com/png.latex?%5B-5.12%2C%205.12%5D" /> |
| 4        | Griewank   | <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20%5Csum_%7Bi%3D1%7D%5Ed%20%5Cfrac%7Bx_i%5E2%7D%7B4000%7D%20-%20%5Cprod_%7Bi%3D1%7D%5Ed%20%28%5Cfrac%7Bx_i%7D%7B%5Csqrt%7Bi%7D%7D%29%20&plus;%201" /> | <img src="https://latex.codecogs.com/png.latex?%5B-600%2C%20600%5D" /> |
| 5        | Sphere     | <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20%5Csum_%7Bi%3D1%7D%5Ed%20x_i%5E2" /> |  <img src="https://latex.codecogs.com/png.latex?%5B-5.12%2C%205.12%5D" /> |
| 6        | Dixon-Price| <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20%28x_1%20-%201%29%5E2%20&plus;%20%5Csum_%7Bi%3D2%7D%5Ed%20i%282x_i%5E2%20-%20x_%7Bi-1%7D%29%5E2" /> | <img src="https://latex.codecogs.com/png.latex?%5B-10%2C%2010%5D" /> |
| 7        | Sum Squares| <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20%5Csum_%7Bi%3D1%7D%5Ed%20ix_i%5E2" /> | <img src="https://latex.codecogs.com/png.latex?%5B-10%2C%2010%5D" /> |
| 8        | Sum of Different Powers| <img src="https://latex.codecogs.com/png.latex?f%28x%29%20%3D%20%5Csum_%7Bi%3D1%7D%5Ed%20%7Cx_i%7C%5E%7Bi&plus;1%7D" /> | <img src="https://latex.codecogs.com/png.latex?%5B-1%2C%201%5D" /> |

# Hill Climbing
The hill climbing algorithm will take an initial position, evaluate its fitness against one of the included functions, and then generate four possible "moves" away from that position. Repeating the evaluative step, if a fitness is better than previously found, that becomes the new position and additional moves are made from that position. Moves are found using a stochastic summand appended to a position in all dimensions, leading to a random (close) move from the original position.

Some pseudocode:

```
initialize best fitness BF as arbitrarily high
initialize best position BP as empty
while termination condition not met:
  find a random position P
  find fitness F of P
  while P is within bounds:
    find four moves M, where M = P + random summand
    for each M:
      find fitness MF of M
      if MF < F:
        F = MF
        P = M
    if F < BF:
      BF = F
      BP = P
```

Eventually, the best fitness will be found and the position at which the fitness was found is stored. This is the global minimum for the function. Typical pitfalls (pun intended) of hill climbing algorithms is getting trapped in local minima: this issue is still prevalent in this program. It it meant only as an exercise in hill climbing, not an improvement upon a vanilla implementation.

# Some Changes You May Wish To Make
Some aspects of the program are hardcoded. You may wish to change some of these lines for your own experimentation:

| Line | What it Does                        | Default |              
| ---- | ----------------------------------- | ------- |
| 11   | Dimensionality of problem           | `2`     |
| 12   | How many moves to make per position | `4`     |
| 13   | Min/max number of threads           | `1`, `8`|
