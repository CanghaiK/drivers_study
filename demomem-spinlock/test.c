#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "demomem.h"

#define DEVICE_PATH                 "/dev/demomem"


int main(void)
{
    int fd;
    int ret;
    int n;
    int val;
    char buf[10] = "hello\n";

    printf("Open \n");
    fd = open(DEVICE_PATH,O_RDWR);
    if(fd<0){
        printf("failed to open\n");
        return -1;
    }
    
    printf("Sleep\n");
    sleep(10);

    printf("Close\n");
    close(fd);


    return 0;

}
    
