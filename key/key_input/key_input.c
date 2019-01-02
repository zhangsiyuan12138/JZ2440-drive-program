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

#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>


//定义 input_dev 
static struct input_dev *key_dev;

//定义定时器节点
static struct timer_list key_timer;

//定义引脚结构体
static struct pin_desc
{
    int irq;
    char *name;
    unsigned int pin;
    unsigned int key_val;
};

//定义一个全局引脚结构体
 static struct pin_desc *key_id;


//结构体数组
 struct pin_desc pin_descs[4]=
{
   {IRQ_EINT0,"s1",S3C2410_GPF0,KEY_L},
   {IRQ_EINT2,"s2",S3C2410_GPF2,KEY_S},
   {IRQ_EINT11,"s3",S3C2410_GPG3,KEY_ENTER},
   {IRQ_EINT19,"s4",S3C2410_GPG11,KEY_LEFTSHIFT},
};


//中断处理函数
static  irqreturn_t key_interrupt(int irq,void *dev_id)
{
    key_id =  (struct pin_desc *)dev_id;
    mod_timer(&key_timer,jiffies+HZ/100);
    return  IRQ_RETVAL(IRQ_HANDLED);
}

//定时器处理函数
static void key_timer_function(unsigned long data)
{
    struct pin_desc *pindesc = key_id; 
    unsigned int pinval;

	if(!pindesc) return ;

	pinval = s3c2410_gpio_getpin(pindesc->pin);

	 if(pinval)
	 	{
           input_event(key_dev,EV_KEY,pindesc->key_val,0); //void input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
		   input_sync(key_dev);
	    }
	 else
	 	{
		 input_event(key_dev,EV_KEY,pindesc->key_val,1); //void input_event(struct input_dev *dev, unsigned int type, unsigned int code, int value)
		 input_sync(key_dev);

	     }

}


static  int key_input_init(void)
{
     int i=0;
	 
   // 1.初始化 input_dev
    key_dev = input_allocate_device();

   // 2.设置 input_dev
    set_bit(EV_KEY,key_dev->evbit);
    set_bit(EV_REP,key_dev->evbit);

    set_bit(KEY_L,key_dev->keybit);
    set_bit(KEY_S,key_dev->keybit);
	set_bit(KEY_ENTER, key_dev->keybit);
	set_bit(KEY_LEFTSHIFT, key_dev->keybit);

   // 3. 注册到 input_dev链表
   input_register_device(key_dev);

   // 4. 中断 和 定时器
   init_timer(&key_timer);  /* TIMER_INITIALIZER */
   key_timer.function = key_timer_function;
   add_timer(&key_timer);

  
   for(i=0;i<4;i++)  request_irq(pin_descs[i].irq,key_interrupt,IRQT_BOTHEDGE,pin_descs[i].name,&pin_descs[i]);

    return 0;
}


static void key_input_exit(void)
{
   int i=0;
   del_timer(&key_timer);
   for(i=0;i<4;i++)  free_irq(pin_descs[i].irq,&pin_descs[i]);
   input_free_device(key_dev);
   input_unregister_device(key_dev);
}


module_init(key_input_init);
module_exit(key_input_exit);
MODULE_LICENSE("GPL");

























































