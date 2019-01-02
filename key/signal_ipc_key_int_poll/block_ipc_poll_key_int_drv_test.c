#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


int fd=0;


int main(int argc,char **argv)
{
   unsigned char key_val=0;
   int oflags=0;

   int ret;
   struct pollfd fds[1];

   fd = open("/dev/block_ipc_poll_key",O_RDWR | O_NONBLOCK);
   
   if(fd <0) 
   {   
   printf("error: can't open device :/dev/block_ipc_poll_key\n");
   return -1;
   }
  
   while(1)
   	{
		 ret = read(fd,&key_val,1);
         printf("key_val= %d ,return= %d\n",key_val,ret);
		 sleep(5);
   }
return 0;
}





























































