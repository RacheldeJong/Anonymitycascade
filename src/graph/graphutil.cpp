#include "graphutil.h"

std::unordered_set<int> get_neighborhood_nodes(const sparsegraph sg, const int v, const int d, int &edges){
   std::queue< int > q; // Queue with nodes to be visited
   std::unordered_set<int> vis; //Visited nodes (unordered_set for quick access when in set)
   
   int cur, nid;
   int depth = 0;
   int time_to_increase_depth = 1;
   edges = 0;
   vis.clear();
   q.push(v);
   vis.insert(v);

   // Iterate over nodes to be visited
   while(!q.empty()){
        
      cur = q.front();
      // Keep track of when to increase depth of search
      if((time_to_increase_depth -= 1) == 0){
         if(++depth > d) break;
         time_to_increase_depth = q.size();
      }
      q.pop();

      // Iterate over edges of current node
      for(int node_it = 0; node_it < sg.d[cur]; ++node_it){
         nid = sg.e[sg.v[cur] + node_it];
         edges++;
         // Do not add to queue again if node is already added
         if(vis.find(nid) != vis.end()) continue;
         q.push(nid);
         vis.insert(nid);
      }
   }
   return vis;
}

std::unordered_set<int> get_neighborhood_nodes_directed(const sparsegraph sgo, const sparsegraph sgi, const int v, const int d, int &edges){
   std::queue< int > q; // Queue with nodes to be visited
   std::unordered_set<int> vis; //Visited nodes (unordered_set for quick access when in set)
   
   int cur, nid;
   int depth = 0;
   int time_to_increase_depth = 1;
   edges = 0;
   vis.clear();
   q.push(v);
   vis.insert(v);

   // Iterate over nodes to be visited
   while(!q.empty()){
        
      cur = q.front();
      // Keep track of when to increase depth of search
      if((time_to_increase_depth -= 1) == 0){
         if(++depth > d) break;
         time_to_increase_depth = q.size();
      }
      q.pop();

      // Iterate over outgoing edges of current node
      for(int node_it = 0; node_it < sgo.d[cur]; ++node_it){
         nid = sgo.e[sgo.v[cur] + node_it];
         edges++;
         // Do not add to queue again if node is already visited
         if(vis.find(nid) != vis.end()) continue;
         q.push(nid);
         vis.insert(nid);
      }

      // Iterate over ingoing edges of current node
      for(int node_it = 0; node_it < sgi.d[cur]; ++node_it){
         nid = sgi.e[sgi.v[cur] + node_it];
         edges++;
         // Do not add to queue again if node is already visited
         if(vis.find(nid) != vis.end()) continue;
         q.push(nid);
         vis.insert(nid);
      }
   }

   return vis;
}

std::unordered_set<int> get_neighborhood_nodes_directed_distribution(const sparsegraph sgo, const sparsegraph sgi, const int v, const int d, int &edges){
   std::queue< int > q; // Queue with nodes to be visited
   std::unordered_set<int> vis; //Visited nodes (unordered_set for quick access when in set)
   
   int cur, nid;
   int depth = 0;
   int time_to_increase_depth = 1;
   edges = 0;
   vis.clear();
   q.push(v);
   vis.insert(v);

   // Iterate over nodes to be visited
   while(!q.empty()){
        
      cur = q.front();
      // Keep track of when to increase depth of search
      if((time_to_increase_depth -= 1) == 0){
         if(++depth > d) break;
         time_to_increase_depth = q.size();
      }
      q.pop();

      // Iterate over edges of current node
      for(int node_it = 0; node_it < sgo.d[cur]; ++node_it){
         nid = sgo.e[sgo.v[cur] + node_it];
         edges++;
         // Do not add to queue again if node is already visited
         if(vis.find(nid) != vis.end()) continue;
         q.push(nid);
         vis.insert(nid);
      }

      for(int node_it = 0; node_it < sgi.d[cur]; ++node_it){
         nid = sgi.e[sgi.v[cur] + node_it];
         edges++;
         // Do not add to queue again if node is already visited
         if(vis.find(nid) != vis.end()) continue;
         q.push(nid);
         vis.insert(nid);
      }
   }
   return vis;
}

