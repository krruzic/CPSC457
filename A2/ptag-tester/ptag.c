#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <string.h>
//#include <sys/status.h>


#define __NR_ptag 337
int main(int argc, char *argv[]) {
    struct stat name;
    char *tst;
    if (argc != 4) {    
        printf("Usage: ptag <PID> <flag> <tag>\n");
        return 0;
    }
    pid_t PID = atoi(argv[1]);

    if(kill(PID, 0) != 0) {
        printf("Error: PID does not exist or does not belong to you!\n");
        return -1;
    }
    int opt = getopt(argc, argv, "ar");
    if (opt == 63)
        return 0;
    long x = syscall(__NR_ptag, PID, argv[3], opt);
    return 0;
}