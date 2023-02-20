#define _GNU_SOURCE

#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <stdnoreturn.h>

#include <sys/mman.h>
#include <sys/wait.h>
 #include <sys/mount.h>
 
#define STACK_SIZE (1024 * 1024)
#define MAX_ARGS 32
#define min(a, b) (a < b ? a: b)


char* alloc_stack(size_t size) {
    char *stack = (char *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    return stack;
}

typedef struct jail_args {
    const char *command;
    char *argv[MAX_ARGS + 1];
} jail_args_t;

noreturn int command(void *arg) {
    jail_args_t *jail_args = (jail_args_t*) arg;
    execvp(jail_args->command, jail_args->argv);
    perror("execvp failed");
    exit(EXIT_FAILURE);
}

int jail(void *arg) {
    if (chroot("/var/images/ubuntu") == -1) {
        perror("chroot failed");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") == -1) {
        perror("chdir failed");
        exit(EXIT_FAILURE);
    }

    if (mount("proc", "/proc", "proc", 0, 0) == -1) {
        perror("mount failed");
        exit(EXIT_FAILURE);
    }

    jail_args_t *jail_args = (jail_args_t*) arg;

    char *stack = alloc_stack(STACK_SIZE);
    char *stack_top = stack + STACK_SIZE;

    int pid = clone(command, stack_top, SIGCHLD, arg);
    if (pid == -1) {
        perror("clone failed");
        exit(EXIT_FAILURE);
    }

    if (waitpid(pid, NULL, 0) == -1)  {
        perror("waitpid failed");
        exit(EXIT_FAILURE);
    }

    if (umount("/proc") == -1) {
        perror("umount failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}


int main(int argc, char *const *argv) {
    if (argc == 1) {
        printf("usage: %s [COMMAND] [ARG...] \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *stack = alloc_stack(STACK_SIZE);
    char *stack_top = stack + STACK_SIZE;

    jail_args_t args = {0};
    args.command = argv[1];
    for (int i = 1; i < min(argc, MAX_ARGS); i++) {
       args.argv[i - 1] = argv[i];
    }

    int pid = clone(jail, stack_top, CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, &args);
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
