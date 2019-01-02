#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
 
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
 
#include <asm/io.h>
 
#include <asm/arch/regs-nand.h>
#include <asm/arch/nand.h>


//定义一个 nand_chip 结构体
static struct nand_chip *s3c_chip;

//定义一个 mtd_info 结构体
static struct mtd_info *s3c_info;

//定义nand寄存器结构体
static struct s3c_nand_regs
{
	unsigned long nfconf  ;
	unsigned long nfcont  ;
	unsigned long nfcmd   ;
	unsigned long nfaddr  ;
	unsigned long nfdata  ;
	unsigned long nfeccd0 ;
	unsigned long nfeccd1 ;
	unsigned long nfeccd  ;
	unsigned long nfstat  ;
	unsigned long nfestat0;
	unsigned long nfestat1;
	unsigned long nfmecc0 ;
	unsigned long nfmecc1 ;
	unsigned long nfsecc  ;
	unsigned long nfsblk  ;
	unsigned long nfeblk  ;

}*s3c_nand_regs;

//分区表
static struct mtd_partition s3c_partitions[] =
{
      	[0] = {
        .name   = "bootloader",
        .size   = 0x00040000,
		.offset	= 0,
	},
	[1] = {
        .name   = "params",
        .offset = MTDPART_OFS_APPEND,
        .size   = 0x00020000,
	},
	[2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_APPEND,
        .size   = 0x00200000,
	},
	[3] = {
        .name   = "root",
        .offset = MTDPART_OFS_APPEND,
        .size   = MTDPART_SIZ_FULL,
	}
};
	
static void s3c_select_chip(struct mtd_info *mtd, int chip)
{
	if (chip == -1)		s3c_nand_regs->nfcont |= (1<<1);		
	else		s3c_nand_regs->nfcont &= ~(1<<1);
}

int s3c_dev_ready(struct mtd_info *mtd)
{
	return (s3c_nand_regs->nfstat & (1<<0));
}


void s3c_cmd_ctrl(struct mtd_info *mtd, int dat,unsigned int ctrl)
{
	if (ctrl & NAND_CLE)		s3c_nand_regs->nfcmd = dat;
	else		s3c_nand_regs->nfaddr = dat;
}


static int s3c_nand_init(void)
{
   struct clk *clk;
   //申请 nand_chip
   s3c_chip = kzalloc(sizeof(struct nand_chip),GFP_KERNEL);
   
   //初始化 nand_chip 函数供扫描用           1.选中 2.状态 3.发地址和命令 5.发数据 6.读数据
   s3c_nand_regs = ioremap(0x4E000000,sizeof(struct s3c_nand_regs));
   s3c_chip->select_chip =s3c_select_chip;
   s3c_chip->dev_ready   =s3c_dev_ready;
   s3c_chip->cmd_ctrl    =s3c_cmd_ctrl;
   s3c_chip->IO_ADDR_R   =&s3c_nand_regs->nfdata;
   s3c_chip->IO_ADDR_W   =&s3c_nand_regs->nfdata;
   s3c_chip->ecc.mode    =NAND_ECC_SOFT;

   //初始化 读写时序NFCONF 开关FCONT           NAND时钟
   s3c_nand_regs->nfconf = (0<<12)|(1<<8)|(0<<4);
   s3c_nand_regs->nfcont = (1<<1) | (1<<0);
   clk =clk_get(NULL,"nand");
   clk_enable(clk);
   
   //申请 初始化 mtd_info
   s3c_info = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
   s3c_info->priv = s3c_chip;
   s3c_info->owner = THIS_MODULE;
   
   //nand_scan           nand_chip --> mtd_info
    nand_scan(s3c_info,1);  

   //add_mtd_partitions  mtd_info --> gendisk
   add_mtd_partitions(s3c_info, s3c_partitions,4);

     return 0;
}

static void s3c_nand_exit(void)
{
  iounmap(s3c_nand_regs);
  kfree(s3c_chip);
  kfree(s3c_info);
  del_mtd_partitions(s3c_info);

}

module_init(s3c_nand_init);
module_exit(s3c_nand_exit);
MODULE_LICENSE("GPL");