void get_neighborhood(const sparsegraph sg1, sparsegraph &sg2,int &v, const int d){
   std::unordered_map<int, int> map; // Maps each node in sg to new node in sg2
   int deg, pos, pos1, to, edges;

   // Get nodes in k-neighborhood
   std::unordered_set<int> nodes = get_neighborhood_nodes(sg1, v, d, edges);
   int n = nodes.size();

   // Initialize sub-graph
   sg2.nv = nodes.size(); // Set number of nodes
   sg2.nde = 0;

   // Initialize map sg->sg2
   n = 0;
   for(auto node_id : nodes){
      map[node_id] = n;
      n++;
   }

   pos = 0;
   // For each node in the subgraph
   for(auto node_id : nodes){
      deg = 0;
      sg2.v[map[node_id]] = pos; // Set position of node in sg2
      pos1 = sg1.v[node_id]; 
      
      // Iterate over edges from it in original graph
      for(int i = 0; i < sg1.d[node_id]; i++){
         to = sg1.e[pos1 + i];

         // Check if edge is contained in subgraph
         if(nodes.find(to) == nodes.end()) continue;

         // Otherwise add edge to subgraph
         sg2.nde++;
         sg2.e[pos + deg] = map[to];
         deg++;
      }

      sg2.d[map[node_id]] = deg;
      pos += deg;
   }
   v = map[v]; // Set v to corresponding node in new graph
}

void get_neighborhood_directed(const sparsegraph sgo, const sparsegraph sgi, sparsegraph &sg2, int &v, const int d){
   std::unordered_map<int, int> map; // Maps each node in sg to new node in sg2
   int deg, pos, pos1, to, edges;

   // Get nodes in k-neighborhood
   std::unordered_set<int> nodes = get_neighborhood_nodes_directed(sgo, sgi, v, d, edges);
   int n = nodes.size();

   // Initialize sub-graph
   sg2.nv = nodes.size(); // Set number of nodes
   sg2.nde = 0;

   // Initialize map sg->sg2
   n = 0;
   for(auto node_id : nodes){
      map[node_id] = n;
      n++;
   }

   pos = 0;
   // For each node in the subgraph
   for(auto node_id : nodes){
      deg = 0;
      sg2.v[map[node_id]] = pos; // Set position of node in sg2
      pos1 = sgo.v[node_id];
      
      // Iterate over edges from it in original graph
      for(int i = 0; i < sgo.d[node_id]; i++){
         to = sgo.e[pos1 + i];

         // Check if edge is contained in subgraph
         if(nodes.find(to) == nodes.end()) continue;

         // Otherwise add edge to subgraph
         sg2.nde++;
         sg2.e[pos + deg] = map[to];
         deg++;
      }

      sg2.d[map[node_id]] = deg;
      pos += deg;
   }
   v = map[v]; // Set v to corresponding node in new graph
}

std::vector<int> get_giant_component(const sparsegraph sg1){
   size_t max_size = sg1.vlen;
   std::vector<int> node_set;
   std::queue<int> fringe;
   std::set<int> visited, bfs;
   size_t w, nbr;
   float threshold = GIANT_FRAC_THRESHOLD*sg1.nv;

   // Iterate over nodes
   for(size_t v = 0; v < sg1.vlen; v++){
      // Visit only nodes that have not been added yet
      if(visited.find(v) != visited.end())
         continue;
      bfs.clear();
      fringe.push(v);
      bfs.insert(v);
      visited.insert(v);
      
      // Do BFS from node
      while(!fringe.empty()){
         w = fringe.front();
         fringe.pop();
         size_t start = sg1.v[w];
         for(int i = 0; i < sg1.d[w]; i++){
            nbr = sg1.e[start + i];
            if(visited.find(nbr) == visited.end()){
               fringe.push(nbr);
               bfs.insert(nbr);
            }
         }
         visited.insert(w);
      }

      // If component larger than threshold, add to node set
      if(bfs.size() > threshold){
         for(auto it : bfs)
            node_set.push_back(it);
      }
   }
   return node_set;
}

std::map<int, size_t> get_degree_distribution(const sparsegraph sg){
   std::map<int, size_t> degrees;
   int degree;

   for(size_t i = 0; i < sg.nv; i++){
      degree = sg.d[i];
      
      if(degrees.find(degree) == degrees.end()) degrees.insert({degree, 1});
      else degrees[degree]++;
   }
   return degrees;
}

std::map<int, size_t> get_degree_distribution_directed(const sparsegraph sg){
   std::map<int, size_t> outdegs;
   int outdeg;

   for(size_t i = 0; i < sg.nv; i++){
      outdeg = sg.d[i];
      
      if(outdegs.find(outdeg) == outdegs.end()) outdegs.insert({outdeg, 1});
      else outdegs[outdeg]++;
   }
   return outdegs;
}

long get_hash(sparsegraph sg){
   return hashgraph_sg(&sg, HASHKEY);
}

void print_graph(const sparsegraph sg){
   int n = sg.nv;
   int pos;

   for(int i = 0; i < n; i++){
      printf("%d: ", i);
      pos = sg.v[i];
      for(int j = 0; j < sg.d[i]; j++ ){
         printf("%d, ", sg.e[pos + j]);
      }
      printf("\n");
   }
}