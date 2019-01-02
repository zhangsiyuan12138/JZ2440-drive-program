#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>

//定义一个 map_info
static struct map_info *s3c_map_info;

//定义一个 mtd_info
static struct mtd_info *s3c_mtd_info;

//定义一个 分区表
static  struct mtd_partition s3c_mtd_partition[] =
{
     [0] = { 
             .name = "bootloader_nor",
             .size = 0x40000,
			 .offset = 0,
	       },
     [1] = {
             .name = "nothing here",
             .size = MTDPART_OFS_APPEND,
			 .size = MTDPART_SIZ_FULL,
	       }
};


static int s3c_nor_flash_init(void)
{
  //分配 map_info
  s3c_map_info = kzalloc(sizeof(struct map_info),GFP_KERNEL);

  //设置 map_info
  s3c_map_info->bankwidth = 2;
  s3c_map_info->name = "s3c_nor_flash";
  s3c_map_info->phys =  0;
  s3c_map_info->size = 0x1000000;
  s3c_map_info->virt =  ioremap(s3c_map_info->phys,s3c_map_info->size);

  simple_map_init(s3c_map_info);
   
  //识别芯片 do_map_probe             map_info --> mtd_info
  s3c_mtd_info = do_map_probe("cfi_probe",s3c_map_info);
  if(s3c_mtd_info) printk("use cfi probe ");
  if(!s3c_mtd_info) 
  	{
      printk("use jedec probe ");
      s3c_mtd_info = do_map_probe("jedec_probe",s3c_map_info);
    }
  if(!s3c_mtd_info) 
  	{
      printk("sorry : can't creat mtd_info ");
	  	kfree(s3c_map_info);
	    iounmap(s3c_map_info->virt);
		return -EIO;
    }
  
  //添加分区 add_mtd_partitions        mtd_info --> gendisk
  add_mtd_partitions(s3c_mtd_info, s3c_mtd_partition , 2);

  return 0;
}

static void s3c_nor_flash_exit(void)
{
  iounmap(s3c_map_info->virt);
  kfree(s3c_map_info);
  del_mtd_partitions(s3c_mtd_info);
}

module_init(s3c_nor_flash_init);
module_exit(s3c_nor_flash_exit);
MODULE_LICENSE("GPL");


