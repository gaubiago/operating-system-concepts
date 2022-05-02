#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <pthread.h>

#define LOWER_BOUND 1
#define USAGE_ERROR 1

// Struct type to hold arguments for sorting operations
typedef struct {
	std::pair<size_t, size_t> idx_pair;
	size_t i;
} sorting_op_args_t;

// Struct type to hold arguments for merging operations
typedef struct {
	std::vector<int> in_part_a;
	std::vector<int> in_part_b;
	std::vector<int> out_part;
	size_t i;
} merging_op_args_t;

// rand_int_list: generated list of random integers (original list)
std::vector<int> rand_int_list;

// pairs: tuple to hold both starting and ending indices of each partition
std::vector< std::pair<size_t, size_t> > pairs;

// Temporary vectors for merging operations 
// input_partitions: partitions before merging operations
// output_partitions: partitions after merting operations
// pass_last_partitions: last partition of the list of partitions at each pass (when p_before_merge is odd)
std::vector< std::vector<int> > input_partitions, 
																output_partitions, 
																pass_last_partitions;

// last_partitions_merge_output: result of the merging of two pass_last_partitions
std::vector<int> last_partitions_merge_output;

void print_usage();
void validate_argv(int &argc, char *argv[]);
std::string dump_partition(std::vector<int> &partition); // Overloaded
void generate_list(const size_t &n, const size_t &upper_bound);
void print_list();
void set_partition_delimiters(const size_t &n, const size_t &p);
std::string dump_partition(std::pair<size_t, size_t> &pair); // Overloaded 
void print_partitions(); // Overloaded 
void display_sort_msg(std::pair<size_t, size_t>& idx_pair, size_t& part_id);
void* cpp_sort(void *args_ptr);
void sort_partitions_multithreaded(std::vector<pthread_t>& threads, const size_t& p);
void print_partitions(std::vector< std::vector<int> >& partitions); // Overloaded
void display_merge_msg(std::vector<int>& in_part_a, std::vector<int>& in_part_b, std::vector<int>& out_part, size_t& part_id);
void* cpp_merge(void* args_ptr);
void merge_partitions_multithreaded(std::vector<pthread_t>& threads, const size_t& p);

int main(int argc, char *argv[]) {
	// Validate arguments that are passed in upon running executable
	validate_argv(argc, argv);

	// n: number of elements to be generated
	const size_t n = std::stoi(argv[1]);
	// upper_bound: max. possible value of an integer element of the original list
	const size_t upper_bound = std::stoi(argv[2]);
	// p: number of partitions to be created
	const size_t p = std::stoi(argv[3]);

	// Generate list of random integers
	generate_list(n, upper_bound);
	std::cout << "Generated list of random integers:\n\n  ";
	print_list();

	// No need for partitioning, sorting, and merging if list has only 1 element
	if (rand_int_list.size() == 1) {
		std::cout << std::endl
							<< "List is already sorted." << std::endl;
		return 0;
	}

	// No need for partitioning and merging
	if (p == 1) {
		std::cout << "\nNo need for partitioning and merging. Result of sorting:\n\n  ";
		std::sort(rand_int_list.begin(), rand_int_list.end());
		std::cout << dump_partition(rand_int_list) << std::endl;
		return 0;
	}

	// Break down list into partitions (within original random int list)
	set_partition_delimiters(n, p);
	std::cout << "\nList breakdown into partitions:\n";
	print_partitions();

	// Sort partitions in a multithreaded and sorted way
	std::cout << "\nPartitions after multithreaded sorting:\n\n";
	std::vector<pthread_t> threads(p);
	sort_partitions_multithreaded(threads, p);

	// Merge partitions in a multithreaded and sorted way, building back one single list
  std::cout << "\nMultithreaded merging of partitions:\n";
	threads.clear();
  merge_partitions_multithreaded(threads, p);

	std::cout << "\nResult of list partitioning, followed by partition multithreaded sorting, followed by partition multithreaded merging:\n\n  ";
	rand_int_list = output_partitions[0]; // This assignment is part of the requirements of the project
	std::cout << dump_partition(rand_int_list) << std::endl;

	return 0;
}

void print_usage() {
	std::cout << "Usage:" << std::endl
						<< std::endl
						<< "    multi_threaded_merge_sort <N> <MAX_VALUE> <P>" << std::endl
						<< std::endl
						<< "where" << std::endl
						<< std::endl
						<< "    <N> is a positive integer representing the size of your list of elements" << std::endl
						<< "    <MAX_VALUE> is a positive integer representing the possible max. value of the list elements " << std::endl
						<< "    <P> is a positive integer (greater than zero) representing the intended number of partitions to break down the list into" << std::endl
						<< std::endl
						<< "Example:" << std::endl
						<< "    $ ./multi_threaded_merge_sort 25 100 5" << std::endl
						<< std::endl;
}

