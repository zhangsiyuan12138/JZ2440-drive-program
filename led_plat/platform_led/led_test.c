#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc ,char **argv)
{
   int val=1,fd;
   fd = open("/dev/led_dev",O_RDWR);
   
   if(fd <0)     printf("ooops...  can't open\n");
   if(argc != 2) printf("use like this: ./led_test on\n");
   if(strcmp("on", argv[1])==0) val = 1;
   if(strcmp("off", argv[1])==0) val = 0;
   
   write(fd,&val,4);
   return 0;
}












































