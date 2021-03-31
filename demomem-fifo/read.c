#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_PATH  "/dev/demomem"

int main(void)
{
    int fd;
    int n;
    char buf[20];
    
    fd = open(DEVICE_PATH,O_RDWR);

    if(fd < 0){
        printf("failed to open %s\n",DEVICE_PATH);
        return -1;
    }

    //read buffer
    while(1){
        memset(buf,0x00,20);
        n = read(fd,buf,20);
        if(n<0){
            printf("failed to read,n = %d\n",n);
            break;
        }
        printf("read %d butes: %s\n",n,buf);
    }

    close(fd);
    return n ;
}
