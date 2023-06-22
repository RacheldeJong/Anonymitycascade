#include "dk-anonymity.h"

// Default settings
int use_cache = 1;
int print_eq_class = 0;
int print_statistics = 1;
int conf_choice = 2;
int print_time_can_labelling = 0;
int debug_mode = 0;
int filter_giant = 0;

size_t iso_checks, nr_can;

std::vector<int> eq_map; // Map node_id -> equivalence_class_id

bool are_same_sg_can(sparsegraph *cg1, sparsegraph *cg2, const int v1, const int v2, const int v1_pos, int *lab, int *orbits){
   int i, n, m, target;
   n = cg1->nv;
   m = SETWORDSNEEDED(n);

   iso_checks++;

   // Compare canonically labelled graphs
   // If graphs are isomorph, then after Traces function sg1 == sg2 holds
   if (aresame_sg(cg1,cg2)){
      
      // Find node in sg2 isomorph with v1:
      // Each node i in the graph is mapped to a new label: lab1[i](G1) -> i (G'1)
      // lab1[i] =iso lab2[i] -> if v1 = lab1[i] then lab2[i] is a node in G2 isomorphic to v1 in G1
      target = lab[v1_pos];

      // Check if nodes are in the same orbit 
      // v1 =iso lab1[i] =iso lab2[i] and lab2[i] == target
      // -> v1 =iso target
      // if orbits[target] == orbits[v2] then target =iso v2 thus v1 =iso target =iso v2
      // -> v1 =iso v2
      if(orbits[v2] == orbits[target]){
         return true;
      }
   }
   return false;
}

std::map<int, size_t> get_conf_distribution(const sparsegraph sg, const int v){
   std::map<int, size_t>distribution;
   int w;
   int eq_id;
   
   // Iterate over direct neighbors v
   for(int it = 0; it < sg.d[v]; ++it){
      // Get neighbor
      w = sg.e[sg.v[v] + it];

      // Get their equivalence id
      eq_id = eq_map[w];

      // Store in distribution
      if(distribution.find(eq_id) == distribution.end())
         distribution.insert({eq_id, 1});
      else 
         distribution[eq_id]++;
   }

   // Return distribution
   return distribution;
}

void update_eq_map(std::vector< std::vector<int> >eq){
   size_t counter = 0;
   for(size_t i = 0; i < eq_map.size(); i++){
      eq_map[i] = -1;
   }

   // Iterate over equivalence classes
   for(auto it : eq){
      for(int v : it){
         eq_map[v] = counter;
      }
      counter++;
   }
}