void validate_argv(int &argc, char *argv[]) {
	if (argc != 4) {
		std::cout << "Invalid number of arguments." << std::endl;
		print_usage();
		exit(USAGE_ERROR);
	}

	// n: Number of elements to be generated
	const std::string n_str(argv[1]);
	for (int i = 0; i < n_str.size(); i++) {
		if (!isdigit(n_str[i]))
		{
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
	bool terminate = !std::stoi(p_str) ? true : false;
	for (int i = 0; i < p_str.size(); i++) {
		if (!isdigit(p_str[i]) || terminate) {
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

// Overloaded
std::string dump_partition(std::vector<int> &partition) {
	// Build partition string
	std::string str("");
	str += '[';
	for (int i = 0; i < partition.size(); i++) {
		str += std::to_string(partition[i]);
		if (i != partition.size() - 1)
			str += ' ';
	}
	str += ']';
	return str;
}

void generate_list(const size_t &n, const size_t &upper_bound) {
	// Initialize random seed
	srand(time(NULL));

	for (int i = 0, tmp; i < n; i++) {
		tmp = rand() % (!upper_bound ? 1 : upper_bound + 1) + LOWER_BOUND;
		rand_int_list.push_back(tmp);
	}
}

void print_list() {
	std::cout << dump_partition(rand_int_list) << std::endl
						<< std::flush;
}

void set_partition_delimiters(const size_t &n, const size_t &p) {
	// pair: holds starting and ending indices of a given partition
	std::pair<size_t, size_t> pair;
	for (size_t i = 0; i < p; i++) {
		pair.first = i * floor(n / p);
		if (i < p - 1)
			pair.second = (i + 1) * floor(n / p);
		else
			pair.second = n;
		pairs.push_back(pair);
	}
}

// Overloaded
std::string dump_partition(std::pair<size_t, size_t> &pair) {
	// Build partition string
	std::string str("");
	str += '[';
	for (size_t i = pair.first; i < pair.second; i++) {
		str += std::to_string(rand_int_list[i]);
		if (i != pair.second - 1)
			str += ' ';
	}
	str += ']';
	return str;
}

// Overloaded
void print_partitions() {
	std::cout << std::endl;
	for (int i = 0; i < pairs.size(); i++)
		std::cout << "  Part. " << i << ": " << dump_partition(pairs[i])
							<< " (size=" << pairs[i].second - pairs[i].first << ")"
							<< std::endl
							<< std::flush;
}

void display_sort_msg(std::pair<size_t, size_t>& idx_pair, size_t& part_id) {
  std::string str("  Part. " + std::to_string(part_id) + ": " + dump_partition(idx_pair) 
														 + " (size=" + std::to_string(idx_pair.second - idx_pair.first ) + ")\n");
  std::cout << str;
}

void *cpp_sort(void *args_ptr) {
	sorting_op_args_t* args = (sorting_op_args_t*) args_ptr;
	std::sort(rand_int_list.begin() + args->idx_pair.first,
						rand_int_list.begin() + args->idx_pair.second);
	display_sort_msg(args->idx_pair, args->i);
	pthread_exit(0);
}

void sort_partitions_multithreaded(std::vector<pthread_t>& threads, const size_t& p) {
	std::vector<sorting_op_args_t> sorting_args;

	// Populate sorting struct vector in preparation for sorting
	for (size_t i = 0; i < p; i++)
		sorting_args.push_back((sorting_op_args_t) {
														.idx_pair = pairs[i],
														.i = i
													});

	// Sort elements in each partition of the original random int list (same resource) in a multithreaded way
	for (size_t i = 0; i < p; i++)	
		pthread_create(&threads[i], NULL, &cpp_sort, (void *) &sorting_args[i]);	

	// Synchronize all threads
	for (size_t i = 0; i < p; i++)
		pthread_join(threads[i], NULL);
}

// Overloaded
void print_partitions(std::vector< std::vector<int> >& partitions) {
  std::cout << std::endl;
  for (int i = 0; i < partitions.size(); i++) {
    std::cout << "  Part. " << i << ": " << dump_partition(partitions[i]) << " (size=" << partitions[i].size() << ")"
              << std::endl << std::flush;
  }
}

void display_merge_msg(std::vector<int>& in_part_a, std::vector<int>& in_part_b, std::vector<int>& out_part, size_t& part_id) {
  std::string str("\n * Merged \n    Part. " + std::to_string(2*part_id) + ":\t " + dump_partition(in_part_a) + " (size=" + std::to_string(in_part_a.size()) + ")" + " and " 
                            + "\n    Part. " + std::to_string(2*part_id + 1) + ":\t " + dump_partition(in_part_b) + " (size=" + std::to_string(in_part_b.size()) + ")" 
                + "\n - Result: \n    New Part. " + std::to_string(part_id) + ": " + dump_partition(out_part) + " (size=" + std::to_string(out_part.size()) + ")\n");
  std::cout << str;
}

void* cpp_merge(void* args_ptr) {
	merging_op_args_t* args = (merging_op_args_t*) args_ptr;

	// Output partition has the size of the sum of the two input partitions
	args->out_part.resize(args->in_part_a.size() + args->in_part_b.size());

  std::merge(args->in_part_a.begin(), args->in_part_a.end(), 
						 args->in_part_b.begin(), args->in_part_b.end(), 
						 args->out_part.begin());
  display_merge_msg(args->in_part_a, args->in_part_b, args->out_part, args->i);
  pthread_exit(0);
}

void merge_partitions_multithreaded(std::vector<pthread_t>& threads, const size_t& p) {
  // p_before_merge: number of partitions before merging operations
  size_t p_before_merge = p;
  // p_after_merge: number of partitions after merging operations
  size_t p_after_merge = p/2;
  // counter: pass counter
  size_t counter = 0;

	// Prepare for multithreaded merging by creating temporary vectors 
	for (size_t i = 0; i < p; i++) 
		input_partitions.push_back(std::vector<int> (rand_int_list.begin() + pairs[i].first,
																								 rand_int_list.begin() + pairs[i].second));

	// pass_last_partition_m_args: holds the arguments for the last partition (Part. <p_before_merge-1>)
	//														 at a given pass while merging partitions
	merging_op_args_t pass_last_partition_m_args;

  while (p_after_merge > 0) {
    std::cout << "\n-----------------------------   PASS " << ++counter << "   -----------------------------" << std::endl;

    threads.clear();
    output_partitions.clear();

		std::vector<merging_op_args_t> merging_args;

		// Populate merting struct vector in preparation for merging
		for (size_t i = 0; i < p_after_merge; i++) 
			merging_args.push_back((merging_op_args_t) {
															.in_part_a = input_partitions[2*i],
															.in_part_b = input_partitions[2*i + 1],
															.i = i
														});

    // Do multithreaded merging operations
		threads.resize(p_after_merge);
		for (size_t i = 0; i < p_after_merge; i++)
			pthread_create(&threads[i], NULL, &cpp_merge, (void *) &merging_args[i]);	

    // Is the number of partitions before the merging operations odd?
    if (p_before_merge%2) {
      // Append last partition of the list of partitions
      pass_last_partitions.push_back(input_partitions[p_before_merge-1]);
			if (pass_last_partitions.size() == 1) {
				pass_last_partition_m_args.in_part_a = input_partitions[p_before_merge-1];
			}
      // Carry out multithreaded merging operation if the vector's size is 2
      if (pass_last_partitions.size() == 2) {
				pass_last_partition_m_args.in_part_b = input_partitions[p_before_merge-1];
				pass_last_partition_m_args.i = p_after_merge;
				threads.push_back(0);
				pthread_create(&threads[threads.size() - 1], NULL, &cpp_merge, (void *) &pass_last_partition_m_args);
      }
    }

		// Synchronize all threads
		for (size_t i = 0; i < threads.size(); i++) 
			pthread_join(threads[i], NULL);

    // Is the number of partitions before the merging operations odd, and is pass_last_partitions' size 2?
    if (p_before_merge%2 && pass_last_partitions.size() == 2) {
      // Clear pass_last_partitions
      pass_last_partitions.clear();
      // Print out the result of the merging operation
      dump_partition(pass_last_partition_m_args.out_part);
      // Append the resulting partition to pass_last_partitions
      pass_last_partitions.push_back(pass_last_partition_m_args.out_part);
    }

		std::cout << "\nPartitions after multithreaded merging - PASS: " << counter << "\n";
		for (size_t i = 0; i < p_after_merge; i++) 
			output_partitions.push_back(merging_args[i].out_part);
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
    threads.resize(1);
    output_partitions.clear();
		merging_op_args_t final_merging_args = (merging_op_args_t) {
																							.in_part_a = input_partitions[0],
																							.in_part_b = pass_last_partitions[0],
																							.out_part = output_partitions[0],
																							.i = 0
																					 };
    pthread_create(&threads[0], NULL, &cpp_merge, (void *) &final_merging_args);
		pthread_join(threads[0], NULL);
		output_partitions.push_back(final_merging_args.out_part);

    std::cout << "\nPartitions after multithreaded merging - PASS: " << counter << "\n";
    print_partitions(output_partitions);
  }
	std::cout << "\n----------------------------------------------------------------------" << std::endl;
}