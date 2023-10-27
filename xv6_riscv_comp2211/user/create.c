#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Simple string concatenation function
void concat(char *dest, const char *src) {
    while(*dest) {
        dest++;
    }
    while((*dest++ = *src++))
        ;
}

int main(int argc, char *argv[])
{
    int fd;

    if(argc < 2){
        fprintf(2, "Usage: create filename\n");
        exit(1);
    }

    // Make sure the 'test' directory exists
    if(mkdir("test") < 0) {
        // Directory might already exist or another error might occur
    }

    // Create the path to the file in the 'test' directory
    char filepath[128];
    strcpy(filepath, "test/");
    concat(filepath, argv[1]);

    // O_CREATE flag indicates to create the file if it does not exist
    // O_RDWR flag indicates that we want to open it for reading and writing
    fd = open(filepath, O_CREATE | O_RDWR);

    if(fd < 0) {
        fprintf(2, "create: error creating file %s\n", filepath);
        exit(1);
    }

    // Write "hello" to the file
    if(write(fd, "hello", 5) != 5) {
        fprintf(2, "create: write error to file %s\n", filepath);
        close(fd);
        exit(1);
    }

    // Close the file descriptor
    close(fd);

    exit(0);
}
