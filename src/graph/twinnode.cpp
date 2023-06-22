#include "twinnode.h"

int twin_nbs = 5; // Default value
int twinnode_count;

std::vector<int> find_twin_nodes_open(const sparsegraph sg, const std::vector<int> node_set, std::map<int, std::set<int>> &twin_node_map){
   std::map<std::set<int>, int>targets; // Maps set of neighbours to a node
   std::set<int> neighbour_set;
   std::vector<int> node_set_new;
   twinnode_count = 0;

   twin_node_map.clear();

   // For each node, check if it has a twin node / clone
   for(auto i : node_set){
      neighbour_set.clear();

      // Skip node if it has more than max_nbs neighbours
      if(sg.d[i] > twin_nbs){
         node_set_new.push_back(i);
         continue;
      }

      // Get set of neighbours
      for(size_t j = 0; j < sg.d[i]; j++)
         neighbour_set.insert(sg.e[sg.v[i] +j]);

      // Check if its neighbours are in targets list
      auto it = targets.find(neighbour_set);
      
      // If yes, it has a twin node
      if(it != targets.end()){

         (twin_node_map[it->second]).insert(i);

         twinnode_count++;
         continue;
      }
      
      // Otherwise, not: update target list and insert into all set
      else{
         targets[neighbour_set] = i;
         node_set_new.push_back(i);
      }
      neighbour_set.clear();
   }
   return node_set_new;
}

std::vector<int> find_twin_nodes_all(const sparsegraph sg, const std::vector<int> node_set, std::map<int, std::set<int>> &twin_node_map1, std::map<int, std::set<int>> &twin_node_map2){
   std::map<std::set<int>, int>targets1, targets2; // Maps set of neighbours to a node
   std::set<int> neighbour_set, neighbourhood_set;
   std::vector<int> node_set_new;

   twin_node_map1.clear();
   twin_node_map2.clear();

   // For each node, check if it has a twin node / clone
   for(auto i : node_set){
      neighbour_set.clear();
      neighbourhood_set.clear();
      // Skip node if it has more than max_nbs neighbours
      if(sg.d[i] > twin_nbs){
         node_set_new.push_back(i);
         continue;
      }

      neighbourhood_set.insert(i);
      // Get set of neighbours
      for(size_t j = 0; j < sg.d[i]; j++){
         neighbour_set.insert(sg.e[sg.v[i] +j]);
         neighbourhood_set.insert(sg.e[sg.v[i] +j]);
      }

      // Check if its neighbours are in targets list
      auto it = targets1.find(neighbour_set);
      auto it2 = targets2.find(neighbourhood_set);
      
      // If yes, it has a twin nodebtw 
      if(it != targets1.end()){
         (twin_node_map1[it->second]).insert(i);
         twinnode_count++;
      }
      else if(it2 != targets2.end()){
         (twin_node_map2[it2->second]).insert(i);
         twinnode_count++;
      }
      // Otherwise, not: update target list and insert into all set
      else{
         targets1[neighbour_set] = i;
         targets2[neighbourhood_set] = i;
         node_set_new.push_back(i);
      }
   }
   return node_set_new;
}

size_t get_uniquely_identifiable_nodes(std::vector< std::vector<int> > eq, 
const std::map<int, std::set<int>>twin_node_map, const std::map<int, std::set<int>>twin_node_map_c){
   size_t unique_id_count = 0;

   // Iterate over eq classes
   for(auto it : eq){

      //If unique node
      if(it.size() == 1){
         auto node = it.front();
         auto twin_nodes = twin_node_map.find(node);
         auto twin_nodes_c = twin_node_map_c.find(node);
         unique_id_count += 1;
         // Check for twin nodes
         if(twin_nodes != twin_node_map.end()){
            unique_id_count += twin_nodes->second.size();
         }
         if(twin_nodes_c != twin_node_map_c.end()){
            unique_id_count += twin_nodes_c->second.size();
         }
      }
   }
   return unique_id_count;
}

std::vector< std::vector<int> >process_twin_nodes(std::vector< std::vector<int> >eq, const std::map<int, std::set<int>>twin_node_map){
   std::vector< std::vector<int> >eq_new;
   std::vector<int> eq_class;
   std::set<int> nodes;

   // Iterate over classes
   for(auto it : eq){
      // For each class, add twin nodes
      for(auto it2 : it){
         eq_class.push_back(it2);

         // Check if node has a twin
         auto it3 = twin_node_map.find(it2);
         // Copy twins to new equivalence class
         if(it3 != twin_node_map.end()){
            std::copy(it3->second.begin(), it3->second.end(), std::back_inserter(eq_class));
         }
      }
      eq_new.push_back(eq_class);
      eq_class.clear();
   }
   return eq_new;
}

// Remove twin nodes from equivalence classes to determine anonymity if twins are
// counted as one candidate
std::vector< std::vector<int> >remove_twins(std::vector< std::vector<int> >eq, const std::vector<int> node_to_twin){
   std::vector< std::vector<int> >eq_new;
   std::vector<int> eq_class;
   std::set<int> nodes;

   // Iterate over classes
   for(auto it : eq){
      // For each class, add twin nodes
      for(auto it2 : it){
         // Iterate over nodes
         if(node_to_twin[it2] != it2)
            eq_class.push_back(it2);
      }
      eq_new.push_back(eq_class);
      eq_class.clear();
   }
   return eq_new;
}

void print_twinnodes(std::map<int, std::set<int>> twin_node_map, int mode){
   printf("Start twin nodes");
   if(mode==0) printf("\n");
   else if(mode==1) printf(" open\n");
   else if(mode==2) printf(" closed\n");

   for(auto it : twin_node_map){
      printf("%d, ", it.first);
      for(auto it2 : it.second){
         printf("%d, ", it2);
      }
      printf("\n");
   }
   printf("End twin nodes.\n");
}

void print_twinnode_stats(std::map<int, std::set<int>> twin_node_map, const int mode){
   size_t twinnodes_skipped = 0;
   size_t nodes_with_twin = 0;
   std::string twin_type;

   if(mode==0) printf("Twin nodes\n");
   else if(mode==1) printf("Open twin nodes\n");
   else if(mode==2) printf("Closed twin nodes\n");

   for(auto it : twin_node_map){
      nodes_with_twin++;
      for(auto it2 : it.second){
         nodes_with_twin++;
         twinnodes_skipped++;
      }
   }
   printf("Skipped twin nodes: %ld\n", twinnodes_skipped);
   printf("Nodes with a twin: %ld\n", nodes_with_twin);
}
