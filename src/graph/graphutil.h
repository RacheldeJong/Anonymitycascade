// graphutil.h
// 
// Contains utility functions required for computing the d-k-anonymity distribution
// of a graph
// 
// Last edited: 25-5-2021
//

#include "../util.h"

#define HASHKEY 42
#define GIANT_FRAC_THRESHOLD 0.2

// Given a graph sg1 (sgo and sgi for directed), vertex v and distance d, 
// this function generates and returns the d-neighbourhood of v
// 
// @sg: sparse graph in which n-neighborhood should be found (undirected)
// @sgo: graph with outgoing edges only (directed)
// @sgi: graph with ingoing edges only (directed)
// @sg2: graph where d-neighbourhood is stored
// @v: node for which the d-neighborhood should be found
//     NOTE: this value changes based on the label it gets in the subgraph
//           nodes in the resulting graph (sg2) will have new labels
// @d: distance used for neighbourhood (d-neighbourhood)
//
void get_neighborhood(const sparsegraph sg, sparsegraph &sg2, int &v, const int d);
void get_neighborhood_directed(const sparsegraph sgo, const sparsegraph sgi, sparsegraph &sg2, int &v, const int d);

// Given a graph sg, this function finds all nodes that are in the d-neighborhood
// of node v. The number of edges it contains is also stored in variable edges
//
// @sg: sparse graph in which d-neighborhood should be found
// @v: node for which the n-neighborhood should be found
//     NOTE: this value changes based on the label it gets in the subgraph
// @k: k-neighborhood, maximum distance for nodes in subgraph from node vs
// @edges: Number of edges obtained in the subgraph. 
//         NOTE: This value is not used, but only changed during execution so the 
//         number of edges is known afterwards (Useful for allocating space for subgraph)
//
// @Returns: set containing all nodes in d-neighborhood of v
//
std::unordered_set<int> get_neighborhood_nodes(const sparsegraph sg, const int v, const int d, int &edges);

// Get the degree distribution (out-degree distribution for directed graphs)
// and store it in degrees (outdegs)
//
// @sg: sparse graph for which the degree distribution is computed
// 
// @Returns: degrees / outdegs: data structure in which the degree distribution is stored
//
std::map<int, size_t>  get_degree_distribution(const sparsegraph sg);
std::map<int, size_t>  get_degree_distribution_directed(const sparsegraph sg);

std::vector<int> get_giant_component(const sparsegraph sg1);

// Generate a hash representing sparsegrpaph sg
//
// @sg: the sparse grpah to be hashed
//
// @Returns: hash with data type long
//
long get_hash(sparsegraph sg);

// Simple function to print sparse graph. Format:
// from: to, to,
// 0: 1, 2, 
// 1: 2, 3,
// ...
//
// @sg: sparse graph to print
// 
void print_graph(const sparsegraph sg);
