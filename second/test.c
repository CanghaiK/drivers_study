#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>



int main(void)
{
    int fd;
    int counter = 0;

    fd = open("/dev/second",O_RDONLY);

    if(fd < 0){
        printf("failed to open device\n");
        return -1;
    }
    //read second
    while(1){
        read(fd,&counter,sizeof(int));
        printf("counter = %d\n",counter);
    }
    close(fd);

    return 0;
}