std::vector< std::vector< int > > split_equivalence_class(const sparsegraph sg, std::vector <int> eclass, const int d){
   std::vector< std::vector< int > >neweclass = {}; // Store equivalence classes
   std::vector< long > can_hash = {}; // For each eq. class, store hashes of generated canonical labelling
   std::vector< long > can_pos = {}; // And position in canonical labelling
   std::vector< int > it_classes;
   std::vector<int> class_id(sg.nv, -1); // Maps node to class id
   SG_DECL(sub1); SG_DECL(sub2);
   SG_DECL(cansub1); SG_DECL(cansub2);
   int n, m, nodes, edges, i;
   int cur1, cur2;
   int v1_pos, v2_pos;
   int orbit1, orbit2;
   long hashc1, hashc2; // Store hashed canonical labellings
   size_t tel, conf_id;
   bool added;
   clock_t t1, td;

   // Key -> equivalence class candidates
   std::map< std::pair<int, int>, std::vector< int > > class_key = {}; // Use nr. nodes + edges as map
   std::map< std::map<int, size_t>, std::vector< int > > distribution_keys = {}; // Use outdegree distribution as map
   std::map<int, size_t> distribution = {};

   // Declare required arrays
   DYNALLSTAT(int, lab, lab_sz);
   DYNALLSTAT(int, lab2, lab2_sz);
   DYNALLSTAT(int, ptn, ptn_sz);
   DYNALLSTAT(int, orbits, orbits_sz);
   DYNALLSTAT(int, orbits2, orbits2_sz);
   DYNALLSTAT(int, map, map_sz);
   static DEFAULTOPTIONS_SPARSEGRAPH(options);
   options.getcanon = TRUE;
   statsblk stats;
   
   // Return if class has size 1, can not split further
   if(d == 0 || eclass.size() == 1){
      neweclass.push_back(eclass);
      return neweclass;
   }
   n = sg.nv;
   m = sg.elen;

   // Allocate space for neighborhood graphs
   SG_ALLOC(sub1, n, m, "malloc");
   SG_ALLOC(sub2, n, m, "malloc");

   tel = 0;
   td = clock();
   if(debug_mode >= 3) fprintf(stderr, "/// Current nodes: ");
   
   // Iterate over nodes in current eclass, place them in new subclass
   for(auto it : eclass){
      added = false;
      cur1 = it;
      hashc1 = -1; // Default: not initialized
      v1_pos = -1;
      //if(cur1 == 0) continue;

      //Update statistics every 10 minutes:
      if(debug_mode >= 2 && ((double)(clock() - (double)td))/CLOCKS_PER_SEC > DEBUG_UPDATE_TIME){
         if(debug_mode >= 3) fprintf(stderr, "\n");
         fprintf(stderr, "//node %ld / %ld\n", tel + 1, eclass.size());
         fprintf(stderr, "//can nbh1 %ld\n", nr_can);
         fprintf(stderr, "//iso checks %ld\n", iso_checks);
         fprintf(stderr, "//new classes %ld\n", neweclass.size());
         fprintf(stderr, "//heuristic classes ");
         if(conf_choice == 2) fprintf(stderr, "%ld\n", distribution_keys.size());
         else fprintf( stderr, "%ld\n", class_key.size());
         fflush(stderr);
         td = clock();
         if(debug_mode >= 3) fprintf(stderr, "/// Current nodes: ");
      }
      if(debug_mode >= 3){
         fprintf(stderr, "%d, ", cur1);
         fflush(stderr);
      }
      tel += 1;

      // Get neighborhood of node to be placed
      get_neighborhood(sg, sub1, cur1, d);
      n = sub1.nv;
      m = sub1.nde;

      // Dynamic allocation for Traces
      DYNALLOC1(int, lab, lab_sz, n, "malloc");
      DYNALLOC1(int, lab2, lab2_sz, n, "malloc");
      DYNALLOC1(int, ptn, ptn_sz, n, "malloc");
      DYNALLOC1(int, orbits, orbits_sz,n, "malloc");
      DYNALLOC1(int, orbits2, orbits2_sz,n, "malloc");
      DYNALLOC1(int, map, map_sz, n, "malloc");

      // Get keys using selected heuristic
      if(conf_choice == CONF_NAIVE || conf_choice == CONF_ITERATIVE) // No heuristic
         it_classes = class_key[std::make_pair(0, 0)];
      else if(conf_choice == CONF_DEGREE){ // Outdegree distribution
         distribution = get_degree_distribution(sub1);
         it_classes = distribution_keys[distribution];
      }
      else if(conf_choice == CONF_EQ){
         if(d == 1) // At distance 1: use degree distribution
            distribution = get_degree_distribution(sub1);
         else
            distribution = get_conf_distribution(sg, it);
         it_classes = distribution_keys[distribution];
      }
      else // Use nr. nodes and edges of neighborhood (default)
         it_classes = class_key[std::make_pair(n, m)];

      // Try to place node in one of the found classes
      if(it_classes.size() != 0){

         // Get canonically labeled graph and position of it in lab1 array
         t1 = clock();
         sparsenauty(&sub1, lab, ptn, orbits, &options, &stats, &cansub1);

         if(print_time_can_labelling){
            printf("Can time first: %d, %f\n", sub1.nv, ((double)(clock() - (double)t1))/CLOCKS_PER_SEC);
            fflush(stdout);
         }

         // Find position
         for (i = 0; i < sub1.nv; i++){
            if(lab[i] == cur1) {
               v1_pos = i;
               break;
            }
         }

         hashc1 = get_hash(cansub1);
         orbit1 = orbits[cur1];
         nr_can++;
         for(int it2 = 0; it2 < it_classes.size(); it2++){
            conf_id = it_classes[it2]; // conf id
            cur2 = neweclass[conf_id][0]; // Get node from equivalence class
            // Get from cache
            if(use_cache && can_hash[conf_id] != -1 && can_pos[conf_id] != -1){
               hashc2 = can_hash[conf_id];
               v2_pos = can_pos[conf_id];
               if(v2_pos >= cansub1.nv) continue; 
               orbit2 = orbits[lab[v2_pos]];
            }
            // Compute canonical labelling of d-neighborhood
            else{
               get_neighborhood(sg, sub2, cur2, d);
               t1 = clock();
               sparsenauty(&sub2, lab2, ptn, orbits2, &options, &stats, &cansub2);
               if(print_time_can_labelling){
                  printf("Can time second: %d, %f\n", sub2.nv, ((double)(clock() - (double)t1))/CLOCKS_PER_SEC);
                  fflush(stdout);
               }
               //v2_pos is value of cur2 in canonical labelling (lab[i] s.t. i == cur2)
               for (i = 0; i < sub2.nv; i++){
                  if(lab2[i] == cur2) {
                     v2_pos = i;
                     break;
                  }
               }
               // Check if i is in range. Otherwise continue
               if(i > cansub1.nv) continue;
               // Get values to use
               hashc2 = get_hash(cansub2);
               orbit2 = orbits[lab[v2_pos]];
               // Store values
               can_hash[conf_id] = hashc2;
               can_pos[conf_id] = v2_pos;
               nr_can++;

            }

            // Compare on isomorphism and automorphism
            if(hashc1 == hashc2 && orbit1 == orbit2){
               iso_checks++;
               neweclass[conf_id].push_back(it);
               class_id[it] = conf_id; // Update class id of node it
               added = true;
               break;
            }
         }
      }
      // No equivalence class where node fits, create a new class
      if(!added){
         // Create new equivalence class
         class_id[it] = neweclass.size();
         neweclass.push_back({it});

         // Update keys
         if(conf_choice == CONF_NAIVE || conf_choice == CONF_ITERATIVE) 
            class_key[std::make_pair(0, 0)].push_back(neweclass.size() - 1); // Dummy key
         else if(conf_choice == CONF_DEGREE || conf_choice == CONF_EQ) 
            distribution_keys[distribution].push_back(neweclass.size() - 1); 
         else
            class_key[std::make_pair(n, m)].push_back(neweclass.size() - 1); // Default: n, m -> class nr
         
         // Update hash and position vector
         can_hash.push_back(hashc1);
         can_pos.push_back(v1_pos);
      }
   }

   // Free allocated memory
   SG_FREE(sub1);
   SG_FREE(sub2);
   SG_FREE(cansub1);
   SG_FREE(cansub2);
   if(debug_mode >= 3) fprintf(stderr, "\n");
   if(print_statistics >= 3){
      printf("final heuristic classes ");
      if(conf_choice == 2) printf("%ld\n", distribution_keys.size());
      else printf("%ld\n", class_key.size());
      fflush(stdout);
   }
   return neweclass;
}

