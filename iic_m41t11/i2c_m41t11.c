#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>
#include <linux/fs.h>



static unsigned short normal_i2c[] = {0x68,I2C_CLIENT_END}; //只有七位   0xd0 >> 1
static unsigned short probe[] = {I2C_CLIENT_END};
static unsigned short ignore[] = {I2C_CLIENT_END};

//定义一个 client 地址数据
static struct i2c_client_address_data addr_data =
{
    .normal_i2c = normal_i2c,
    .probe      = probe,
    .ignore     = ignore, 
};

//定义 client 
static struct i2c_client *i2c_m41t11_client;

//定义 主设备号
static int major;

//定义一个 i2c 字符驱动
static struct i2c_driver i2c_m41t11_driver;

static ssize_t i2c_m41t11_read(struct file *file, char __user *buf, size_t size, loff_t * offset)
{
  printk(" <========= this is m41t11 RTC chip =========>  \n");
  return 0;
}


static ssize_t i2c_m41t11_write(struct file *file, const char __user *buf, size_t size, loff_t * offset)
{
	printk(" <========== this is m41t11 RTC chip =========> \n");
    return 0;
}


//定义字符操作函数
static struct file_operations i2c_m41t11_fops =
{
   .owner = THIS_MODULE,
   .read  = i2c_m41t11_read,
   .write = i2c_m41t11_write,
};



//定义一个字符设备类
static struct class *i2c_m41t11_class;


//匹配处理函数
static int m41t11_match_fun(struct i2c_adapter* adpater,int addr ,int kind)
{
   printk("match m41t11 RTC chip successfully \n"); 
   //初始化 client 结构体
   i2c_m41t11_client = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
   i2c_m41t11_client->adapter =  adpater;
   i2c_m41t11_client->driver = &i2c_m41t11_driver;
   i2c_m41t11_client->addr   = addr;
   strcpy(i2c_m41t11_client->name, "i2c_m41t11_client");
   i2c_attach_client(i2c_m41t11_client);

   //注册字符设备
    major = register_chrdev(0,"i2c_m41t11", &i2c_m41t11_fops); 
 
    i2c_m41t11_class = class_create(THIS_MODULE,"i2c_m41t11_class");                     //   /sys/class/i2c_m41t11_class
    class_device_create(i2c_m41t11_class, NULL, MKDEV(major, 0), NULL,"i2c_m41t11_dev"); //   /dev/i2c_m41t11_dev
    return 0;
}


static int i2c_m41t11_attach(struct i2c_adapter *adapter)
{

   printk("trying to match  m41t11 RTC chip \n");

  return   i2c_probe( adapter , &addr_data , m41t11_match_fun );
} 

static int i2c_m41t11_detach(struct i2c_client *client)
{
	printk("delect  m41t11 RTC chip \n");
   //字符设备
    unregister_chrdev(major, "i2c_m41t11");
    class_destroy(i2c_m41t11_class);
    class_device_destroy(i2c_m41t11_class,MKDEV(major, 0));
   	
    //client
    i2c_detach_client(client);
    kfree(i2c_get_clientdata(client));	
	return 0;
}

//初始化 i2c_m41t11_driver
static struct i2c_driver i2c_m41t11_driver =
{
    .driver = { .name = "i2c_m41t11", },
    .attach_adapter = i2c_m41t11_attach,
    .detach_client  = i2c_m41t11_detach,
};


static int i2c_m41t11_init(void)
{
	i2c_add_driver(&i2c_m41t11_driver);
    return 0;
}


static void i2c_m41t11_exit(void)
{
   i2c_del_driver(&i2c_m41t11_driver);
}
	

module_init(i2c_m41t11_init);
module_exit(i2c_m41t11_exit);

MODULE_AUTHOR("ZSY 1225405552@qq.com");
MODULE_LICENSE("GPL");

