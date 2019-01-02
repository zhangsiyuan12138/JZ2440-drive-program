#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


#define REG_R 0
#define REG_W 1
static int  arg_array[2];

int main(int argc, char** argv)
{
    int fd = 0;

	if(argc>4 || argc <3) 
		{
			printf("please using like this :\n");
		   printf("./register_rw_tes <read|write> <reg|reg> [value]\n");
		   return -2;
		}
	else
		{
		    fd = open("/dev/register_rw",O_RDWR);			
			if(fd < 0)
				{
				printf("can't open !\n");					
				 return -2;
				}
														
		   if(strcmp("write",argv[1])==0)	  //write											   
				{											
				  arg_array[0]=  strtoul(argv[3], NULL, 0);    //val;
				  arg_array[1]=  strtoul(argv[2], NULL, 0);    //addr
				  
				  ioctl(fd,REG_W,arg_array);				 
				  printf("write 0x%04x to [0x%08x] \n",arg_array[0],arg_array[1]);	
				  return 0; 							   
				}											 
			if(strcmp("read",argv[1])==0) //read
				{                                                 
				  arg_array[1]=  strtoul(argv[2], NULL, 0); 	 
				  
				  ioctl(fd,REG_R,arg_array);                      
				  printf("read 0x%04x from [0x%08x]\n",arg_array[0],arg_array[1]);  
				  return 0;
				}											

	   }
													  
			
    
  return 0;
}

































