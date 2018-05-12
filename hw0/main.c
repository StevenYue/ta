#include <stdio.h>
#include <sys/resource.h>

int main() {
    struct rlimit lim;
    getrlimit(RLIMIT_STACK, &lim);
    printf("stack size: %ld\n", lim.rlim_cur);    //RLIMIT_STACK
    getrlimit(RLIMIT_NPROC, &lim);
    printf("process limit: %ld\n", lim.rlim_cur); //RLIMIT_NPROC
    getrlimit(RLIMIT_NOFILE, &lim);
    printf("max file descriptors: %ld\n", lim.rlim_cur);  //RLIMIT_NOFILE
    return 0;
}
