#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

static struct class *firstdrv_class;
static struct class_device *firstdrv_class_dev;

static unsigned long *gpfcon = NULL;
static unsigned long *gpfdat = NULL;

static int first_drvice_open(struct inode *inode,struct file *file) //inode ��ʾ�����ļ�  file �򿪵��ļ�������
{
    // IO��ʼ��
    *gpfcon &= ~((0x03<8)|(0x03<<10)|(0x03<<12));
    *gpfcon |= ((0x01<<8)|(0x01<<10)|(0x01<<12));

    return 0;
}


static ssize_t first_drvice_write(struct file *file, const char __user *buf,size_t count,loff_t *ppos ) // file Ҫд����ļ�  buf Ҫд�����ݻ���  count д������ֽ� ppos ��ǰ�ļ�ƫ����
{
    //���ص�
    int val=0;
	copy_from_user(&val,buf,count);              // para1 �ں˿ռ��ַ  para2�û��ռ��ַ  para3 �����ֽ���
    if(val==1)
    {
      *gpfdat &= ~((1<<4)|(1<<5)|(1<<6));
	}
	else
	{
     *gpfdat |= ((1<<4)|(1<<5)|(1<<6));
	}

   return 0;
}

static struct file_operations first_drvice_mode =
{
    .owner = THIS_MODULE,
   	.open = first_drvice_open,
    .write = first_drvice_write,
};

int major;

int first_drvice_init(void)
{
   major = register_chrdev(0,"first_drvice",&first_drvice_mode);
 
   firstdrv_class = class_create(THIS_MODULE,"firstdrv");          //�� /sys/class ������ firstdrv/ ��Ŀ¼
   firstdrv_class_dev = class_device_create(firstdrv_class,NULL,MKDEV(major,0),NULL,"xyz");  //�� /sys/class/firstdrv ������ xyz/�豸Ŀ¼
                                                                                              //������mdev    �� /dev ������      xyz �豸�ڵ�
   gpfcon = (volatile unsigned long*)0x56000050;       //�û��ռ�һ�ε�ַ�������豸�ڴ���
   gpfdat = gpfcon +1;

																							  
   return 0;
}

void first_drvice_exit(void)
{
   unregister_chrdev(major,"first_drvice");

   class_device_unregister(firstdrv_class_dev);
   class_destroy(firstdrv_class);
}

module_init(first_drvice_init);
module_exit(first_drvice_exit);


MODULE_LICENSE("GPL");























































