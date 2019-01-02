#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>


static struct class *log_buf_class;
int major=0;

static int log_buf_open(struct inode *iode, struct file *file)
{
	static int cnt=0;
    zsy_printk("writing time of my_log_buf is :  %d\n",++cnt);
    return 0;
}


static struct file_operations log_buf_chrdev =
{
   .owner =THIS_MODULE,
  	.open =log_buf_open,
};

static int test_module_init(void)
{
   major = register_chrdev(0,"log_buf", &log_buf_chrdev);
   log_buf_class = class_create(THIS_MODULE, "log_buf_cls");                   //   /sys/class/log_buf
   class_device_create(log_buf_class,NULL,MKDEV(major,0),NULL,"log_buf");  //  /dev/log_buf


   printk("insmod ok \n");
   return 0;
}

static void test_module_exit(void)
{
    unregister_chrdev(major,"log_buf");
    class_destroy(log_buf_class);
	class_device_destroy(log_buf_class,MKDEV(major,0));

	printk("remove ok\n");
}

module_init(test_module_init);
module_exit(test_module_exit);

MODULE_LICENSE("GPL");

