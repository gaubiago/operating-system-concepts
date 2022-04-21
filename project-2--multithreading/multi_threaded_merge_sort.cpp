#include <iostream>
#include <cctype>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <thread>

#define LOWER_BOUND 0
#define USAGE_ERROR 1

void print_usage();
void validate_argv(int& argc, char* argv[]);
std::string dump_partition(std::vector<int>& partition);
void generate_list(std::vector<int>& rand_int_list, const size_t& n, const size_t& upper_bound);
void print_list(std::vector<int>& list);
void create_partitions(const std::vector<int>& rand_int_list, std::vector< std::vector<int> >& rand_int_partitions,
                       const size_t& n, const size_t& p);
void print_partitions(std::vector< std::vector<int> >& partitions);
void cpp_sort(std::vector<int>& partition);
void display_merge_msg(std::vector<int>& in_part_a, std::vector<int>& in_part_b, std::vector<int>& out_part);
void cpp_merge(std::vector<int>& in_part_a, std::vector<int>& in_part_b, std::vector<int>& out_part);
void merge_partitions_multithreaded(std::vector< std::vector<int> >& rand_int_partitions, std::vector<std::thread>& threads,
                                    const size_t& p);

int main(int argc, char* argv[]) {
  // Validate arguments that are passed in upon running executable
  validate_argv(argc, argv);

  // n: Number of elements to be generated
  const size_t n = std::stoi(argv[1]);
  // upper_bound: Max. possible value of an integer element
  const size_t upper_bound = std::stoi(argv[2]);
  // p: Number of partitions to be created
  const size_t p = std::stoi(argv[3]);
  
  // Generate list of random integers
  std::vector<int> rand_int_list;
  generate_list(rand_int_list, n, upper_bound);
  print_list(rand_int_list);

  // No need for partitioning, sorting, and merging if list has only 1 element
  if (rand_int_list.size() == 1) {
    std::cout << std:: endl << "List is already sorted." << std::endl;
    return 0;
  }

  // Break down list into partitions
  std::vector< std::vector<int> > rand_int_partitions(p);
  create_partitions(rand_int_list, rand_int_partitions, n, p);
  std::cout << "\nList breakdown into partitions:\n";
  print_partitions(rand_int_partitions);

  // Sort partitions
  std::vector<std::thread> threads;
  for (int i = 0; i < p; i++) {
    threads.push_back(std::thread(cpp_sort, std::ref(rand_int_partitions[i])));
  }
  // Synchronize all threads
  for (auto& th : threads) 
    th.join(); 
  std::cout << "\nPartitions after multithreaded sorting:\n";
  print_partitions(rand_int_partitions);

  // Merge partitions in a multithreaded and sorted way, building back single list
  std::cout << "\nMultithreaded merging of partitions:\n";
  merge_partitions_multithreaded(rand_int_partitions, threads, p);

  return 0;
}

void print_usage() {
  std::cout << "Usage:" << std::endl << std::endl
	    << "    multi_threaded_merge_sort <N> <MAX_VALUE> <P>" << std::endl << std::endl
	    << "where" << std::endl << std::endl
	    << "    <N> is an positive integer representing the size of your list of elements" << std::endl
	    << "    <MAX_VALUE> is an positive integer representing the possible max. value of the list elements " << std::endl
      << "    <P> is an positive integer representing the intended number of partitions to break down the list into" << std::endl
	    << std::endl
	    << "Example:" << std::endl
	    << "    $ ./multi_threaded_merge_sort 25 100 5" << std::endl
	    << std::endl;
}

