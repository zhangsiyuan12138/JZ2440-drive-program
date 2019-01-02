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


//����һ�� fd_info �ṹ��
static struct fb_info *s3c_lcd ;

//IO�Ĵ���
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpgcon;

//LCD���ƼĴ���
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


//����һ�� fd_ops �ṹ��
static struct fb_ops lcd_fb_fops =
{
	.owner		= THIS_MODULE,
	.fb_setcolreg	= lcd_pseudo_palette,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,

};

//���õ�ɫ��
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

	/* ��red,green,blue��ԭɫ�����val */
	val  = chan_to_field(red,	&info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue,	&info->var.blue);
	
	//((u32 *)(info->pseudo_palette))[regno] = val;
	pseudo_palette[regno] = val;
	return 0;
}

static int lcd_init(void)
{
//����һ��     fd_info �ṹ��
   s3c_lcd = framebuffer_alloc(0,NULL);

//��ʼ�� fd_info �ṹ��
   /*���ù̶�����*/
   strcpy(s3c_lcd->fix.id ," mylcd");
   s3c_lcd->fix.smem_len = 480*272*16/8;
   s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;
   s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;//lcd ����
   s3c_lcd->fix.line_length = 480*2;

   /*���ÿɱ����*/
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

   /*���ò�������*/
   s3c_lcd->fbops =&lcd_fb_fops;

   /*��ɫ��*/
   s3c_lcd->pseudo_palette = lcd_pseudo_palette;
   s3c_lcd->screen_size =480*272*2;
  
   //���� LCD�������Ĵ���          IO�Ĵ���
   
     // 1.VD IO
	 gpccon = ioremap(0x56000020,4); 
	 gpdcon = ioremap(0x56000030,4);  
	 *gpccon = 0xaaaaaaaa;
     *gpdcon = 0xaaaaaaaa;
	 
	 // 2. KEYBOARD LCD_PWENB
	 gpbcon = ioremap(0x56000010,4);
     gpbdat = gpbcon +1;
     gpgcon = ioremap(0x56000060,4);
	 *gpbcon &= ~3;   // gpb0 KEYBOARD ����
	 *gpbcon |= 1;
	 *gpbdat &= ~1;

	 *gpgcon |= (3<<8); //gpg4 LCD_PWENB ��Դ
	 // 3. ���ƼĴ���
     lcd_con = ioremap(0x4d000000, sizeof(struct lcd_con));
	 
	     /* lcdcon1  
         * bit[17:8] VCLK = HCLK / [(CLKVAL + 1) �� 2] VCLK=10000KHZ  HCLK=100000KHZ CLKVAL=4
	     * bit[6:5]  0b11 tft_lcd ���
         * bit[4:1]  0b1100 16bpp
         * bit[0]    0 ��ֹvd 1 ʹ��vd
	     */
         lcd_con->LCDCON1 = (4<<8)|(3<<5)|(12<<1)|(0<<0);
		 
		 /* lcdcon2  ��ֱ����ʱ�����
         * bit[31:24] VBPD ��ֱͬ�����ں�ĵ���Ч���� lcdоƬ�ֲ�p15                   T0-T2-T1= 327 - 322 - 1 = 4   3
         * bit[23:14] LINEVAL LCD ���Ĵ�ֱ�ߴ� 320-1 = 319
         * bit[13:6]  VFPD ��ֱͬ������ǰ�ĵ���Ч���� t2 -t5 = 322 - 320 - 1 = 1   
         * bit[5:0]   VSPW  VSYNC ����ĸߵ�ƽ��� T1 -1 = 0
		 */
		 lcd_con->LCDCON2 =  (1<<24) | (271<<14) | (1<<6) | (9);

		 /* lcdcon3  ˮƽ����ʱ�����
		 * bit[25:19]  tft_lcd  HSYNC ���½�������Ч���ݵĿ�ʼ֮��� VCLK ������  T6 -T7 -T8 = 17  16
		 * bit[18:8]    LCD ����ˮƽ�ߴ�   240-1=239
		 * bit[7:0]    ��Ч���ݵĽ����� HSYNC ��������֮��� VCLK ������ 11 - 1 = 10
		 */
		 lcd_con->LCDCON3 =  (1<<19) | (479<<8) | (1);

		 /* lcdcon4 ˮƽ����ͬ���ź�
		 * bit[15:8]  
		 * bit[7:0]   �� VCLK ����ˮƽͬ�������Ⱦ��� HSYNC ����ĸߵ�ƽ���  5-1=4
		 */
		 lcd_con->LCDCON4 = 40;

		 /* lcdcon5
		 * bit[11]  1  16bpp 5:6:5 ��ʽ
		 * bit[10]  0 = VCLK �½���ȡ��Ƶ����
		 * bit[9]   1 = HSYNC�ź�Ҫ��ת,���͵�ƽ��Ч 
		 * bit[8]   1 = VSYNC�ź�Ҫ��ת,���͵�ƽ��Ч
		 * bit[6]   0 = VDEN���÷�ת
		 * bit[3]   0 = PWREN���0
		 * bit[1]  0 = BSWP
		 * bit[0]   1 = HWSWP 2440�ֲ�P413
		 */
		 lcd_con->LCDCON5 = (1<<11)|(0<<10)|(1<<9)|(1<<8)|(0<<6)|(0<<3)|(0<<1)|(1<<0);

	 // 4. �����Դ�
	 s3c_lcd->screen_base =dma_alloc_writecombine(NULL,s3c_lcd->fix.smem_len,&s3c_lcd->fix.smem_start,GFP_KERNEL);
         
		 
		 /*
		 * bit[29:21] ֡�ڴ���ʼ��ַ
         * bit[20:0]  �ӿڻ�������ʼ��ַ 
         */
         lcd_con->LCDSADDR1 = (s3c_lcd->fix.smem_start>>1)& ~(3<<30);

		 /*
		 * bit[20:0] �ӿڻ�����������ַ 
		 */
	     lcd_con->LCDSADDR2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len)>>1 ) & 0x1fffff;

		 /*
		 *  bit[21:11] һ�����һ����������һ���ʼ���ݵ� ��ֵ��һ��
		 *  bit[10:0]  �ӿڵĿ��
		 */
	     lcd_con->LCDSADDR3 =480*1;
		 
//����LCD
   lcd_con->LCDCON1 |= (1<<0);  /* ʹ��LCD������ */
   lcd_con->LCDCON5 |= (1<<3);  /* ʹ��LCD���� */
   *gpbdat |= 1;                /* ����ߵ�ƽ, ʹ�ܱ��� */		


//ע�� fd_info �ṹ��
  register_framebuffer(s3c_lcd);

    return 0;
}

static void lcd_exit(void)
{
   //ȥ��ע��� fd_info �ṹ��
   unregister_framebuffer(s3c_lcd);

   //�ͷ� fd_info �ṹ��ռ�
   framebuffer_release(s3c_lcd);

   //ȡ��ӳ��
   iounmap(lcd_con);
   iounmap(gpbcon);
   iounmap(gpccon);
   iounmap(gpdcon);   
   iounmap(gpgcon); 
   iounmap(gpbdat);

   //�ͷ�֡�ڴ�
   dma_free_writecombine(NULL, s3c_lcd->fix.smem_len, s3c_lcd->screen_base, s3c_lcd->fix.smem_start);

   //�ر�LCD
   lcd_con->LCDCON1 &= ~(1<<0); /* �ر�LCD���� */
   *gpbdat &= ~1;     /* �رձ��� */
   
}

module_init(lcd_init);
module_exit(lcd_exit);
MODULE_LICENSE("GPL");


