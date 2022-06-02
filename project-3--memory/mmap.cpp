#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

void print_usage() {
  std::cout << "Usage:" << std::endl << std::endl
	    << "    mmap <INPUT_FILE> <OUTPUT_FILE>" << std::endl << std::endl
	    << "where" << std::endl << std::endl
	    << "    <INPUT_FILE> is the input filename with some content in itself" << std::endl
	    << "    <OUTPUT_FILE> is the output filename (file will be created if it does not exit beforehand) " << std::endl
	    << std::endl
	    << "Example:" << std::endl
	    << "    $ ./mmap inFile.txt outFile.txt" << std::endl
	    << std::endl;
}

int main(int argc, char** argv)
{
	int src_fd, dest_fd, pagesize;
	size_t count;
	char* src_ptr, * backup_src_ptr, * dest_ptr, * backup_dest_ptr;
	struct stat stats;

	// Ensure the command line is correct
	if (argc != 3) {
		void print_usage();
		exit(1);
	}

	// Open input file
	src_fd = open(argv[1], O_RDWR);
	if (src_fd < 0) {
		std::cout << std::endl 
							<< "Unable to open input file" 
							<< std::endl;
		exit(1);
	}

	// Open or create output file
	dest_fd = open(argv[2], O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
	if (dest_fd < 0) {
			std::cout << std::endl
								<< "Unable to open output file" 
								<< std::endl;
			exit(1);
	}

	// Fetch the input file's size
	if (stat(argv[1], &stats) == 0)
		std::cout << std::endl 
							<< "Size of the input file: " << stats.st_size
							<< std::endl ;
	else
		std::cout << std::endl
							<<"Unable to get file properties"
							<< std::endl;

	// Map the input file into memory
	src_ptr = (char*) mmap(NULL, stats.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, src_fd, 0);
	if (*src_ptr < 0) {
		std::cout << std::endl 
							<< "Input-file memory mapping did not succeed"
							<< std::endl;
		exit(1);
	}

	// Save the src_ptr starting address for munmap later
	backup_src_ptr = src_ptr;

	// Resize output file to input file's size
	ftruncate(dest_fd, stats.st_size);

	// Map the output file into memory
	dest_ptr = (char*) mmap(NULL, stats.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, dest_fd, 0);
	if (*dest_ptr < 0) {
		std::cout << std::endl 
							<< "Output-file memory mapping did not succeed"
							<< std::endl;
		exit(1);
	}

	// Save the dest_ptr starting address for munmap later
	backup_dest_ptr = dest_ptr;

	// Fetch the page size
	pagesize = getpagesize();

	std::cout << std::endl
						<< "File copy being made in chunks of " << 100*pagesize << " bytes..."
						<< std::endl;

	// Copy bytes in chunks of 100*pagesize from one memory region to another
	const size_t COPY_SIZE = 100*pagesize;
	const size_t ITERATIONS = floor(stats.st_size / COPY_SIZE);
	count = 0;
	while (count < ITERATIONS) {
		memcpy(dest_ptr, src_ptr, COPY_SIZE);
		dest_ptr += COPY_SIZE;
		src_ptr += COPY_SIZE;
		count++;
	}

	// Copy remaining bytes if any
	const size_t REMAINDER_BYTES = (stats.st_size % COPY_SIZE);
	if (REMAINDER_BYTES > 0)
		memcpy(dest_ptr, src_ptr, REMAINDER_BYTES); 

	// Unmap the shared memory regions
	if (munmap(backup_src_ptr, stats.st_size) < 0) {
		std::cout << std::endl 
							<< "Input-file memory unmapping did not succeed"
							<< std::endl;
		exit(1);
	}
	if (munmap(backup_dest_ptr, stats.st_size) < 0) {
		std::cout << std::endl 
							<< "Output-file memory unmapping did not succeed"
							<< std::endl;
		exit(1);
	}

	// Close the input and output files
	close(src_fd);
	close(dest_fd);

	// Fetch the output file's size
	if (stat(argv[2], &stats) == 0)
		std::cout << std::endl 
							<< "Size of the output file: " << stats.st_size
							<< std::endl ;
	else
		std::cout << std::endl
							<<"Unable to get file properties"
							<< std::endl;

	return 0;
}