std::vector< std::vector< int > > get_equivalence_classes(const sparsegraph sg, const int d){
   std::vector< std::vector<int> >eq, eq_stat, temp, neweq;
   std::map<int, std::set<int>> twin_node_map, twin_node_map_c;
   std::vector<int> node_set, nodes_giant;
   size_t i, tel;
   size_t it_iso_checks = 0;
   size_t tot_iso_checks = 0;
   size_t it_can = 0;
   size_t tot_can = 0;
   clock_t t2;
   float t1 = 0;
   twinnode_count = 0;
   
   t2 = clock();

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

   // Get set of all nodes: filter out twin nodes if any
   if(do_twin_node_check > 0){
      if(do_twin_node_check == 1){
         node_set = find_twin_nodes_open(sg, node_set, twin_node_map);
      }
      else{
         node_set = find_twin_nodes_all(sg, node_set, twin_node_map, twin_node_map_c);
      }
      // Output twin nodes
      if(print_eq_class){
         print_twinnodes(twin_node_map, 1);
         if(do_twin_node_check > 1)
            print_twinnodes(twin_node_map_c, 2);
      }
      fflush(stdout);
   }

   eq.push_back(node_set);

   if(conf_choice == CONF_EQ){
      eq_map.clear();
      for(size_t i = 0; i < sg.nv; i++)
         eq_map.push_back(0);
   }

   t1 += ((double)(clock() - (double)t2))/CLOCKS_PER_SEC;

   // Iteratively expand neighbourhood radius
   for(int i = 1; i <= d; i++){
      if(conf_choice == 0) i = d; // Naive computation -> start with i=d

      t2 = clock();
      tel = 0;
      it_iso_checks = 0;
      it_can = 0;

      // Iterate over equivalence classes and try to split them further
      for(auto it : eq){
         // Statistics
         iso_checks = 0;
         nr_can = 0;

         // Split current eq class
         temp = split_equivalence_class(sg, it, i);
         for(auto it2 : temp){
            neweq.push_back(it2);
         }

         // Print statistics for splitting eq class
         tel += 1;
         if(print_statistics >=3 && it.size() > 1){
            fprintf(stderr, "/Finished class %ld / %ld\n", tel, eq.size()); 
            fprintf(stderr, "/size %ld\n", it.size());
            fprintf(stderr, "/time %f\n", ((double)(clock() - (double)t2))/CLOCKS_PER_SEC);
            fprintf(stderr, "/can nbh1 %ld\n", nr_can);
            fprintf(stderr, "/iso checks %ld\n", iso_checks);
            fprintf(stderr, "/new classes %ld\n", temp.size());
            fflush(stderr);
         }
         it_iso_checks += iso_checks;
         it_can += nr_can;
      }
      // Update eq and clear neweq
      eq = neweq;
      neweq.clear();

      // Update statistics
      t1 += ((double)(clock() - (double)t2))/CLOCKS_PER_SEC;
      tot_iso_checks += it_iso_checks;
      tot_can += nr_can;

      // Print statistics, get eqclass with twin nodes
      if(do_twin_node_check && ( print_eq_class || print_statistics >=2)){
         eq_stat = process_twin_nodes(eq, twin_node_map);
         if(do_twin_node_check > 1)
            eq_stat = process_twin_nodes(eq_stat, twin_node_map_c);
      }
      else
         eq_stat = eq;

      // Update eq_map, include twin nodes
      if(conf_choice == CONF_EQ){
         update_eq_map(eq_stat);
      }

      if(print_statistics >= 2){
         printf("N%d of %d:\n", i, d);
         printf("time it %d: %f\n", i, ((double)(clock() - (double)t2))/CLOCKS_PER_SEC);
         printf("k it %d: %d\n", i, get_k(eq_stat));
         printf("can nbh it %d: %ld\n", i, it_can);
         printf("iso checks it %d: %ld\n", i, it_iso_checks);
         print_statistics_eq(eq_stat, i);
         if(do_twin_node_check >= 1){
            printf("No twins ");
            print_statistics_eq(eq, i);
            size_t unique = get_uniquely_identifiable_nodes(eq, twin_node_map, twin_node_map_c);
            printf("Fraction identifiable nodes %d: %ld\n", i, unique);
         }
      }
      if(print_eq_class){
         print_equivalence_classes(eq_stat);
      }
      fflush(stdout);
      eq_stat.clear();
   }

   // Print statistics
   if(do_twin_node_check){
      eq = process_twin_nodes(eq, twin_node_map);
      eq = process_twin_nodes(eq, twin_node_map_c);
   }
   if(print_statistics >= 1){
      printf("\nFinal results.\n");
      printf("tot time: %f\n", t1);
      printf("final k: %d\n", get_k(eq));
      printf("tot can nbh: %ld\n", tot_can);
      printf("tot iso checks: %ld\n", tot_iso_checks);
      printf("Node set size: %ld\n", node_set.size());

      if(do_twin_node_check > 0){
         print_twinnode_stats(twin_node_map, 1);
         if(do_twin_node_check >= 2)
            print_twinnode_stats(twin_node_map_c, 2);
      }
      print_statistics_eq(eq, d);
      printf("\n");
   }
   fflush(stdout);
   return eq;
}

