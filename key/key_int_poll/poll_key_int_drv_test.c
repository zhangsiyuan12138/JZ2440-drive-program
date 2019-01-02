#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<poll.h>

int main(int argc,char **argv)
{
   int fd=0;
   unsigned char key_val=0;

   int ret;
   struct pollfd fds[1];

   fd = open("/dev/poll_key_int_drv",O_RDWR);
   if(fd <0) printf("error: can't open device :/dev/poll_key_int_drv");


   fds[0].fd =fd;
   fds[0].events = POLLIN;
   while(1)
   	{   	 
   	  ret = poll(fds, 1,5000);
      if(ret==0)  printf("time out\n");
	  else 
	  	{
           read(fd,&key_val,1);
			  printf("key_val=%d\n",key_val);
	  	}
	  }
return 0;
}





























































