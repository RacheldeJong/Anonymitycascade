#include "util.h"
#include "./graph/graphgen.h"
#include "./graph/graphutil.h"
#include "./measure/dk-anonymity.h"
#include "./measure/cascading.h"
#include "./equivalence/equivalence.h"

// Default settings
bool skip_cascading = false;
bool cascading_distribution = false;
bool twin_nbs_all = true;
int do_twin_node_check = 2;

// Read number of nodes from file
int read_n(const char* file_name){
   FILE* file;
   char c;
   int n = 0;

   if((file = fopen(file_name, "r")) == NULL){ 
      printf("Error: File %s could not be opened\n", file_name);
      return 0;
   }
   // Read n
   while (c != EOF){
      if(c == '\n' && (c=getc(file)) != '!') break;
      if(c == 'n' && (c=getc(file)) == '='){
         c = getc(file);
         while (ISDIGIT(c)){
            n *= 10;
            n += c - '0';
            c = getc(file);
         }
      }
      else c = getc(file);
   }
   return n;
}

// Print settings used
void print_info(const sparsegraph sg1, const int directed, const int distance){
   bool print_graph_info = true;
   if(print_graph_info == false) return;
   
   printf("Info:\n");
   printf("- Graph contains %d nodes\n", sg1.nv);
   printf("- Graph contains %ld edges\n", sg1.nde);
   printf("- Cascading: %d\n", !skip_cascading);
   printf("- Directed: %d\n", directed);
   printf("- Distance: %d\n", distance);
   printf("- Configuration choice = %d: ", conf_choice);
   
   if(conf_choice == CONF_NAIVE) printf("Naive.\n");
   else if(conf_choice == CONF_ITERATIVE) printf("Iterative.\n");
   else if(conf_choice == CONF_EQ) printf("Iterative + equivalence distribution\n");
   else if(conf_choice == CONF_DEGREE) printf("Iterative + out-degree distribution\n");
   else{
      conf_choice = CONF_COUNT; 
      printf("Iterative + count nodes and edges\n");
   }

   printf("- Twin node check : %d\n", do_twin_node_check);
   printf("  Max neighbours : %d\n", twin_nbs);

   printf("- Print statistics classes = %d: ", print_statistics);
   
   if(print_statistics <= 0) printf("none.\n\n");
   else if(print_statistics == 1) printf("Final statistics only\n");
   else if(print_statistics == 2) printf("Per iteration\n");
   else if(print_statistics == 3) printf("Per class split\n");
   else if(print_statistics >= 4) printf("All, including updates\n");
      
   printf("- Print equivalence classes = %d\n", print_eq_class);
   printf("- Print canonical labelling runtime = %d\n\n", print_time_can_labelling);

   // Warning for directed:
   if(directed == 1 && do_twin_node_check)
      printf("WARNING: twin node check is not implemented for directed graphs. This step is skipped\n");
   if(directed == 1 && conf_choice == CONF_EQ){
      printf("WARNING: CONF_EQ is not implemented for directed graphs. Changing to default: COUNT\n");
      conf_choice = CONF_COUNT;
   }
   fflush(stdout);
}

// Parse command line arguments
void parse_input(int argc, char* argv[], int & directed, int &distance){
   for(int i =1; i < argc; i++){
      if(strcmp(argv[i], "-d") == 0){
         distance = (atoi(argv[i + 1]));
         i++;
      }
      else if(strcmp(argv[i], "-dk") == 0){
         skip_cascading = true;
      }
      else if(strcasecmp(argv[i], "-g") == 0){
         filter_giant = 1;
      }
      else if(strcmp(argv[i], "-h") == 0){
         conf_choice = (atoi(argv[i + 1]));
         i++;
      }
      else if(strcmp(argv[i], "-c") == 0){
         do_twin_node_check = (atoi(argv[i + 1]));
      }
      else if(strcmp(argv[i], "-cs") == 0){
         twin_nbs = atoi(argv[i + 1]);
         i++;
      }
      else if(strcmp(argv[i], "-csa") == 0){
         twin_nbs_all = true;
      }
      else if(strcmp(argv[i], "-eq") == 0){
         print_eq_class = 1;
      }
      else if(strcmp(argv[i], "-cd") == 0){
         cascading_distribution = 1;
      }
      else if(strcmp(argv[i], "-s") == 0){
         print_statistics = atoi(argv[i + 1]);
         i++;
      }
      else if(strcasecmp(argv[i], "-t") == 0){
         print_time_can_labelling = 1;
      }
   }
}

int main(int argc, char *argv[]){
   // Read graph
   // Initialize variables
   SG_DECL(sg1); SG_DECL(sg2);
   int m, n = 0;
   char *input_file;

   // Default values
   int directed = 0;
   int distance = 1;
   clock_t t1;

   // Check and parse input
   if(argc < 2){
      printf("Error not enough arguments: need input graph\n");
      exit(1);
   }
   input_file = argv[1];
   parse_input(argc, argv, directed, distance);

   // Read graph
   t1 = clock();
   n = read_n(input_file);
   m = SETWORDSNEEDED(n);
   sg1 = read_graph_from_file(input_file, n);
   print_info(sg1, directed, distance);
   // Read eq classes from file
   std::vector<std::vector<int>> eqs;
   std::vector<int> node_to_eq, unique_casc;
   eqs = get_equivalence_classes(sg1, distance);
   node_to_eq = get_node_to_eq(eqs, n);

   if(!skip_cascading){
      unique_casc = cascading(sg1, eqs, node_to_eq);
   }
   return(0);
}