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
#include <linux/cdev.h>



//定义一个主设备号
static int major;

//定义一个设备编号
static dev_t dev_id;

//在 proc 定义一个类
static struct class *new_chrdev_class;

//定义一个cdev
static struct cdev c_dev;

static int new_chrdev_open(struct inode *inode, struct file *file)
{
   printk("new char devices open successfully! \n");
   return 0;
}


//驱动操作函数
static struct file_operations new_chrdev_fops =
{
   .owner = THIS_MODULE,
   .open  = new_chrdev_open,
};

static int new_chrdev_init(void)
{
// step1. 
   printk("please using like this : ./new_chrdev_app /dev/new_chrdev0 \n");

   if(major)
   {
      dev_id = MKDEV(major,0);
      register_chrdev_region(dev_id, 3, "new_chrdev");                               /* /proc/new_chrdev */
   }
   else
   	{
      alloc_chrdev_region(&dev_id, 0, 3, "new_chrdev");
      major = MAJOR(dev_id);
    }

   cdev_init(&c_dev, &new_chrdev_fops);
   cdev_add(&c_dev, dev_id,3);

// step2.
   new_chrdev_class = class_create(THIS_MODULE, "new_chrdev_class");                  /*   /class/new_chrdev_class */

// step3.   
   class_deivce_create(new_chrdev_class,NULL, MKDEV(major,0),NULL, "new_chrdev0");    /*   /dev/new_chrdev0 */
   class_device_create(new_chrdev_class,NULL, MKDEV(major,1),NULL, "new_chrdev1");
   class_device_create(new_chrdev_class,NULL, MKDEV(major,2),NULL, "new_chrdev2");

   return 0;
}


static void new_chrdev_exit(void)
{
   class_device_destroy(new_chrdev_class,MKDEV(major, 0));
   class_device_destroy(new_chrdev_class,MKDEV(major, 1));
   class_device_destroy(new_chrdev_class,MKDEV(major, 2));
   class_destroy(new_chrdev_class);

   unregister_chrdev_region(MKDEV(major, 0),3);
   cdev_del(&c_dev);

}


module_init(new_chrdev_init);
module_exit(new_chrdev_exit);

MODULE_AUTHOR("ZSY 1225405552@qq.com");
MODULE_LICENSE("GPL");

