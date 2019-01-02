#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>

int main(int argc,char **argv)
{
   int fd=0;
   unsigned char key_val=0;

   fd = open("/dev/key_int_drv",O_RDWR);
   if(fd <0) printf("error: can't open device :/dev/key_int_drv");

   while(1)
   	{   	 
      read(fd,&key_val,1);  
	  printf("key_val=%d\n",key_val);
   }
return 0;
}





























































