#include <stdio.h>
#include <sys/select.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define DEVICE_PATH  "/dev/demomem"

int main(void)
{
    int ret;
    fd_set rfds,wfds;

    int fd;
    int n;
    char buf[20];
    struct timeval time;
    
    fd = open(DEVICE_PATH,O_RDWR);

    if(fd < 0){
        printf("failed to open %s\n",DEVICE_PATH);
        return -1;
    }

    //read buffer
    while(1){
        //clear read and write fd_set
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(fd,&rfds);
        //FD_SET(fd,&wfds);

        time.tv_sec = 2;
        time.tv_usec = 0;

        ret = select(fd + 1,&rfds,&wfds,NULL,&time);
    
        if(ret == 0) {
            printf("timeout\n");
        }else if (ret > 0){
            if(FD_ISSET(fd,&rfds)){
                //read somrthing
                printf("read somrthing\n");
                memset(buf,0x00,20);
                n = read(fd,buf,20);
                printf("read %d bytes: %s\n",n ,buf);
            }
            /*
            if(FD_ISSET(fd,&wfds)){
                //write
                printf("write somrthing\n");
            }
            */
        }
    }

    close(fd);
    return n ;
}
