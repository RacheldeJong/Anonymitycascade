#include "equivalence.h"

std::vector<int> get_node_to_eq(const std::vector< std::vector<int>> eqs, const int n){ 
    
   std::vector<int> node_to_eq(n, -1);
   int eq_id = 0;
   for(auto eq : eqs){
      for(auto node : eq){
         node_to_eq[node] = eq_id;
      }
      eq_id++;
   }
   int tel = 0;
   for(auto it : node_to_eq){
      tel += 1;
   }
   return node_to_eq;
}