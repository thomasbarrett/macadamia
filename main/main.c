#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sched.h>

#include <sys/mman.h>
#include <sys/wait.h>

#define STACK_SIZE (1024 * 1024)

int jail(void *args) {
  printf("Hello !! ( child ) \n");
  return EXIT_SUCCESS;
}

int main(int argc, char const *argv[]) {
    char *stack = (char *) mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    printf("Hello, World! ( parent ) \n");

    int pid = clone(jail, stack_top, SIGCHLD, 0);
    if (pid == -1) {
        exit(EXIT_FAILURE);
    }

    if (waitpid(pid, NULL, 0) == -1)  {
        exit(EXIT_FAILURE);
    }
    
    printf("child has terminated\n");

    return EXIT_SUCCESS;
}
