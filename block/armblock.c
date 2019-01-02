#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>


// ram_block_disk  �����Ĵ����豸�������
static struct gendisk *ram_block_disk;

//����һ��request_queue �ṹ��
static struct request_queue *ram_block_req;

//����һ��������
static DEFINE_SPINLOCK(ramblock_lock);

//����һ�����豸��
static int major;

//���̴�С
#define GENDISK_SIZE (1024*1024)  

//������ʼ��ַ
static unsigned char *gendisk_buffer;

//getgeo �����������Ϣ
static int gendisk_getgeo(struct block_device *dev, struct hd_geometry *geo)
{
  geo->heads = 2;
  geo->cylinders = 32;
  geo->sectors = (GENDISK_SIZE/2/32/512);
  return 0;
}


static struct block_device_operations gendisk_fops=
{
   .owner = THIS_MODULE,
   .getgeo = gendisk_getgeo,
};

//������
static void ram_block_req_fn(request_queue_t * q)
{
    struct request *req;
	unsigned long offset=0, len=0;

	/* ��ö��� request_queue �е�һ��δ��ɵ����� request */
    while((req =  elv_next_request(q))!=NULL)
    {
       offset = req->sector*512;
	   len = req->current_nr_sectors*512;
       
       if(rq_data_dir(req) == READ)  memcpy( req->buffer,gendisk_buffer+offset,len  );  //read   dest src len
        else                    memcpy( gendisk_buffer+offset, req->buffer, len ); // write

      end_request(req,1);
	}
}


static int ram_block_init(void)
{
   //����gendisk
   ram_block_disk = alloc_disk(16);

   //����gendisk
   
       //������ʼ���������
       ram_block_req = blk_init_queue(ram_block_req_fn,&ramblock_lock);
       ram_block_disk->queue = ram_block_req;
	   
	   //����gendisk����       ע��blk
       major = register_blkdev(0, "ramblock" );   /* cat /proc/devices */
       sprintf(ram_block_disk->disk_name, "armblock");
	   ram_block_disk->fops  = &gendisk_fops;
	   ram_block_disk->first_minor   = 0;
	   ram_block_disk->major         = major;
	   set_capacity(ram_block_disk, GENDISK_SIZE/512);
       gendisk_buffer = kzalloc(GENDISK_SIZE,GFP_KERNEL);    //������ʼ��ַ
       
   //���gendisk
   add_disk(ram_block_disk);
   return 0;
}



static void ram_block_exit(void)
{
    del_gendisk(ram_block_disk);
    put_disk(ram_block_disk);

	blk_cleanup_queue(ram_block_req);

	unregister_blkdev(major,"armblock");
	
    kfree(gendisk_buffer);
}

module_init(ram_block_init);
module_exit(ram_block_exit);

MODULE_LICENSE("GPL");
