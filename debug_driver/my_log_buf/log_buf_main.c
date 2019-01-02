#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc,char **argv)
{
   int fd=0;
   fd = open("/dev/log_buf",O_RDWR);
   if(fd<0) printf("can't open /dev/log_buf!\n");
   return 0;
}




















