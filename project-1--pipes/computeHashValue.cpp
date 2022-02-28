/* 
Author: Gaubert Santiago
Email: gaubert.santiago@csu.fullerton.edu
*/

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

using namespace std;

/* The pipe for parent-to-child communications */
int parentToChildPipe[2];

/* The pipe for the child-to-parent communication */
int childToParentPipe[2];

/* The read end of the pipe */
#define READ_END 0

/* The write end of the pipe */
#define WRITE_END 1

/* The maximum size of the array of hash programs */
#define HASH_PROG_ARRAY_SIZE 6

/* The maximum length of the hash value */
#define HASH_VALUE_LENGTH 1000

/* The maximum length of the file name */
#define MAX_FILE_NAME_LENGTH 1000

/* The array of names of hash programs */
const string hashProgs[] = {"md5sum", "sha1sum", "sha224sum", "sha256sum", "sha384sum", "sha512sum"};

string fileName;

/**
 * The function called by a child
 * @param hashProgName - the name of the hash program
 */
void computeHash(const string& hashProgName) {
	/* Child */

	/* ----------------------
	   Child read from parent 
	   ---------------------- */

	/* The received file name string */
	char fileNameRecv[MAX_FILE_NAME_LENGTH];
	/* Fill the buffer with 0's */
	memset(fileNameRecv, (char)NULL, MAX_FILE_NAME_LENGTH);

	/* Close the unused end */
	if (close(parentToChildPipe[WRITE_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}
	/* Read the file name sent by the parent */
	if (read(parentToChildPipe[READ_END], fileNameRecv, sizeof(fileNameRecv)) < 0) {
		perror("Child failed to read from pipe.");
		exit(-1);
	}
	/* Close the unused end */
	if (close(parentToChildPipe[READ_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}
	
	/* ------------------------------
	   Child-grandchild communication
	   ------------------------------ */

	/* The hash value buffer */
	char hashValue[HASH_VALUE_LENGTH];
	/* Reset the value buffer */
	memset(hashValue, (char)NULL, HASH_VALUE_LENGTH);

	/* Create command line, e.g. sha512sum <filename>. */
	string cmdLine(hashProgName);
	cmdLine += " ";
	cmdLine += fileNameRecv;	
	
    /* Open the pipe to the program (specified in cmdLine) 
	   using popen() and save the output into hashValue. 
	   See popen.cpp for examples using popen. */
	FILE* streamPointer = popen(cmdLine.c_str(), "r");
	if (!streamPointer) {
		perror("Failed to create stream.");
		exit(-1);
	}
	if (fread(hashValue, sizeof(char), sizeof(char) * HASH_VALUE_LENGTH, streamPointer) < 0) {
		perror("Child failed to read message from the grandchild.");
		exit(-1);
	}
	if (pclose(streamPointer) < 0) {
		perror("Failed to close stream.");
		exit(-1);
	}

	/* ---------------------
	   Child write to parent 
	   --------------------- */

	/* Close the unused end */
	if (close(childToParentPipe[READ_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}	
	/* Send the hash value to the parent */
	if (write(childToParentPipe[WRITE_END], &hashValue, sizeof(hashValue)) < 0) {
		perror("Child failed to write to pipe.");
		exit(-1);
	}
	/* Close the unused end */
	if (close(childToParentPipe[WRITE_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}

	/* The child terminates */
	exit(0);
}

void parentFunc(const string& hashProgName) {
	/* Parent */

	/* ---------------------
	   Parent write to child 
	   --------------------- */  

	/* Close the unused end */
	if (close(parentToChildPipe[READ_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}
	/* Send the file name to the child */
	if (write(parentToChildPipe[WRITE_END], fileName.c_str(), sizeof(fileName)) < 0) {
		perror("Parent failed to write to pipe.");
		exit(-1);
	}
	/* Close the unused end */
	if (close(parentToChildPipe[WRITE_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}

	/* ----------------------
	   Parent read from child 
	   ---------------------- */

	/* The buffer to hold the string received from the child */
	char hashValue[HASH_VALUE_LENGTH];
	/* Reset the hash buffer */
	memset(hashValue, (char)NULL, HASH_VALUE_LENGTH);

	/* Close the unused end */
	if (close(childToParentPipe[WRITE_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}
	/* Read the hash value sent by the child */
	if (read(childToParentPipe[READ_END], hashValue, sizeof(hashValue)) < 0) {
		perror("Parent failed to read from pipe.");
		exit(-1);
	}
	/* Close the unused end */
	if (close(childToParentPipe[READ_END]) < 0) {
		perror("Unable to close pipe end.");
		exit(-1);
	}

	/* Print the hash value */
	fprintf(stdout, "%s hash value: %s\n", hashProgName.c_str(), hashValue);
	fflush(stdout);
}

int main(int argc, char** argv) {
	/* Check for errors */
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]); 
		exit(-1);
	}	
	
	/* Save the name of the file */
	fileName = argv[1];
	/* The process id */
	pid_t pid;
	
	/* Run a program for each type of hash algorithm */	
	for (int hashAlgNum = 0; hashAlgNum < HASH_PROG_ARRAY_SIZE; ++hashAlgNum) {
		/* Create two pipes */
		if (pipe(parentToChildPipe) < 0) {
			perror("Failed to create pipe.");
			exit(-1);
		}
		if (pipe(childToParentPipe) < 0) {
			perror("Failed to create pipe.");
			exit(-1);
		}

		/* Fork a child process and save the id */
		if ((pid = fork()) < 0) {
			perror("Failed to fork process.");
			exit(-1);
		} else if (pid == 0) { 
			/* Child */
			/* Receive file name, compute the hash value & send it back */
			computeHash(hashProgs[hashAlgNum]);
		} else {
			/* Parent */
			/* Send file name; receive hash value */
			parentFunc(hashProgs[hashAlgNum]);

			/* Wait for the child to terminate */
			if (wait(NULL) < 0) {
				perror("Error occurred while waiting for a child to terminate.");
				exit(-1);
			}
		}
	}

	return 0;
}
