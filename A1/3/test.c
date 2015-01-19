#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int fd = open("/dev/crcdev", O_RDWR);
    write(fd, argv[1], strlen(argv[1])); 
    return 0;
}