std::vector< std::vector< int > > split_equivalence_class_directed(const sparsegraph sgo, const sparsegraph sgi, std::vector <int> eclass, const int d){
   std::vector< std::vector< int > >neweclass = {};
   std::vector< int > it_classes;
   SG_DECL(sub1); SG_DECL(sub2);
   SG_DECL(cansub1); SG_DECL(cansub2);
   int cur1, cur2, n, m, nodes, edges, i, v1_pos;
   size_t tel;
   clock_t td;
   bool added;

   // Keys
   std::map< std::pair<int, int>, std::vector< int > > class_key; // Use nr. nodes + edges as map
   std::map< std::map<int, size_t>, std::vector< int > > outdeg_keys; // Use outdegree distribution as map
   std::map<int, size_t> outdegs;

   // Declare required arrays
   DYNALLSTAT(int, lab, lab_sz);
   DYNALLSTAT(int, ptn, ptn_sz);
   DYNALLSTAT(int, orbits, orbits_sz);
   DYNALLSTAT(int, map, map_sz);
   static DEFAULTOPTIONS_SPARSEGRAPH(options);
   options.getcanon = TRUE;
   statsblk stats;
   
   // Return if class has size 1, can not split further
   if(d == 0 || eclass.size() == 1){
      neweclass.push_back(eclass);
      return neweclass;
   }
   n = sgo.nv;
   m = sgo.elen;

   // Allocate space for neighborhood graphs
   SG_ALLOC(sub1, n, m, "malloc");
   SG_ALLOC(sub2, n, m, "malloc");

   tel = 0;

   if(debug_mode >= 3){
      td = clock();
      printf("Current nodes: ");
   }

   // Iterate over nodes in current eclass, place them in new subclass
   for(auto it : eclass){
      added = false;
      cur1 = it;

      //Update statistics every 10 minutes:
      if(debug_mode >= 2 && ((double)(clock() - (double)td))/CLOCKS_PER_SEC > DEBUG_UPDATE_TIME){
         if(debug_mode >= 3) fprintf(stderr, "\n");
         fprintf(stderr, "//node %ld / %ld\n", tel + 1, eclass.size());
         fprintf(stderr, "//can nbh1 %ld\n", nr_can);
         fprintf(stderr, "//iso checks %ld\n", iso_checks);
         fprintf(stderr, "//new classes %ld\n", neweclass.size());
         fprintf(stderr, "//heuristic classes ");
         td = clock();
         if(debug_mode >= 3) fprintf(stderr, "/// Current nodes: ");
         fflush(stderr);
      }
      if(debug_mode >= 3){
         fprintf(stderr, "%d, ", cur1);
         fflush(stderr);
      }
      tel += 1;

      // Get neighborhood of node to be placed
      get_neighborhood_directed(sgo, sgi, sub1, cur1, d);
      n = sub1.nv;
      m = sub1.nde;

      // Dynamic allocation for Traces
      DYNALLOC1(int, lab, lab_sz, n, "malloc");
      DYNALLOC1(int, ptn, ptn_sz, n, "malloc");
      DYNALLOC1(int, orbits, orbits_sz,n, "malloc");
      DYNALLOC1(int, map, map_sz, n, "malloc");

      // Get keys using selected heuristic
      if(conf_choice == CONF_NAIVE || conf_choice == CONF_ITERATIVE) // No heuristic
         it_classes = class_key[std::make_pair(0, 0)];
      else if(conf_choice == CONF_DEGREE){ // Outdegree distribution
         outdegs = get_degree_distribution_directed(sub1);
         it_classes = outdeg_keys[outdegs];
      }
      else // Use nr. nodes and edges of neighborhood (default)
         it_classes = class_key[std::make_pair(n, m)];

      // Try to place node in one of the found classes
      if(it_classes.size() != 0){
         
         // Get canonically labeled graph and position of it in lab1 array
         sparsenauty(&sub1, lab, ptn, orbits, &options, &stats, &cansub1);
         for (i = 0; i < n; i++){
            if(lab[i] == cur1) {
               v1_pos = i; 
               break;
            }
         }
         nr_can++;
         
         for(int it2 = 0; it2 < it_classes.size(); it2++){

            // Get d-neighborhood of second node + canonically labeled graph
            cur2 = neweclass[it_classes[it2]][0]; // A node in selected class of neweclass
            get_neighborhood_directed(sgo, sgi, sub2, cur2, d);
            sparsenauty(&sub2, lab, ptn, orbits, &options, &stats, &cansub2);
            
            // Compare on isomorphism and automorphism
            if(are_same_sg_can(&cansub1, &cansub2, cur1, cur2, v1_pos, lab, orbits)){
               neweclass[it_classes[it2]].push_back(it);
               added = true;
               break;
            }
         }
      }
      // No class where it fits in, create a new class
      if(!added){
         neweclass.push_back({it});
         if(conf_choice == CONF_NAIVE || conf_choice == CONF_ITERATIVE) class_key[std::make_pair(0, 0)].push_back(neweclass.size() - 1);
         else if(conf_choice == CONF_DEGREE) outdeg_keys[outdegs].push_back(neweclass.size() - 1); 
         else class_key[std::make_pair(n, m)].push_back(neweclass.size() - 1); // Default: n, m -> class nr
      }
   }

   if(print_statistics >= 3){
      printf("final heuristic classes ");
      if(conf_choice == 2) printf("%ld\n", outdeg_keys.size());
      else printf("%ld\n", class_key.size());
      fflush(stdout);
   }

   // Free allocated memory
   SG_FREE(sub1);
   SG_FREE(sub2);
   SG_FREE(cansub1);
   SG_FREE(cansub2);
   
   return neweclass;
}

