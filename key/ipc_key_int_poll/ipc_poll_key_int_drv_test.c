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

void act_fun(void)  // 捕获信号响应函数
{
  unsigned char key_val=0;
  read(fd,&key_val,1);
  printf("key_val= %d\n",key_val);
}


int main(int argc,char **argv)
{
   unsigned char key_val=0;
   int oflags=0;

   int ret;
   struct pollfd fds[1];

   fd = open("/dev/poll_key_int_drv",O_RDWR);
   if(fd <0) printf("error: can't open device :/dev/poll_key_int_drv");
   
   signal(SIGIO,act_fun);           //捕获信号

   fcntl(fd,F_SETOWN,getpid());    //应用程 序用 fcntl 告诉fd驱动 当前应用程序的PID
   oflags = fcntl(fd,F_GETFL);    // 获取 fd驱动的 状态旗标
   fcntl(fd,F_SETFL,oflags|FASYNC);//更新oflags
   
   while(1)
   	{
      sleep(1000);
   }
return 0;
}





























































