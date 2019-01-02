#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <asm/plat-s3c24xx/ts.h>

#include <asm/arch/regs-adc.h>
#include <asm/arch/regs-gpio.h>

//定义一个 input_dev 结构体
static struct input_dev *s3c_ts_dev; 

//定义 ts 结构体
 struct ts_con
{
  unsigned long ADCCON;
  unsigned long ADCTSC;
  unsigned long ADCDLY;
  unsigned long ADCDAT0;
  unsigned long ADCDAT1;
  unsigned long ADCUPDN;
};

static volatile struct ts_con *ts_con;

//定义定时器链表结构体
static struct timer_list ts_timer;

static void pen_up_mode(void)
{
    ts_con->ADCTSC = 0x1d3;     //检测笔尖抬起中断信号
}

static void pen_down_mode(void)
{
    ts_con->ADCTSC = 0xd3;      //检测笔尖落下中断信号
}

//进去xy轴自由转换模式
static void enter_xy_measure_mode(void)
{
   ts_con->ADCTSC |= (1<<2)|(1<<3);
}

//进入 adc_sc 中断模式
static void start_adc_sc(void)
{
   ts_con->ADCCON |= (1<<0);
}

//过滤函数
static int filter(int *x,int *y)
{
   int  count=0;
   for(count=0;count<2;count++)
   {
     if(((x[count] >x[count+1] ? x[count] - x[count+1] : x[count+1] - x[count])<10) && ((y[count] >y[count+1] ? y[count] - y[count+1] : y[count+1] - y[count])<10) )
      return 1; 
	 else 
	 	return 0;
   }
}

//定时器函数
void ts_timer_function(unsigned long data)
{
      if((1<<15) & ts_con->ADCDAT0)  //已经抬起了
      	{
		  pen_down_mode();
	     }
	  else
	  	{
           enter_xy_measure_mode();
           start_adc_sc();	
	    }
   
}

//ts_sc_con 中断处理函数
static irqreturn_t adcstcon_irq(int irq, void *dev_id)
{
    static int cnt=0;
	static int x[4],y[4];

	//优化2 如果已经抬起 则丢包
    unsigned long x_val = ts_con->ADCDAT0 & 0x3ff;
    unsigned long y_val = ts_con->ADCDAT1 & 0x3ff;

    if((1<<15) & ts_con->ADCDAT0)  //已经抬起了
    {
		pen_down_mode();
		cnt = 0;
	}
	else
	{
        //优化3    多次读取取其平均值
        x[cnt] = x_val;
	    y[cnt] = y_val;
	    cnt++;
        if(cnt == 4)
        	{
        	 //优化4     软件过滤
             if(filter(x,y))
             	{
                  printk(" x= %d ,y= %d\n",(x[0]+x[1]+x[2]+x[3])/4,(y[0]+y[1]+y[2]+y[3])/4);
             	}
			  cnt=0;
              pen_up_mode();

			  mod_timer(&ts_timer,jiffies + HZ/100);
            }
		else
			{
			 enter_xy_measure_mode();
             start_adc_sc();			
			}	
	}	
    return IRQ_HANDLED; 
}


// ts_up_down 中断处理函数
static irqreturn_t pen_up_down_irq(int irq, void *dev_id)
{
  if( (1<<15) & ts_con->ADCDAT0)
  	{
       printk(" pen is up\n");      // 1 笔尖抬起态
	   pen_down_mode();
    }
  else
	{
	  //   printk("pen is down\n");   // 0 笔尖落下态
	 //   enter_xy_measure_mode();	    
		 enter_xy_measure_mode();
	     start_adc_sc();
	}
  return IRQ_HANDLED;
}

static int ts_init(void)
{
   struct clk* clk;
//申请 input_dev 
   s3c_ts_dev = input_allocate_device();

//初始化 input_dev
   //事件类型
    set_bit(EV_KEY,s3c_ts_dev->evbit);
    set_bit(EV_ABS,s3c_ts_dev->evbit);
   //哪些事件
    set_bit(BTN_TOUCH,s3c_ts_dev->keybit);
    input_set_abs_params(s3c_ts_dev,ABS_X,0,0X3FF,0,0);
    input_set_abs_params(s3c_ts_dev,ABS_Y,0,0X3FF,0,0);
    input_set_abs_params(s3c_ts_dev,ABS_PRESSURE,0,1,0,0);

//注册 input_dev 结构体
   input_register_device(s3c_ts_dev);

//硬件初始化
   //时钟初始化	
   	clk = clk_get(NULL, "adc");
	clk_enable(clk);

   //映射寄存器
   ts_con = ioremap(0x58000000,sizeof(struct ts_con));

   /* ADCCON
   *  bit[14]    : 1
   *  bin[13:6]  : 49
   *  bit[0]     : 0
   */
   ts_con->ADCCON = (49<<6)|(1<<14);

   request_irq(IRQ_TC,pen_up_down_irq,IRQF_SAMPLE_RANDOM,"ts_up",NULL);
   request_irq(IRQ_ADC,adcstcon_irq,IRQF_SAMPLE_RANDOM,"adcstcon_irq",NULL);

   // 优化1 延时启动定时器
   ts_con->ADCDLY = 0xffff;

   //优化5    滑动和长按
   init_timer(&ts_timer);
   ts_timer.function = ts_timer_function;
   add_timer(&ts_timer);
   
   pen_down_mode();

   return 0;
}


static void ts_exit(void)
{
    input_free_device(s3c_ts_dev);
    input_unregister_device(s3c_ts_dev);
    iounmap(ts_con);
    free_irq(IRQ_TC,NULL);
    free_irq(IRQ_ADC,NULL);
	del_timer(&ts_timer);
}


module_init(ts_init);
module_exit(ts_exit);

MODULE_LICENSE("GPL");