std::vector< std::vector< int > > get_equivalence_classes_directed(const sparsegraph sgo, const sparsegraph sgi, const int d){
   std::vector<int> all;
   std::vector< std::vector<int> >eq, temp, neweq;
   clock_t t2; 
   double t1 = 0.0;
   size_t i, n, tel;
   size_t it_iso_checks = 0;
   size_t tot_iso_checks = 0;
   size_t it_can = 0;
   size_t tot_can = 0;
   n = sgo.nv;

   // Start with class of all nodes (no information, all equivalent)
   for(int i = 0; i < n; i++){
      all.push_back(i);
   }
   eq.push_back(all);

   // Expand neighborhood
   for(int i = 1; i <= d; i++){
      t2 = clock();
      tel = 0;
      it_iso_checks = 0;
      it_can = 0;

      // Iterate over equivalence classes and try to split them further
      for(auto it : eq){
         iso_checks = 0;
         nr_can = 0;
         temp = split_equivalence_class_directed(sgo, sgi, it, i);
         for(auto it2 : temp){
            neweq.push_back(it2);
         }
         tel += 1;
         if(debug_mode >= 1 && it.size() > 1){
            fprintf(stderr, "/Finished class %ld / %ld\n", tel, eq.size()); 
            fprintf(stderr, "/size %ld\n", it.size());
            fprintf(stderr, "/time %f\n", ((double)(clock() - (double)t2))/CLOCKS_PER_SEC);
            fprintf(stderr, "/can nbh1 %ld\n", nr_can);
            fprintf(stderr, "/iso checks %ld\n", iso_checks);
            fprintf(stderr, "/new classes %ld\n", temp.size());
            fflush(stderr);
         }
         it_iso_checks += iso_checks;
         it_can += nr_can;
      }
      eq = neweq;
      neweq.clear();
      t1 += ((double)(clock() - (double)t2))/CLOCKS_PER_SEC;
      tot_iso_checks += it_iso_checks;
      tot_can += nr_can;

      // Print statistics
      if(print_statistics >= 2){
         printf("N%d of %d:\n", i, d);
         printf("time it %d: %f\n", i, ((double)(clock() - (double)t2))/CLOCKS_PER_SEC);
         printf("k it %d: %d\n", i, get_k(eq));
         printf("can nbh1 it %d: %ld\n", i, it_can);
         printf("iso checks it %d: %ld\n", i, it_iso_checks);
         print_statistics_eq(eq, i);
      }
      if(print_eq_class){
         print_equivalence_classes(eq);
      }
      fflush(stdout);
   }
   if(print_statistics >= 1){
      printf("\nFinal results.\n");
      printf("tot time %f\n", t1);
      printf("tot can nbh1 %ld\n", tot_can);
      printf("tot iso checks %ld\n", tot_iso_checks);
      if(print_statistics == 1){
         print_statistics_eq(eq, d);
      }
      fflush(stdout);
   }
   return eq;
}

