#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>

int main(int argc,char **argv)
{
   int fd=0;
   int cnt=0;
   unsigned char key_val[4]={1,1,1,1};
   fd = open("/dev/key_devices",O_RDWR);
   if(fd <0) printf("error: can't open device :/dev/key_devices");

   while(1)
   	{   	 
      read(fd,key_val,sizeof(key_val));
	  if(!key_val[0])  printf("key1 is down\n"); 
	  if(!key_val[1])  printf("key2 is down\n"); 
      if(!key_val[2])  printf("key3 is down\n"); 
	  if(!key_val[3])  printf("key4 is down\n"); 
   }
return 0;
}





























































