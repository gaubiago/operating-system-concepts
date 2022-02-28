/* Homework 1 */

// Author: Gaubert Santiago

#include <stdio.h> // e.g.: stderr pointer, (f)printf()
#include <stdlib.h> // e.g.: exit
#include <sys/types.h> // declare system data types: pid_t
#include <unistd.h> // provides access to the POSIX operating system API
                    // e.g.: getpid(), execl()...
#include <sys/wait.h> // e.g.: wait()

int main() {
    pid_t child = fork();
    if (child < 0) {
        fprintf(stderr, "Forked failed.\n");
        exit(EXIT_FAILURE);
    } else if (child == 0) {
        pid_t grandchild = fork();
        if (grandchild < 0) {
            fprintf(stderr, "Forked failed.\n");
            exit(EXIT_FAILURE);
        } else if (grandchild == 0) {
            printf("I'am a grandchild. My ID is %d.\n", getpid());
            fflush(stdout);
            printf("My parent is process %d.\n\n", getppid());
            fflush(stdout);
            printf("Grandchild turning into a Mozilla Firefox process.\n");
            fflush(stdout);
            // exec family of functions:
            // - Blow away the current process' virtual address space
            // - Begin executing the specified program in the current process
            execl("/usr/bin/firefox", "firefox", NULL); // By convention, argv[0] is just the file name of the executable, 
                                                        // normally it's set to the same as file.
        } else {
            printf("I'am a child. My ID is %d.\n", getpid());
            fflush(stdout);
            printf("My parent is process %d.\n\n", getppid());
            fflush(stdout);
        }
    } else {
        printf("I am the original process %d.\n\n", getpid());
        fflush(stdout);
    }
    wait(NULL);
    printf("\nProcess %d terminating.\n", getpid());
    return 0;
}