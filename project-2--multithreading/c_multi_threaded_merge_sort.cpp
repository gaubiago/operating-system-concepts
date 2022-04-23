#include <iostream>
#include <cctype>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <atomic>
#include <pthread.h>

#define LOWER_BOUND 0
#define USAGE_ERROR 1

std::vector<int> rand_int_list;
std::vector<std::pair<size_t, size_t>> pairs;

void print_usage();
void validate_argv(int &argc, char *argv[]);
std::string dump_partition(std::vector<int> &partition);
void generate_list(const size_t &n, const size_t &upper_bound);
void print_list(std::vector<int> &list);
void set_partition_delimiters(const size_t &n, const size_t &p);
std::string dump_partition(std::vector<int> &partition, std::pair<size_t, size_t> &pair);
void print_partitions();
void *cpp_sort(void *args_ptr);

int main(int argc, char *argv[]) {
	// Validate arguments that are passed in upon running executable
	validate_argv(argc, argv);

	// n: Number of elements to be generated
	const size_t n = std::stoi(argv[1]);
	// upper_bound: Max. possible value of an integer element
	const size_t upper_bound = std::stoi(argv[2]);
	// p: Number of partitions to be created
	const size_t p = std::stoi(argv[3]);

	// Generate list of random integers
	generate_list(n, upper_bound);
	print_list(rand_int_list);

	// No need for partitioning, sorting, and merging if list has only 1 element
	if (rand_int_list.size() == 1) {
		std::cout << std::endl
							<< "List is already sorted." << std::endl;
		return 0;
	}

	// Break down list into partitions (within original random int list)
	set_partition_delimiters(n, p);
	std::cout << "\nList breakdown into partitions:\n";
	print_partitions();

	// Sort partitions
	pthread_t threads[p];
	for (size_t i = 0; i < p; i++) {
		pthread_create(&threads[i], NULL, &cpp_sort, (void *) &pairs[i]);	
	}
	// Synchronize threads
	for (size_t i = 0; i < p; i++) {
		pthread_join(threads[i], NULL);
	}

	std::cout << "\nPartitions after multithreaded sorting:\n";
	print_partitions();

	return 0;
}

void print_usage() {
	std::cout << "Usage:" << std::endl
						<< std::endl
						<< "    multi_threaded_merge_sort <N> <MAX_VALUE> <P>" << std::endl
						<< std::endl
						<< "where" << std::endl
						<< std::endl
						<< "    <N> is an positive integer representing the size of your list of elements" << std::endl
						<< "    <MAX_VALUE> is an positive integer representing the possible max. value of the list elements " << std::endl
						<< "    <P> is an positive integer representing the intended number of partitions to break down the list into" << std::endl
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

void print_list(std::vector<int> &list) {
	std::cout << "Generated list of random integers:"
						<< "\n\n  "
						<< dump_partition(list) << std::endl
						<< std::flush;
}

void set_partition_delimiters(const size_t &n, const size_t &p) {
	// pair: holds start and end indices of a given partition
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

std::string dump_partition(std::vector<int> &partition, std::pair<size_t, size_t> &pair) {
	// Build partition string
	std::string str("");
	str += '[';
	for (size_t i = pair.first; i < pair.second; i++) {
		str += std::to_string(partition[i]);
		if (i != pair.second - 1)
			str += ' ';
	}
	str += ']';
	return str;
}

void print_partitions() {
	std::cout << std::endl;
	for (int i = 0; i < pairs.size(); i++) {
		std::cout << "  Part. " << i << ": " << dump_partition(rand_int_list, pairs[i])
							<< " (size=" << pairs[i].second - pairs[i].first << ")"
							<< std::endl
							<< std::flush;
	}
}

void *cpp_sort(void *args_ptr) {
	std::pair<size_t, size_t>* pair = static_cast<std::pair<size_t, size_t>*> (args_ptr);
	std::cout << ("(" + std::to_string(pair->first) + ", " + std::to_string(pair->second) + ") ");
	std::sort(rand_int_list.begin() + pair->first,
						rand_int_list.begin() + pair->second);
	pthread_exit(0);
}
