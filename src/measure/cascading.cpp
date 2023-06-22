#include "cascading.h"

std::vector<int> cascading(sparsegraph sg, std::vector<std::vector<int>> eq, std::vector<int> node_to_eq){
   std::vector<int> unique_casc, unique_casc_new, node_set;
   std::queue<int> q;
   std::vector<int> unique_check(sg.nv, 0);
   std::map<int, std::set<int>> twin_node_map, twin_node_map_c;
   std::vector<int> node_to_twin_o(sg.nv, -1), node_to_twin_c(sg.nv, -1);
   int unique_before = 0;
   int unique_count = 0;
   int unique_count_prev = 0;
   int unique_casc_count = 0;
   int unique_casc_count_prev = 0;
   int unique_casc_twins_count = 0;
   int unique_casc_twins_count_prev = 0;
   size_t casc_twin_v, casc_v, casc_twin_vu, casc_vu;
   clock_t t1, t2;
   bool reuse_knowledge=false;

   // Find twin nodes
   t1 = clock();
   t2 = clock();
   int i = 0;
   
   // Filter giant component
   if(filter_giant){
      node_set = get_giant_component(sg);
      if(print_statistics > 0)
         printf("Giant component(s) nodes: %ld / %d\n", node_set.size(), sg.nv);
   }
   else{
      for(i = 0; i < sg.nv; i++){
         node_set.push_back(i);
      }
   }
   if(do_twin_node_check == 1){
      find_twin_nodes_open(sg, node_set, twin_node_map);
   }
   else if(do_twin_node_check == 2){
      find_twin_nodes_all(sg, node_set, twin_node_map, twin_node_map_c);
   }
   else if(print_statistics > 0) printf("Skip twin node step\n");

   // Create mapping node -> twin node class
   for(auto it : twin_node_map){
      node_to_twin_o[it.first] = it.first;
      for(auto it2 : it.second){
         node_to_twin_o[it2] = it.first;
      }
   }
   for(auto it : twin_node_map_c){
      node_to_twin_c[it.first] = it.first;
      for(auto it2 : it.second){
         node_to_twin_c[it2] = it.first;
      }
   }

   if(print_statistics > 0) printf("Time twin nodes: %f\n", ((double)(clock() - (double)t2))/CLOCKS_PER_SEC);
   
   // Find unique nodes
   for(auto it : eq){
      // Unique
      if(it.size() == 1){
         q.push(it.front());
         unique_casc.push_back(it.front());
         unique_check[it.front()] = 1;
         unique_count++;
      }
      // Twin-unique
      else if((do_twin_node_check >= 1 && are_twins_set(node_to_twin_o, it))
      || (do_twin_node_check >= 2 && are_twins_set(node_to_twin_c, it)) ){
         // For each node, add if not yet marked as unique
         for(auto it2 : it){
            q.push(it2);
            unique_casc.push_back(it2);
            unique_check[it2] = 1;
            unique_count++;
            unique_casc_twins_count++;
         }
      }
   }

   unique_before = q.size();
   int q_size = q.size();
   int layer = 0;
   unique_count_prev = unique_before;

   if(print_statistics > 0){
      printf("Nodes %d, edges %ld\n", sg.nv, sg.nde);
      printf("Layer: nr, unique total, unique cascading, unique cascading + twins\n");
      printf("Layer: %d, %f, %d, %d, %d\n", layer, ((double)(clock() - (double)t2))/CLOCKS_PER_SEC, 
                                                    unique_count,
                                                    unique_casc_count - unique_casc_count_prev,
                                                    unique_casc_twins_count - unique_casc_twins_count_prev);
   }
   if(print_eq_class){
      printf("Cascading %d: ", 0);
      for(auto it : unique_casc){
         printf("%d, ", it);
      }
      printf("\n");
   }
   if(cascading_distribution)
      printf("node, total, totalu, casc, cascu, casct, casctu\n");
   
   // Iterate over empty nodes
   while(!q.empty()){
      int v = q.front();
      q.pop();
      
      // Create equivalence distribution over all neighbors of v
      int start = sg.v[v];
      std::map<int, std::set<int> > eq_distr;
      for(int i=0; i < sg.d[v]; i++){
         // Get eq distribution
         int nbr = sg.e[start + i];
         int eq_nbr = node_to_eq[nbr];

         // Don't add nodes that are already identified as unique
         if(reuse_knowledge && unique_check[nbr])
            continue;
         if(eq_distr.find(eq_nbr) != eq_distr.end())
            eq_distr[eq_nbr].insert(nbr);
         else
            eq_distr[eq_nbr] = {nbr};
      }
      if(cascading_distribution){
         casc_v = 0;
         casc_vu = 0;
         casc_twin_v = 0;
         casc_twin_vu = 0;
      }
      // Process created eq distribution
      for(auto it : eq_distr){
         // Found node that is unique in neighborhood of (uniquely identifiable node) v
         if(it.second.size() == 1){
            casc_v++;
            // If not already set as uniquely identifiable
            if(unique_check[*it.second.begin()] != 0){
               casc_vu++;
               continue;
            }
            // Add to queue
            q.push(*it.second.begin());
            unique_casc.push_back(*it.second.begin());
            unique_casc_new.push_back(*it.second.begin());
            unique_check[*it.second.begin()] = 1;
            unique_count++;
            unique_casc_count++;
         }
         // No unique equivalence class, but all nodes in class are twins
         else if(do_twin_node_check >= 1 && are_twins_set(node_to_twin_o, it.second) || 
         (do_twin_node_check >=2 && are_twins_set(node_to_twin_c, it.second))){
            casc_twin_v++;
            // For each node, add if not yet marked as unique
            for(auto it2 : it.second){
               if(unique_check[it2] != 0){
                  casc_twin_vu++; 
                  continue;
               }
               q.push(it2);
               unique_casc.push_back(it2);
               unique_casc_new.push_back(it2);
               unique_check[it2] = 1;
               unique_count++;
               unique_casc_twins_count++;
            }
         }
         if(cascading_distribution && !q.empty()){
            printf("%d, %ld, %ld, ", v, casc_v + casc_twin_v, casc_v + casc_twin_v - (casc_vu + casc_twin_vu));
            printf("%ld, %ld, %ld, %ld\n", casc_v, casc_v - casc_vu, casc_twin_v, casc_twin_v - casc_twin_vu);
         }
      }
      q_size -= 1;
      if(q_size == 0){
         layer++;
         printf("Layer: %d, %f, %d, %d, %d\n", layer, ((double)(clock() - (double)t2))/CLOCKS_PER_SEC,
                                           unique_count - unique_count_prev,
                                           unique_casc_count - unique_casc_count_prev,
                                           unique_casc_twins_count - unique_casc_twins_count_prev);
         t2 = clock();
         unique_count_prev = unique_count;
         unique_casc_count_prev = unique_casc_count;
         unique_casc_twins_count_prev = unique_casc_twins_count;
         q_size = q.size();
         if(print_eq_class){
            printf("Cascading %d: ", layer);
            for(auto it : unique_casc_new){
               printf("%d, ", it);
            }
            printf("\n");
         }
         unique_casc_new = {};
         if(cascading_distribution)
            printf("node, total, totalu, casc, cascu, casct, casctu\n");
      }
   }
   printf("Final: %d, %f, %d, %d, %d\n", layer, ((double)(clock() - (double)t1))/CLOCKS_PER_SEC, unique_count, unique_casc_count, unique_casc_twins_count);
   
   if(print_eq_class){
      printf("Cascading final: ");
      for(auto it : unique_casc){
         printf("%d, ", it);
      }
      printf("\n");
   }
   return unique_casc;
}