#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/io.h>

//引脚定义
static unsigned long *gpfcon;
static unsigned long *gpfdat;
static int pin;

//为设备定义一个类
static struct class *led_platform_class;

//定义主设备号
static int major;
static ssize_t led_drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int val;
	copy_from_user(&val, buf, count); 

	if (val == 1) *gpfdat &= ~(1<<pin);
	else *gpfdat |= (1<<pin);
	
	return 0;
}

static int led_drv_open(struct inode *inode, struct file *file)
{
   //配置寄存器
   *gpfcon &= ~(0x3<<(pin*2));
   *gpfcon |= (0x1<<(pin*2));
	return 0;	
}

static struct file_operations led_fops =
{
   .owner = THIS_MODULE,
   .write   = led_drv_write,
   .open   = led_drv_open,
};


static int probe_led(struct platform_device *pfdrv)
{
    // 引脚初始化
    struct resource *sodrv;
	sodrv = platform_get_resource(pfdrv,IORESOURCE_MEM,0);
    gpfcon = ioremap(sodrv->start,sodrv->end - sodrv->start+1);    
    gpfdat = gpfcon +1;

	sodrv =  platform_get_resource(pfdrv,IORESOURCE_IRQ,0);
	pin = sodrv->start;

	//注册字符设备 类 节点
	major = register_chrdev(0,"led_devices",&led_fops);                                  //  proc/devices/led_class
	led_platform_class = class_create(THIS_MODULE,"led_class");                          //  sys/class/led_class
    class_device_create(led_platform_class,NULL, MKDEV(major,0), NULL, "led_dev");       //  dev/led_dev
	return 0;
}

static int  remove_led(struct platform_device *pfdrv)
{
   iounmap(gpfcon);
   unregister_chrdev(major,"led_platform");
   class_destroy(led_platform_class);
   class_device_destroy(led_platform_class,MKDEV(major,0));
   return 0;
}

static struct platform_driver led_platform =
{
   .probe = probe_led,
   .remove = remove_led,
   .driver ={.name = "platform_led",}
};


static int led_drv_init(void)
{
   platform_driver_register(&led_platform);
   return 0;
}


static void led_drv_exit(void)
{
   platform_driver_unregister(&led_platform);
}


module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
