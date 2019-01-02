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


static struct class *key_int_class;
static struct class_device *key_int_class_device;

volatile  unsigned long *GPFCON=NULL;
volatile  unsigned long *GPFDAT=NULL;
volatile  unsigned long *GPGCON=NULL;
volatile  unsigned long *GPGDAT=NULL;


struct  pin_desc     
{
   unsigned int pin;
   unsigned int key_val;
};

static unsigned char key_val;
static volatile int ev_press=0;

static struct pin_desc pin_desc_array[4]={{S3C2410_GPF0,0x01},{S3C2410_GPF2,0X02},{S3C2410_GPG3,0x03},{S3C2410_GPG11,0x04}};


static DECLARE_WAIT_QUEUE_HEAD(wait_key);

static irqreturn_t key_handler(int irq, void *dev_id)
{
     struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	 unsigned int pinval=0;

	  pinval = s3c2410_gpio_getpin(pindesc->pin);

	  if(pinval)   key_val = 0x08 | pindesc->key_val; 
	  	else       key_val =  pindesc->key_val;

	  
	  wake_up_interruptible(&wait_key);
	  ev_press = 1;
      return IRQ_RETVAL(IRQ_HANDLED);
}



static int key_drv_open(struct inode *inode,struct file *file)
{
      request_irq(IRQ_EINT0, key_handler,IRQT_BOTHEDGE,"KEY1", &pin_desc_array[0]);
	  request_irq(IRQ_EINT2, key_handler,IRQT_BOTHEDGE,"KEY2", &pin_desc_array[1]);
	  request_irq(IRQ_EINT11, key_handler,IRQT_BOTHEDGE,"KEY3",&pin_desc_array[2]);
	  request_irq(IRQ_EINT19, key_handler,IRQT_BOTHEDGE,"KEY4",&pin_desc_array[3]);

      return 0;
}

 ssize_t key_drv_read(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{  
	if (count != 1)
		return -EINVAL;

     wait_event_interruptible(wait_key,ev_press);
     ev_press=0;
     copy_to_user(buf,&key_val,1);
	 
	 return 1;
}

static int key_drv_close(struct inode *inode,struct file *file)
{
	free_irq(IRQ_EINT0, &pin_desc_array[0]);
	free_irq(IRQ_EINT2, &pin_desc_array[1]);
	free_irq(IRQ_EINT11, &pin_desc_array[2]);
	free_irq(IRQ_EINT19, &pin_desc_array[3]);

  return 0;
}

unsigned int key_drv_poll(struct file *file,poll_table *wait)
{
    unsigned int mask=0;
    poll_wait(file,&wait_key,wait);

	if(ev_press) mask |= POLLIN | POLLRDNORM;

    return mask;
}


static struct file_operations key_drv_mode=
{
    .owner = THIS_MODULE,
    .open = key_drv_open,
    .read = key_drv_read,
    .release = key_drv_close,
    .poll = key_drv_poll,       //发生中断才调用
};                              //poll机制就是给定一段时间，在这一段时间内驱动处于睡眠状态一直等待某一个资源，
                                //得到资源则返回，超过时间也返回，有其他信号等待处理也返回
	                            //返回后进程就是运行模式,不是休眠模式

int major=0;
static int key_drv_init(void)
{
    major = register_chrdev(0,"poll_key_int_drv",&key_drv_mode);  //  /proc/devices

	key_int_class = class_create(THIS_MODULE,"key_int_class");
	key_int_class_device = class_device_create(key_int_class,NULL,MKDEV(major,0),NULL,"poll_key_int_drv");  //    /dev/key_int_drv

    GPFCON=(volatile unsigned long *)ioremap(0x56000050,16);  
    GPFDAT=GPFCON+1;
    GPGCON=(volatile unsigned long *)ioremap(0x56000060,16);  
    GPGDAT=GPGCON+1;
	
	return 0;
}

static void key_drv_exit(void)
{
   unregister_chrdev(major,"poll_key_int_drv");

   class_device_unregister(key_int_class_device);
   class_destroy(key_int_class);
   iounmap(GPFCON);
   iounmap(GPGCON);
   
}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");





























