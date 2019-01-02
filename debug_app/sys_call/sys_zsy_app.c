#include<errno.h>
#include<unistd.h>



void swi_instruct(char *buf,int count)
{
   asm (
        "mov r0 ,%0\n"
        "mov r1 ,%1\n"
        "swi %2\n"
        :
        :"r"(buf), "r"(count), "i" (0x900000 + 352)
		:"r0","r1"
       );
}

int main(int argc , char** argv)
{
   printf("app call swi_instruct\n");
   swi_instruct("from usr mode go to kernel mode",31);
    return 0;
}





