int get_k(const std::vector< std::vector< int > > eclasses){
   int min_size;
   int cur_size;

   if(eclasses.size() == 0)
      return 0;
   
   min_size = eclasses[0].size();

   for(auto it : eclasses){
      cur_size = it.size();
      if(cur_size < min_size)
         min_size = cur_size;
   }
   return min_size;
}

void print_equivalence_classes(const std::vector< std::vector < int > > eclasses){
   printf("Start equivalence classes.\n");
   for(auto it : eclasses){
      for(auto it2 : it)
         printf("%d ", it2);
      printf("\n");
   }
   printf("End equivalence classes.\n");
}

void print_equivalence_classes_to_file(const std::vector < std::vector< int > > eclasses, char * file_name){
   FILE *fp;
   fp = fopen(file_name, "w+");
   
   for(auto it : eclasses){
      for(auto it2 : it)
         fprintf(fp, "%d ", it2);
      fprintf(fp, "\n");
   }
   fclose(fp);
}

void print_statistics_eq(const std::vector <std::vector <int > > eclasses, const int it){
   std::map<int, int> sizes;
   int max, min, cur;
   max = eclasses[0].size();
   min = eclasses[0].size();
   for(auto it : eclasses){
      cur = it.size();
      if(sizes.find(cur) == sizes.end()){
         sizes.emplace(cur, 0);
      }
      sizes[cur]++;
      if(cur < min) min = cur;
      if(cur > max) max = cur;
   }

   printf("eq %d: ", it);
   int nodes = 0;
   for(auto it : sizes){
      printf("[%d, %d], ", it.first, it.second);
      nodes += it.first * it.second;
   }
   printf("\n");
   printf("Nodes: %d\n", nodes);
   fflush(stdout);
}
