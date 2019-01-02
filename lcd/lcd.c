#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <asm/arch/regs-lcd.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/fb.h>


//定义一个 fd_info 结构体
static struct fb_info *s3c_lcd ;

//IO寄存器
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpgcon;

//LCD控制寄存器
static struct lcd_con 
{
	 unsigned long LCDCON1; 
	 unsigned long LCDCON2; 
	 unsigned long LCDCON3; 
	 unsigned long LCDCON4; 
	 unsigned long LCDCON5; 
	 unsigned long LCDSADDR1; 
	 unsigned long LCDSADDR2; 
	 unsigned long LCDSADDR3; 
	 unsigned long REDLUT; 
	 unsigned long GREENLUT; 
	 unsigned long BLUELUT; 
	 unsigned long RESERVE[9];   //guess why
	 unsigned long DITHMODE; 
	 unsigned long TPAL; 
	 unsigned long LCDINTPND; 
	 unsigned long LCDSRCPND; 
	 unsigned long LCDINTMSK; 
	 unsigned long LPCSEL; 
};

static volatile struct lcd_con *lcd_con;

static int lcd_pseudo_palette(unsigned int regno,  unsigned int red,
						       unsigned int green,  unsigned int blue,
						       unsigned int transp, struct fb_info *info);


//定义一个 fd_ops 结构体
static struct fb_ops lcd_fb_fops =
{
	.owner		= THIS_MODULE,
	.fb_setcolreg	= lcd_pseudo_palette,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,

};

//设置调色板
static u32 pseudo_palette[16];

static inline unsigned int chan_to_field(unsigned int chan, struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}


