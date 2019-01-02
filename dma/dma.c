#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/poll.h>
#include <linux/dma-mapping.h>

//Դ��ַ
static unsigned char *src;
static unsigned int src_phy;

//Ŀ�ĵ�ַ
static unsigned char* dest;
static unsigned int dest_phy;

//�ռ��С
#define  DMA_SIZE (1204*512)

//������
static struct class *dma_class;

//���豸��
static int major ;

#define USE_DMA 1
#define NO_DMA 0

//DMA�Ĵ���
static struct s3c_dma_reg 
{
     unsigned long disrc;
     unsigned long disrcc;
     unsigned long didst;
     unsigned long didstc;
     unsigned long dcon;
     unsigned long dstat;
     unsigned long dcsrc;
     unsigned long dcdst;    
     unsigned long dmaskting;
}*s3c_dma_regs;

//DMA��ַ
static unsigned long dma_addr0 = 0x4b000000;
static unsigned long dma_addr1 = 0x4b000040;
static unsigned long dma_addr2 = 0x4b000080;
static unsigned long dma_addr3 = 0x4b0000c0;

//Ϊ������̴����ȴ�����
static DECLARE_WAIT_QUEUE_HEAD(dma_wait_queue);

//���̶��б�־λ
static volatile int dma_wait_queue_condition=0;


static int s3c_dma_iotcl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long args)
{
   int i=0;
   	memset(src, 0xAA, DMA_SIZE);
	memset(dest, 0x55, DMA_SIZE);

	
    switch(cmd)
   {
      case NO_DMA :
      	{
          for(i=0;i<DMA_SIZE;i++) dest[i] = src[i];
          if(memcmp(src,dest,DMA_SIZE) == 0) printk("copy without DMA successfuly !\n");
          else printk("copy without DMA fialed !\n");
		  break;
	  }
      case USE_DMA :
      	{
		 // ����DMA�Ĵ���
          s3c_dma_regs->disrc = src_phy;
          s3c_dma_regs->disrcc = (0<<0)|(0<<0);
          s3c_dma_regs->didst = dest_phy;
		  s3c_dma_regs->didstc =(0<<3)|(0<<1)|(0<<0);
          s3c_dma_regs->dcon = (1<<30)|(1<<29)|(0<<28)|(1<<27)|(0<<23)|(0<<20)|(DMA_SIZE<<0);
		  s3c_dma_regs->dmaskting = (1<<1)|(1<<0);

         //���߽���
		  dma_wait_queue_condition = 0; 	 //״̬��־λΪ��		 ����dma.c����	  �� ���̿ɱ��ź��ж�����,���ط�0ֵ��ʾ���߱��ź��ж�
          wait_event_interruptible(dma_wait_queue, dma_wait_queue_condition);  

		  if(memcmp(src,dest,DMA_SIZE) == 0) printk(" copy with DMA successfuly !\n");
          else printk("copy with DMA failed !\n");
		  break;
	  }
	 }
  return 0;

}

//�ַ��豸��������
static struct file_operations dma_fops = 
{
     .owner = THIS_MODULE,
     .ioctl = s3c_dma_iotcl,
};


//�жϴ�����
static     int s3c_dma_irq(int irq, void *devid)
{
	dma_wait_queue_condition = 1;            //״̬Ϊ��     ����dma.c����
	wake_up_interruptible(&dma_wait_queue);
    return IRQ_HANDLED;
}


static int dma_init(void)
{
    //�����ж�
    request_irq(IRQ_DMA3, s3c_dma_irq,0, "s3c_dma", 1);
	
    //����Դ��ַ�ռ�
     src = dma_alloc_writecombine(NULL,DMA_SIZE, &src_phy,GFP_KERNEL);
     //����Ŀ�ĵ�ַ�ռ�
	 dest= dma_alloc_writecombine(NULL,DMA_SIZE, &dest_phy,GFP_KERNEL);

	//dma_regӳ��
	 s3c_dma_regs = ioremap(dma_addr3,sizeof(struct s3c_dma_reg));

    //ע���ַ��豸
    major = register_chrdev(0, "s3c_dma",&dma_fops);

    //ע���� �豸�ڵ�
    dma_class = class_create(THIS_MODULE,"s3c_dma_class");                //     /sys/class/s3c_dma_class
	class_device_create(dma_class,NULL,MKDEV(major,0),NULL, "s3c_dma");   //     /dev/s3c_dma
	                                                                      
    return 0;
}

static void dma_exit(void)
{
   free_irq(IRQ_DMA3,1);
   dma_free_writecombine(NULL,DMA_SIZE, src,GFP_KERNEL);
   dma_free_writecombine(NULL,DMA_SIZE, dest,GFP_KERNEL);
   iounmap(s3c_dma_regs);
   unregister_chrdev(major, "s3c_dma");
   class_destroy(dma_class);
   class_device_destroy( dma_class,MKDEV(major,0)); 
}


module_init(dma_init);
module_exit(dma_exit);

MODULE_AUTHOR("ZSY 	1225405552@qq.com");
MODULE_LICENSE("GPL");