void validate_argv(int& argc, char* argv[]) {
  if(argc != 4) {
    std::cout << "Invalid number of arguments." << std::endl;
    print_usage();
    exit(USAGE_ERROR);
  }

  // n: Number of elements to be generated
  const std::string n_str(argv[1]);
  for (int i = 0; i < n_str.size(); i++) {
    if (!isdigit(n_str[i])) {
      std::cout << "Invalid list size." << std::endl;
      print_usage();
      exit(USAGE_ERROR);
    }
  }

  // upper_bound: The biggest integer element the list can have
  std::string upper_bound_str(argv[2]);
  for (int i = 0; i < upper_bound_str.size(); i++) {
    // Negative integers are not accepted (handled by condition below)
    if (!isdigit(upper_bound_str[i])) {
      std::cout << "Invalid max. possible value for any list element." << std::endl;
      print_usage();
      exit(USAGE_ERROR);
    }
  }

  // p: Number of partitions to be created
  const std::string p_str(argv[3]);
  for (int i = 0; i < p_str.size(); i++) {
    if (!isdigit(p_str[i])) {
      std::cout << "Invalid number of intended partitions." << std::endl;
      print_usage();
      exit(USAGE_ERROR);
    }
  }  

  // Stop execution if #partitions > #elements
  if (std::stoi(argv[3]) > std::stoi(argv[1])) {
    std::cout << "The number of elements in the list (1st arg.) has to be bigger the the number of intended partitions (3rd arg.)." 
              << std::endl;
    exit(USAGE_ERROR);
  }
}

std::string dump_partition(std::vector<int>& partition) {
  // Build partition string
  std::string str("");
  str += '[';
  for (int i = 0; i < partition.size(); i++) {
    str += std::to_string(partition[i]);
    if (i != partition.size()-1)
      str += ' ';
  }
  str += ']';
  return str;
}

void generate_list(std::vector<int>& rand_int_list, const size_t& n, const size_t& upper_bound) {
  // Initialize random seed
  srand (time(NULL));

  for (int i = 0, tmp; i < n; i++) {
    tmp = rand()%(!upper_bound ? 1 : upper_bound+1) + LOWER_BOUND;
    rand_int_list.push_back(tmp);
  }
}

void print_list(std::vector<int>& list) {
  std::cout << "Generated list of random integers:" << "\n\n  "
            << dump_partition(list) << std::endl << std::flush;
}

void create_partitions(const std::vector<int>& rand_int_list, std::vector< std::vector<int> >& rand_int_partitions,
                       const size_t& n, const size_t& p) {
  // quotient: Partition size
  size_t quotient = n/p; 
  // remainder: Extra elements to be distributed into a required # of partitions
  size_t remainder = n%p;
  // offset: Number of extra elements added individually to different partitions
  size_t offset = 0;

  for (int i = 0, j = 0; i < n; i++) {
    // Whenever a partition reaches its indended, even size and it's not the first iteration
    if ((i-offset)%(quotient) == 0 && i != 0) {
      // If there is any remainder, add the next element in the list to the current partition 
      if (remainder) {
        rand_int_partitions[j].push_back(rand_int_list[i]);
        remainder--;
        i++;
        offset++;
      }
      j++;
    }
    rand_int_partitions[j].push_back(rand_int_list[i]);
  }
}

void print_partitions(std::vector< std::vector<int> >& partitions) {
  std::cout << std::endl;
  for (int i = 0; i < partitions.size(); i++) {
    std::cout << "  Part. " << i << ": " << dump_partition(partitions[i]) << " (size=" << partitions[i].size() << ")"
              << std::endl << std::flush;
  }
}

void cpp_sort(std::vector<int>& partition) {
  std::sort(partition.begin(), partition.end());
}

void display_merge_msg(std::vector<int>& in_part_a, std::vector<int>& in_part_b, std::vector<int>& out_part) {
  std::string str("\n * Merged \n\t" + dump_partition(in_part_a) + " (size=" + std::to_string(in_part_a.size()) + ")" + " and " 
                            + "\n\t" + dump_partition(in_part_b) + " (size=" + std::to_string(in_part_b.size()) + ")" 
                + "\n    - Result: \n\t" + dump_partition(out_part) + " (size=" + std::to_string(out_part.size()) + ")\n");
  std::cout << str;
}