static int lcd_pseudo_palette(unsigned int regno,  unsigned int red,
						       unsigned int green,  unsigned int blue,
						       unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	
	if (regno > 16)
		return 1;

	/* 用red,green,blue三原色构造出val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	//((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;
	return 0;
}

static int lcd_init(void)
{
//申请一个     fd_info 结构体
   s3c_lcd = framebuffer_alloc(0,NULL);

//初始化 fd_info 结构体
   /*设置固定参数*/
   strcpy(s3c_lcd->fix.id ," mylcd");
   s3c_lcd->fix.smem_len = 480*272*16/8;
   s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;
   s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;//lcd 类型
   s3c_lcd->fix.line_length = 480*2;

   /*设置可变参数*/
   s3c_lcd->var.xres =480;
   s3c_lcd->var.yres =272;
   s3c_lcd->var.xres_virtual =480;
   s3c_lcd->var.yres_virtual =272;
   s3c_lcd->var.bits_per_pixel =16;
   
   s3c_lcd->var.green.length =6;
   s3c_lcd->var.green.offset =5;

   s3c_lcd->var.red.length =5;
   s3c_lcd->var.red.offset =11;

   s3c_lcd->var.blue.length =5;
   s3c_lcd->var.blue.offset =0;

   s3c_lcd->var.activate = FB_ACTIVATE_NOW;

   /*设置操作函数*/
   s3c_lcd->fbops =&lcd_fb_fops;

   /*调色盘*/
   s3c_lcd->pseudo_palette = lcd_pseudo_palette;
   s3c_lcd->screen_size =480*272*2;
  
   //设置 LCD控制器寄存器          IO寄存器
   
     // 1.VD IO
	 gpccon = ioremap(0x56000020,4); 
	 gpdcon = ioremap(0x56000030,4);  
	 *gpccon = 0xaaaaaaaa;
     *gpdcon = 0xaaaaaaaa;
	 
	 // 2. KEYBOARD LCD_PWENB
	 gpbcon = ioremap(0x56000010,4);
     gpbdat = gpbcon +1;
     gpgcon = ioremap(0x56000060,4);
	 *gpbcon &= ~3;   // gpb0 KEYBOARD 背光
	 *gpbcon |= 1;
	 *gpbdat &= ~1;

	 *gpgcon |= (3<<8); //gpg4 LCD_PWENB 电源
	 // 3. 控制寄存器
     lcd_con = ioremap(0x4d000000, sizeof(struct lcd_con));
	 
	     /* lcdcon1  
         * bit[17:8] VCLK = HCLK / [(CLKVAL + 1) × 2] VCLK=10000KHZ  HCLK=100000KHZ CLKVAL=4
	     * bit[6:5]  0b11 tft_lcd 面板
         * bit[4:1]  0b1100 16bpp
         * bit[0]    0 禁止vd 1 使能vd
	     */
         lcd_con->LCDCON1 = (4<<8)|(3<<5)|(12<<1)|(0<<0);
		 
		 /* lcdcon2  垂直方向时间参数
         * bit[31:24] VBPD 垂直同步周期后的的无效行数 lcd芯片手册p15                   T0-T2-T1= 327 - 322 - 1 = 4   3
         * bit[23:14] LINEVAL LCD 面板的垂直尺寸 320-1 = 319
         * bit[13:6]  VFPD 垂直同步周期前的的无效行数 t2 -t5 = 322 - 320 - 1 = 1   
         * bit[5:0]   VSPW  VSYNC 脉冲的高电平宽度 T1 -1 = 0
		 */
		 lcd_con->LCDCON2 =  (1<<24) | (271<<14) | (1<<6) | (9);

		 /* lcdcon3  水平方向时间参数
		 * bit[25:19]  tft_lcd  HSYNC 的下降沿与有效数据的开始之间的 VCLK 周期数  T6 -T7 -T8 = 17  16
		 * bit[18:8]    LCD 面板的水平尺寸   240-1=239
		 * bit[7:0]    有效数据的结束与 HSYNC 的上升沿之间的 VCLK 周期数 11 - 1 = 10
		 */
		 lcd_con->LCDCON3 =  (1<<19) | (479<<8) | (1);

		 /* lcdcon4 水平方向同步信号
		 * bit[15:8]  
		 * bit[7:0]   算 VCLK 的数水平同步脉冲宽度决定 HSYNC 脉冲的高电平宽度  5-1=4
		 */
		 lcd_con->LCDCON4 = 40;

		 /* lcdcon5
		 * bit[11]  1  16bpp 5:6:5 格式
		 * bit[10]  0 = VCLK 下降沿取视频数据
		 * bit[9]   1 = HSYNC信号要反转,即低电平有效 
		 * bit[8]   1 = VSYNC信号要反转,即低电平有效
		 * bit[6]   0 = VDEN不用反转
		 * bit[3]   0 = PWREN输出0
		 * bit[1]  0 = BSWP
		 * bit[0]   1 = HWSWP 2440手册P413
		 */
		 lcd_con->LCDCON5 = (1<<11)|(0<<10)|(1<<9)|(1<<8)|(0<<6)|(0<<3)|(0<<1)|(1<<0);

	 // 4. 分配显存
	 s3c_lcd->screen_base =dma_alloc_writecombine(NULL,s3c_lcd->fix.smem_len,&s3c_lcd->fix.smem_start,GFP_KERNEL);
         
		 
		 /*
		 * bit[29:21] 帧内存起始地址
         * bit[20:0]  视口缓冲区起始地址 
         */
         lcd_con->LCDSADDR1 = (s3c_lcd->fix.smem_start>>1)& ~(3<<30);

		 /*
		 * bit[20:0] 视口缓冲区结束地址 
		 */
	     lcd_con->LCDSADDR2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len)>>1 ) & 0x1fffff;

		 /*
		 *  bit[21:11] 一行最后一个数据与下一行最开始数据的 差值的一半
		 *  bit[10:0]  视口的宽度
		 */
	     lcd_con->LCDSADDR3 =480*1;
		 
//启动LCD
   lcd_con->LCDCON1 |= (1<<0);  /* 使能LCD控制器 */
   lcd_con->LCDCON5 |= (1<<3);  /* 使能LCD本身 */
   *gpbdat |= 1;                /* 输出高电平, 使能背光 */		


//注册 fd_info 结构体
  register_framebuffer(s3c_lcd);

    return 0;
}

static void lcd_exit(void)
{
   //去掉注册的 fd_info 结构体
   unregister_framebuffer(s3c_lcd);

   //释放 fd_info 结构体空间
   framebuffer_release(s3c_lcd);

   //取消映射
   iounmap(lcd_con);
   iounmap(gpbcon);
   iounmap(gpccon);
   iounmap(gpdcon);   
   iounmap(gpgcon); 
   iounmap(gpbdat);

   //释放帧内存
   dma_free_writecombine(NULL, s3c_lcd->fix.smem_len, s3c_lcd->screen_base, s3c_lcd->fix.smem_start);

   //关闭LCD
   lcd_con->LCDCON1 &= ~(1<<0); /* 关闭LCD本身 */
   *gpbdat &= ~1;     /* 关闭背光 */
   
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");


