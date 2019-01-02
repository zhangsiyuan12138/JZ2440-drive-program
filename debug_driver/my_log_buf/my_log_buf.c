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
#include <linux/proc_fs.h>

//����һ�� �����ļ�ϵͳ�µ�Ŀ¼
struct proc_dir_entry *log_buf;

//����һ�� log_buf ������
#define LOG_BUF_SIZE 1024
static char my_log_buf[LOG_BUF_SIZE];

//����һ�� tmp_buf ������
#define TMP_BUF_SIZE 1024
static char my_tmp_buf[TMP_BUF_SIZE];

//log_buf ��дָ��
static int log_buf_r=0,log_buf_w=0;

//����һ���ȴ�����
static DECLARE_WAIT_QUEUE_HEAD(log_buf_wait_queue);



/***********************************************************************************************/

//log_buf �ṩ��д������ ��ָ��
static int log_buf_for_read=0;
static int my_log_buf_empty_for_read(void)
{
	return (log_buf_for_read == log_buf_w);
}
static int my_log_buf_read_ready(char *data)
{
   if(my_log_buf_empty_for_read()) return 0;

   *data = my_log_buf[log_buf_for_read];
   log_buf_for_read = (log_buf_for_read+1)%LOG_BUF_SIZE;
   return 1;
}



/*****************************************************************************************/

// long_buf �Ƿ��
static int log_buf_empty(void)
{
   return (log_buf_r == log_buf_w);
}


// log_buf �Ƿ���
static int log_buf_full(void)
{
  return ((log_buf_r + 1)/LOG_BUF_SIZE == log_buf_w);
}

// log_buf д��һ���ֽ�
static int my_log_buf_write_buf(char data)
{
  if(log_buf_full()) 
  	{
      log_buf_r = (log_buf_r + 1)%LOG_BUF_SIZE;
	  if((log_buf_for_read +1)%LOG_BUF_SIZE == log_buf_r)
	  	{
           log_buf_for_read = log_buf_r;
	    }
    }
   my_log_buf[log_buf_w] = data;
   log_buf_w = (log_buf_w + 1) % LOG_BUF_SIZE;	

	   wake_up_interruptible(&log_buf_wait_queue); 
	  return 1;
}

// log_buf ����һ���ֽ�
static int my_log_buf_read_buf(char *data)
{
  if(log_buf_empty()) 
  	{
      return 0;
    }
   *data = my_log_buf[log_buf_r];
  log_buf_r = (log_buf_r +1) % LOG_BUF_SIZE;
  return 1;
}




/************************************************************************************************************/

// my_printk(data) -> my_tmp_buf -> my_log_buf
int zsy_printk(char* fmt,...)
{
   va_list args;
   int i=0 , j=0;

   va_start(args, fmt);
   i = vsnprintf(my_tmp_buf,INT_MAX,fmt,  args);
   va_end(args);
//   printk("my_tmp_buf: %s i=%d \n",my_tmp_buf,i);

   for(j=0;j<i;j++) my_log_buf_write_buf(my_tmp_buf[j]);
     printk("my_log_buf: %s j=%d \n",my_log_buf,j);
   return i;
}

//     cat /proc/my_log_buf  -->  file_operations  -->  .read -->  copy_to_user
static ssize_t my_log_buf_read(struct file *file, char __user *buf, size_t count, loff_t *opps)
{
  int i = 0;
  char c = 0;

  if((file->f_flags & O_NONBLOCK) && !my_log_buf_empty_for_read()) return -EAGAIN;
  wait_event_interruptible(log_buf_wait_queue, !my_log_buf_empty_for_read());          //û������������          ��������copy_to_user

  while(my_log_buf_read_ready(&c) && i<count)
  	{
  	 __put_user(c,buf); printk("log_buf_for_read = %d \n",log_buf_for_read);
     buf++;
	 i++;
    }
  return i;
}

static int my_log_buf_open(struct inode *inode, struct file *file)
{
  log_buf_for_read = log_buf_r;
  return 0;
}


//����һ��fops
static struct file_operations my_log_buf_fops = 
{
   	.read = my_log_buf_read,
   	.open = my_log_buf_open,
};


static int my_log_buf_init(void)
{
    log_buf = create_proc_entry("my_log_buf", S_IRUSR, &proc_root);
    log_buf->proc_fops = &my_log_buf_fops;
    return 0;
}


static void my_log_buf_exit(void)
{
    remove_proc_entry("my_log_buf",&proc_root);
}

EXPORT_SYMBOL(zsy_printk);


module_init(my_log_buf_init);
module_exit(my_log_buf_exit);

MODULE_AUTHOR("ZSY");
MODULE_LICENSE("GPL");

