#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/init.h>
#include<linux/delay.h>
#include<asm/uaccess.h>
#include<asm/irq.h>
#include<asm/io.h>
#include<asm/arch/regs-gpio.h>
#include<asm/hardware.h>

static struct class *key_class;
static struct class_device *key_class_device;

volatile  unsigned long *GPFCON=NULL;
volatile  unsigned long *GPFDAT=NULL;
volatile  unsigned long *GPGCON=NULL;
volatile  unsigned long *GPGDAT=NULL;

static int key_drv_open(struct inode *inode,struct file *file)
{
  *GPFCON &= (~((0x03<<0)|(0x03<<4)));
  *GPGCON &= (~((0x03<<6)|(0x03<<22)));

   return 0;
}

 ssize_t key_drv_read(struct file *file,const char __user *buf,size_t count,loff_t *ppos)
{
   unsigned char key_val[4]={0};
   unsigned long keyval=0;

   if(count != sizeof(key_val)) return -EINVAL;

   keyval = *GPFDAT;
   key_val[0] = (keyval & (1<<0))?1:0 ;
   key_val[1] = (keyval & (1<<2))?1:0 ;

   keyval = *GPGDAT;
   key_val[2] = (keyval & (1<<3))?1:0 ;
   key_val[3] = (keyval & (1<<11))?1:0 ;

   copy_to_user(buf,key_val,sizeof(key_val));

   return sizeof(key_val);
}

static struct file_operations key_drv_mode=
{
    .owner = THIS_MODULE,
    .open = key_drv_open,
    .read = key_drv_read,
};

int major=0;
static int key_drv_init(void)
{
    major = register_chrdev(0,"key_drv",&key_drv_mode);

	key_class = class_create(THIS_MODULE,"key_class");
	key_class_device = class_device_create(key_class,NULL,MKDEV(major,0),NULL,"key_devices");

    GPFCON=(volatile unsigned long *)ioremap(0x56000050,16);  
    GPFDAT=GPFCON+1;
    GPGCON=(volatile unsigned long *)ioremap(0x56000060,16);  
    GPGDAT=GPGCON+1;
	
	return 0;
}

static void key_drv_exit(void)
{
   unregister_chrdev(major,"key_drv");

   class_device_unregister(key_class_device);
   class_destroy(key_class);
   iounmap(GPFCON);
   iounmap(GPGCON);
   
}

module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");





























