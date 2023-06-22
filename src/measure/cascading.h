
#ifndef CASCADING
#define CASCADING

#include "../util.h"
#include "../equivalence/equivalence.h"
#include "../measure/dk-anonymity.h"

extern bool cascading_distribution;
extern int do_twin_node_check;

// Perform anonymity-cascade. Unique nodes are derived from eq.
// 
// @sg: Sparsegraph
// @eq: Contains equivalence classes determined by d-k-anonymity
// @node_to_eq: Maps each node to its equivalence class
//
// @Returns: set of nodes that can be uniquely identified using anonymity-cascade
std::vector<int> cascading(sparsegraph sg, std::vector<std::vector<int>> eq, std::vector<int> node_to_eq);

#endif