# Anonymity-cascade
This repository contains code used to measure uniqueness in complex networks with `anonymity-cascade` [1].
The measure, uses and builds upon `d-k-anonymity` [2, 3] and uses uses Nauty [4].

# The framework
* `src`: Directory containing source code
  * `anonymity_cascade.cpp`: Contains the main function and code
  * `util.h`: Includes the required libraries
  * `graph`: Contains files related to graphs
      * `graphgen.cpp`: Code to generate graphs and read graphs from input
      * `graphutil.cpp`: Functions useful for working with graphs such as get_neighbourhood or get_degree_distribution
      * `twinnode.cpp`: Functions to find twin nodes and add them correctly to equivalence classes
  * `measure`: Contains files related to the anonymity measures
      * `dk-anonymity.cpp`: Code to partition vertices of a (possibly directed) graph based on `d-k-anonymity` [1]
      * `cascading.cpp` : Code to perform `anonymity-cascade` given an initial equivalence partition of nodes
  * `equivalence`: Contains helping code for equivalence classes
      * `equivalence.cpp`: Code to compute a map that maps each node to the corresponding equivalence class
* `examples`: Directory containing example graphs

# Installation
Before using this code, the Nauty framework should be downloaded from:  https://users.cecs.anu.edu.au/~bdm/nauty/
* Move this git-directory (Anonymity-cascade) in the downloaded nauty directory (nauty27r3)
* Run the following commands:
```
./configure
make
```
Last tested with nauty version `nauty27r3`

# Compilation
```
make 
```

# How to run
To run the code, at least one input graph should be given. Command line arguments are optional.
Use the following command to run the code:
```
./bin/anonymitycascade ./examples/example2 [-arguments]
```
For example:
```
./bin/anonymitycascade ./examples/example2 -s 2
```

## Command line arguments
Various command line arguments can be used to adjust the settings of the algorithm.

* `-d 5` : Set the distance for `d-k-anonymity` (default 1)
* `-dk`  : Only perform `d-k-anonymity` and no `anonymity-cascade`
* `-g`   : Compute anonymity for giant component only 
* `-c` : Turn off twin node preprocessing (default on)
* `-cs 5` : Select maximum number of neighbour to check for twin nodes (default all)
* `-csa`  : Check and use all twin nodes (default)
* `-eq`: Choose to print equivalence classes per iteration / level of cascading (default: off)
* `-s 1`: Choose which statistics to print
    * `0` - Print no statistics
    * `1` - Print final statistics only (defauls)
    * `2` - Print statistics per iteration
    * `3` - Print per class split (Preceded by `/`)
    * `4` - Print all statistics, including progress updates (Preceded by `//`)
* `-cd`: Compute cascading distribution
* `-t`: Print runtime for each canonical labeling computation (`d-k-anonymity only`, default: off)
* `-db 0` : Debug mode for `d-k-anonymity` (default off)
* `-h 3` : Set the heuristic / algorithm choice `d-k-anonymity`
    * `0` - Naive (slowest)
    * `1` - Iterative
    * `2` - Iterative + number of vertices and edges as heuristic (fastest, default)
    * `3` - Iterative + degree distribution heuristic (fastest)
    * `4` - Iterative + equivalence class distribution (not implemented for directed)

# Input graphs

Input graph files should be using the .dre format. Some examples are included in directory `examples`.
Start with !n=number_vertices, then for each vertex 0 to n-1 vertex: edge1, target2, ...;. To indicate the end of the file, end for the last node with `.` instead of `;`.

Example ` example2`:

```
!n=6
0: 3 4 5;
1: 3 4 5;
2: 3 4 5;
3: 0 1 2;
4: 0 1 2;
5: 0 1 2.
```
# Examples
Folder `examples` contains some small toy examples to test the d-k-anonymity measure. For each test case "x" there are 3 files:
* testx: The edgelist that can be read by Nauty
* testx.group: The grouping that d-k-anonymity will make if d = diameter(G). Vertices on the same line are in the same group
* testx.png: A visualization of the graph with each node coloured according to the group it belongs in. Generated with Networkx in `visualize.py`

Note that test1 is an empty graph: networkx does not show these nodes. Therefore `test1` is empty
Examples can be generated with `visualize.py`, code to generate the examples are given in `examples.sh`

# References
[1] de Jong, Rachel G., Mark P. J. van der Loo, and Frank W. Takes. "Algorithms for Efficiently Computing Structural Anonymity in Complex Networks." ACM Journal of Experimental Algorithmics (2023). \
[2] de Jong, Rachel G., Mark P. J. van der Loo, and Frank W. Takes. "The effect of distant connections on node anonymity in complex networks." Scientific Reports 14.1 (2024): 1156.
 \
[3] https://github.com/RacheldeJong/dkAnonymity \
[4] B. D. McKay and A. Piperno, “Practical graph isomorphism”, Journal of Symbolic Computa-tion, vol. 60, no. 0, pp. 94–112, 2014.

