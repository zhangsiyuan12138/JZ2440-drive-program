#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc,char **argv)
{
  int fd=0;
  fd =  open(argv[1], O_RDWR);
  if(argc != 2) printf("please using like this : ./new_chrdev /dev/new_chrdev0 \n");
  if(fd<0) printf("can't open %s \n",argv[1]);
  else printf(" open %s \n",argv[1]);

  return 0;
}


















