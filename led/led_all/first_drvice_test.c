#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc, char **argv)
{
   int fd;
   int val=1;
   fd = open("/dev/xyz",O_RDWR);
   if(fd<0)
   	{
   printf("can't open!\n");
   	}
   if(argc != 2)
   	{
       printf("use like this: %s <on|off>\n",argv[0]);
    }
   if(strcmp(argv[1],"on")==0)
   	{
        val=1;
    }
   if(strcmp(argv[1],"off")==0)
   	{
        val=0;
    }
   else ;
   
   write(fd,&val,4);
   return 0;
}




















































