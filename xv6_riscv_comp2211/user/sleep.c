#include "kernel/types.h"
#include "user/user.h"
int main(int argc, char *argv[]) {

    if (argc != 2){
        printf("Wrong number of CLI!");
        return 1;
    }
    
    // both work
    write(1, argv[1], strlen(argv[1]));
    printf("\nargv is %s\n", argv[1]);

    // convert argv to int
    int sleep_count = atoi(argv[1]);
    printf("sleepcount %d\n", sleep_count);

    sleep(sleep_count);
    printf("sleepcount %d\n", sleep_count);

    exit(0);
    };