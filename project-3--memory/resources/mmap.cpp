#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

using namespace std;


int main(int argc, char** argv)
{
	/* Make sure the command line is correct */
	if (argc < 2)
	{
		cout << "FILE NAME missing\n";

		exit(1);
	}

	/* Open the specified file */
	int fd = open(argv[1], O_RDWR);


	if (fd < 0)
	{
		cout << "\n" << "input file cannot be opened" << "\n";
		exit(1);
	}

	struct stat stats;
	if (stat(argv[1], &stats) == 0)
		cout << endl << "file size " << stats.st_size;
	else
		cout << "Unable to get file properties.\n";

	/* Get the page size  */
	int pagesize = getpagesize();
	cout << endl << "page size is " << pagesize << "\n";

	/* map the file into memory */
	char* data = (char*)mmap(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


	/* Did the mapping succeed ? */
	if (!data)
	{
		cout << "\n" << "mapping did not succeed" << "\n";
		exit(1);
	}


	/* Print the whole file character-by-character */
	for (int fIndex = 0; fIndex < pagesize; ++fIndex)
	{

		cout << data[fIndex];
		
	}
	cout << endl;
	/* Write a string to the mapped region */
/*	memcpy(data, "Hello world, this is a test\n", sizeof("Hello world, this is a test"));*/

	/* Unmap the shared memory region */
	munmap(data, pagesize);

	/* Close the file */
	close(fd);

	return 0;
}
