#include <unistd.h>
#include <sys/mman.h>

#define STACK_SIZE (1024 * 1024)

int main(int argc, char const *argv[]) {
    char *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
           if (stack == MAP_FAILED)
               errExit("mmap");
    /* code */
    return 0;
}
