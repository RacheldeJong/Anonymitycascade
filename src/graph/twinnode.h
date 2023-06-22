// twinnode.h: 
//
// Contains code to find twin nodes and update equivalence classes
//
// Last edited: 9-8-2021
//

#ifndef TWINNODE
#define TWINNODE

#include "../util.h"
#include "../graph/graphutil.h"

extern int twin_nbs;
extern int twinnode_count;

// Find all twin nodes in sg and store them in twin_node_map
// e.g. if 1, 2, 3 are twins: (1, [2, 3]) is stored in twin_node_map
//  1 is the node that represents the twin nodes
//
// @sg1: Sparsegraph
// @twin_node_map: after execution, will contain the twin nodes of sg
//
std::vector<int> find_twin_nodes_open(const sparsegraph sg, const std::vector<int> node_set, std::map<int, std::set<int>> &twin_node_map);
std::vector<int> find_twin_nodes_all(const sparsegraph sg, const std::vector<int> node_set, std::map<int, std::set<int>> &twin_node_map, std::map<int, std::set<int>> &twin_node_map2);

// Updates eq by adding the twin nodes: the node representing a set of twins should be in one class of eq
// other twins can be added to this equivalence class
//
// @eq: the equivalence class without twin nodes
// @twin_node_map: the map created by twin_node_check that contains twin nodes
//
// @Returns: eq, where twin nodes are added in the correct class
//
std::vector< std::vector<int> >process_twin_nodes(std::vector< std::vector<int> >eq, const std::map<int, std::set<int>>twin_node_map);

// Get the number of uniquely identifiable nodes. This counts all
// nodes in an equivalence class of size 1 + all its open or closed twin nodes
size_t get_uniquely_identifiable_nodes(std::vector< std::vector<int> > eq, 
   const std::map<int, std::set<int>>twin_node_map, const std::map<int, std::set<int>>twin_node_map_c);

// Return an equivalence class such that for each set of twins, all but one are removed
std::vector< std::vector<int> >remove_twins(std::vector< std::vector<int> >eq, const std::vector<int> node_to_twin);

// Given a set of nodes, return true if they are all twins of each other.
// i.e., they appear in the same set of twin nodes and therefore have the same key
//
// @node_to_twin: maps each node to its twin class. Generated by function find_twin_nodes
// @nodes: contains the set of nodes
//
// @Returns: true if all nodes in the set are twins, otherwise false
//
template <typename T> 
bool are_twins_set(std::vector<int> node_to_twin, T nodes){
   int twin = node_to_twin[*nodes.begin()];
   
   // No twin class for first node
   if(twin == -1){
      return false;
   }

   // Check if all twins in same class
   for(auto it : nodes){
      if(node_to_twin[it] != twin){
         return false;
      }
   }
   return true;
};

// Print twinnodes found. Each set of twinnodes is separated by "\n"
// 
// @twin_node_map: maps node used to its twins: -> [n]. Generated by find_twin_nodes
// @mode: mode of twin nodes. 0 for nothing, 1 open only, 2 open and closed
void print_twinnodes(std::map<int, std::set<int>> twin_node_map, const int mode);

// Print statistics for tiwn nodes:
// - Number of skipped nodes
// - Number of nodes with a twin
//
// @twin_node_map: maps node used to its twins: -> [n]. Generated by find_twin_nodes
// @mode: mode of twin nodes. 0 for nothing, 1 open only, 2 open and closed
void print_twinnode_stats(std::map<int, std::set<int>> twin_node_map, int mode);

#endif