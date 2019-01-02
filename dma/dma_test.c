#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>


#define USE_DMA 1
#define NO_DMA 0

int main(int argc ,char **argv)
{
    int fd=0;
	fd = open("/dev/s3c_dma", O_RDWR);
    if(fd<0)   printf("can't open \n");
	if(argc !=2) printf("please using like this : ./dma_test  <dma|no_dma>\n"); 
	
    if(argc ==2)
    	{
    	 if(strcmp(argv[1],"dma")==0) while(1) ioctl(fd,USE_DMA);
         if(strcmp(argv[1],"no_dma")==0)while(1)  ioctl(fd,NO_DMA);
	    }
	
  return 0;
}





















