#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>

#include <sys/mman.h>
#include <sys/wait.h>

#define STACK_SIZE (1024 * 1024)
#define MAX_ARGS 32
#define min(a, b) (a < b ? a: b)

typedef struct jail_args {
    const char *command;
    char *argv[MAX_ARGS + 1];
} jail_args_t;

int jail(void *arg) {
    jail_args_t *jail_args = (jail_args_t*) arg;
    return execvp(jail_args->command, jail_args->argv);
}

int main(int argc, char *const *argv) {
    if (argc == 1) {
        printf("usage: %s [COMMAND] [ARG...] \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *stack = (char *) mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    char *stack_top = stack + STACK_SIZE;

    jail_args_t args = {0};
    args.command = argv[1];
    for (int i = 1; i < min(argc, MAX_ARGS); i++) {
       args.argv[i - 1] = argv[i];
    }

    int pid = clone(jail, stack_top, CLONE_NEWUTS | SIGCHLD, &args);
    if (pid == -1) {
        perror("clone failed");
        exit(EXIT_FAILURE);
    }

    if (waitpid(pid, NULL, 0) == -1)  {
        perror("waitpid failed");
        exit(EXIT_FAILURE);
    }
    
    return EXIT_SUCCESS;
}

