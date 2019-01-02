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

void act_fun(void)  // �����ź���Ӧ����
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
   
   signal(SIGIO,act_fun);           //�����ź�

   fcntl(fd,F_SETOWN,getpid());    //Ӧ�ó� ���� fcntl ����fd���� ��ǰӦ�ó����PID
   oflags = fcntl(fd,F_GETFL);    // ��ȡ fd������ ״̬���
   fcntl(fd,F_SETFL,oflags|FASYNC);//����oflags
   
   while(1)
   	{
      sleep(1000);
   }
return 0;
}





























































