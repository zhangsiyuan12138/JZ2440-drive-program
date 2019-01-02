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
#include <linux/device.h>

//定义一个主设备号
static int major; 

//定义一个类 /sys/class/register_rw_class
static struct class *register_rw_class;

//传参类型
#define REG_R 0
#define REG_W 1

static int register_rw_iotcl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long args)
{
    int long arg_array[2];
    int long *reg_p;
    int long val=0,addr=0;

   //读取寄存器地址 
   copy_from_user(arg_array, (const void __user *)args, 8); 
   val  = arg_array[0];
   addr = arg_array[1];

   //映射寄存器
   reg_p = (volatile unsigned long *)ioremap(addr, 4);
   
    switch(cmd)
    	{
			case REG_R :
			{
		        //读寄存器数据		
	            val = *reg_p;                             
	            copy_to_user((void __user *)args,&val,4); 
		        break;
			}
	        case REG_W :
	        {
		       	//写数据到寄存器
	            *reg_p = val;     
		        break;
			}
    	}
    iounmap(reg_p);
    return 0;
}



//定义一个fops
static struct file_operations register_rw_fops = 
{
    .owner = THIS_MODULE,
	.ioctl = register_rw_iotcl,
};


static int register_rw_init(void)
{
    major = register_chrdev(0, "register_rw", &register_rw_fops);  
    register_rw_class = class_create(THIS_MODULE,"register_rw_class");                 //  /sys/class/register_rw_class
	class_device_create(register_rw_class,NULL, MKDEV(major,0), NULL, "register_rw");   //  /dev/register_rw
     return 0;
}


static void register_rw_exit(void)
{
   unregister_chrdev(major, "register_rw");
   class_destroy(register_rw_class);
   class_device_destroy(register_rw_class,MKDEV(major,0));
}


module_init(register_rw_init);
module_exit(register_rw_exit);

MODULE_AUTHOR("zsy");
MODULE_LICENSE("GPL");