void cpp_merge(std::vector<int>& in_part_a, std::vector<int>& in_part_b, std::vector<int>& out_part) {
  out_part.resize(in_part_a.size() + in_part_b.size());
  std::merge(in_part_a.begin(), in_part_a.end(), in_part_b.begin(), in_part_b.end(), out_part.begin());
  display_merge_msg(in_part_a, in_part_b, out_part);
}

void merge_partitions_multithreaded(std::vector< std::vector<int> >& rand_int_partitions, std::vector<std::thread>& threads,
                                    const size_t& p) {
  // input_partitions: partitions before merging operations
  // output_partitions: partitions after merting operations
  // pass_last_partitions: last partition of the list of partitions at each pass (when p_before_merge is odd)
  std::vector< std::vector<int> > input_partitions, output_partitions, pass_last_partitions;
  // last_partitions_merge_output: result of the merging of two pass_last_partitions
  std::vector<int> last_partitions_merge_output;
  // p_before_merge: number of partitions before merging operations
  size_t p_before_merge = p;
  // p_after_merge: number of partitions after merging operations
  size_t p_after_merge = p/2;
  // counter: pass counter
  size_t counter = 0;

  input_partitions = rand_int_partitions;

  while (p_after_merge > 0) {
    std::cout << "\n-----------------------------   PASS " << ++counter << "   -----------------------------" << std::endl;

    threads.clear();
    output_partitions.resize(p_after_merge);

    // Do multithreaded merging operations
    for (int i = 0, j = 0; i < p_before_merge-1; i += 2, j++) {
      threads.push_back(std::thread(cpp_merge, std::ref(input_partitions[i]), std::ref(input_partitions[i+1]), std::ref(output_partitions[j])));
    }

    // Is the number of partitions before the merging operations odd?
    if (p_before_merge%2) {
      // Append last partition of the list of partitions
      pass_last_partitions.push_back(input_partitions[p_before_merge-1]);
      // Carry out multithreaded merging operation if the vector's size is 2
      if (pass_last_partitions.size() == 2) {
        threads.push_back(std::thread(cpp_merge, std::ref(pass_last_partitions[0]), std::ref(pass_last_partitions[1]), std::ref(last_partitions_merge_output)));
      }
    }

    // Synchronize all threads
    for (auto& th : threads) 
      th.join();

    // Is the number of partitions before the merging operations odd, and is pass_last_partitions' size 2?
    if (p_before_merge%2 && pass_last_partitions.size() == 2) {
      // Clear pass_last_partitions
      pass_last_partitions.clear();
      // Print out the result of the merging operation
      dump_partition(last_partitions_merge_output);
      // Append the resulting partition to pass_last_partitions
      pass_last_partitions.push_back(last_partitions_merge_output);
    }

    std::cout << "\nPartitions after multithreaded merging - PASS: " << counter << "\n";
    if (pass_last_partitions.size())
      output_partitions.push_back(pass_last_partitions[0]);
    print_partitions(output_partitions);

    // Update variables for the next pass
    input_partitions = output_partitions;
    p_before_merge = p_after_merge;
    p_after_merge /= 2;
  }

  // Is there one partition left in pass_last_partitions (occurs when p_before_merge is odd at any pass)?
  if (pass_last_partitions.size()) {
    std::cout << "\n-----------------------------   PASS " << ++counter << "   -----------------------------" << std::endl;

    // Do the multithreaded merging operation of two remaining partitions (input_partitions[0] and output_partitions[0])
    threads.clear();
    output_partitions.resize(1);
    threads.push_back(std::thread(cpp_merge, std::ref(input_partitions[0]), std::ref(pass_last_partitions[0]), std::ref(output_partitions[0])));
    threads[0].join();

    std::cout << "\nPartitions after multithreaded merging - PASS: " << counter << "\n";
    print_partitions(output_partitions);
  }
}