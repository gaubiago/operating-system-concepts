#include <iostream>
#include <cctype>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <thread>

#define LOWER_BOUND 0
#define UPPER_BOUND 101

void cppSort(std::vector<int>& partition) {
  std::sort(partition.begin(), partition.end());
}

std::string dumpPartition(std::vector<int>& partition) {
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

void displayMergeMsg(std::vector<int>& part_a, std::vector<int>& part_b, std::vector<int>& sortedNumPartition) {
  std::cout << " * Merged " << dumpPartition(part_a) << " and " << dumpPartition(part_b) << std::endl;
  std::cout << "     Result: " << dumpPartition(sortedNumPartition) << std::endl;
}

void cppMerge(std::vector<int>& part_a, std::vector<int>& part_b, std::vector<int>& sortedNumPartition) {
  sortedNumPartition.resize(part_a.size() + part_b.size());
  std::merge(part_a.begin(), part_a.end(), part_b.begin(), part_b.end(), sortedNumPartition.begin());
  displayMergeMsg(part_a, part_b, sortedNumPartition);
  
}

int main(int argc, char* argv[]) {
  // n: Number of elements to be generated
  const std::string n_str(argv[1]);
  for (int i = 0; i < n_str.size(); i++) {
    if (!isdigit(n_str[i])) {
      std::cout << "This is how you must execute this program: n xxx" << std::endl;
      return 1;
    }
  }
  const size_t n = std::stoi(n_str);

  // p: Number of partitions to be created
  const std::string p_str(argv[2]);
  for (int i = 0; i < p_str.size(); i++) {
    if (!isdigit(p_str[i])) {
      std::cout << "This is how you must execute this program: p xxx" << std::endl;
      return 1;
    }
  }
  const size_t p = std::stoi(p_str);

  // Stop execution if #partitions > #elements
  if (p > n) {
    std::cout << "The first argument passed in needs to be bigger than the second one." << std::endl;
    return 1;
  }

  // Generate list of random integers
  std::vector<int> randNumList;
  int tmp;
  for (int i = 0; i < n; i++) {
    tmp = rand()%UPPER_BOUND + LOWER_BOUND;
    std::cout << tmp << " ";
    randNumList.push_back(tmp);
  }

  // Break down list into partitions
  std::vector< std::vector<int> > randNumPartitions(p);
  size_t quotient = n/p; // Partition size
  size_t remainder = n%p; // Extra elements to be distributed into a required # of partitions
  size_t offset = 0; // Number of extra elements added individually to different partitions
  for (int i = 0, j = 0; i < n; i++) {
    // Whenever a partition reaches its indended, even size and it's not the first iteration
    if ((i-offset)%(quotient) == 0 && i != 0) {
      // If there is any remainder, add the next element in the list to the current partition 
      if (remainder) {
        randNumPartitions[j].push_back(randNumList[i]);
        remainder--;
        i++;
        offset++;
      }
      j++;
    }
    randNumPartitions[j].push_back(randNumList[i]);
  }

  // Print partitions
  std::cout << std::endl;
  for (int i = 0; i < p; i++) {
    std::cout << "Part. " << i << " (size=" << randNumPartitions[i].size() << "): ";
    for (int j = 0; j < randNumPartitions[i].size(); j++) {
      std::cout << randNumPartitions[i][j] << " ";
    }
    std::cout << std::endl;
  }

  // Sort partitions
  std::vector<std::thread> threads;
  for (int i = 0; i < p; i++) {
    // std::sort(randNumPartitions[i].begin(), randNumPartitions[i].end());
    threads.push_back(std::thread(cppSort, std::ref(randNumPartitions[i])));
  }
  for (auto& th : threads) th.join(); // Synchronizing all threads

  // Print partitions
  std::cout << std::endl;
  for (int i = 0; i < p; i++) {
    std::cout << "Part. " << i << " (size=" << randNumPartitions[i].size() << "): ";
    for (int j = 0; j < randNumPartitions[i].size(); j++) {
      std::cout << randNumPartitions[i][j] << " ";
    }
    std::cout << std::endl;
  }

  // Merge partitions (1st round)
  threads.clear();
  std::vector< std::vector<int> > sortedNumPartitions(p/2);
  for (int i = 0, j = 0; i < p-1; i += 2, j++) {
    threads.push_back(std::thread(cppMerge, std::ref(randNumPartitions[i]), std::ref(randNumPartitions[i+1]), std::ref(sortedNumPartitions[j])));
  }
  for (auto& th : threads) th.join(); // Synchronizing all threads

  // Print partitions
  std::cout << std::endl;
  for (int i = 0; i < sortedNumPartitions.size(); i++) {
    std::cout << "Part. " << i << " (size=" << sortedNumPartitions[i].size() << "): ";
    for (int j = 0; j < sortedNumPartitions[i].size(); j++) {
      std::cout << sortedNumPartitions[i][j] << " ";
    }
    std::cout << std::endl;
  }

  // Merge partitions (2nd round)

  return 0;
}