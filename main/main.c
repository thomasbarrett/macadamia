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

typedef struct child_args {
    char *argv[MAX_ARGS + 1];
} child_args_t;

int child(void *arg) {

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL)) {
        perror("mount failed");
        return -1;
    }

    if (chroot("/var/images/ubuntu") == -1) {
        perror("chroot failed");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") == -1) {
        perror("chdir failed");
        exit(EXIT_FAILURE);
    }

    if (mount("proc", "/proc", "proc", 0, NULL)) {
        perror("mount proc failed\n");
        return errno;
    }

    if (mount("sys", "/sys", "sysfs", 0, NULL)) {
        perror("mount sys failed\n");
        return errno;
    }

    if (mount("dev", "/dev", "devtmpfs", 0, NULL)) {
        perror("mount dev failed\n");
        return errno;
    }

    child_args_t *child_args = (child_args_t*) arg;
    execvpe(child_args->argv[0], child_args->argv, NULL);
    perror("execvp failed");
    exit(EXIT_FAILURE);
}

int main(int argc, char *const *argv) {
    if (argc == 1) {
        printf("usage: %s [COMMAND] [ARG...] \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *stack = alloc_stack(STACK_SIZE);
    char *stack_top = stack + STACK_SIZE;

    child_args_t args = {0};
    for (int i = 1; i < min(argc, MAX_ARGS); i++) {
       args.argv[i - 1] = argv[i];
    }

    int pid = clone(child, stack_top, CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, &args);
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
