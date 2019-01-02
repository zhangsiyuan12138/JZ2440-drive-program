#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>


//定义一个input_dev 结构体
static  struct input_dev *usb_mouse_dev;

static int usb_pipe;
static char *usb_buff;
static int usb_len;
static struct urb *usb_urb;
static dma_addr_t usb_buff_phy;


// id_table     .bInterfaceClass = (cl), .bInterfaceSubClass = (sc), .bInterfaceProtocol = (pr)
static struct usb_device_id usb_mouse_driver_id_table[] =
{
  { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID,USB_INTERFACE_SUBCLASS_BOOT,USB_INTERFACE_PROTOCOL_MOUSE)},  /* usb驱动都是绑定在interface接口的 */
};


//usb 中断
static void usb_mouse_int(struct urb *urb)
{
#if 0
   int i=0;
   static int cnt=0;
   printk("cnt = %d",++cnt);
   for (i = 0; i < usb_len; i++)
	{
		printk("%02x ", usb_buff[i]);
	}
	printk("\n");
#endif 

#if 1
	static unsigned char pre_val;

	/*	   bit[0] 左		 bit[1] 右 		 bit[3] 滑轮安下
	   */

    if ((pre_val & (1<<0)) != (usb_buff[0] & (1<<0)))
	{
		input_event(usb_mouse_dev, EV_KEY, KEY_L, (usb_buff[0] & (1<<0)) ? 1 : 0);
		input_sync(usb_mouse_dev);
	}

	if ((pre_val & (1<<1)) != (usb_buff[0] & (1<<1)))
	{		
		input_event(usb_mouse_dev, EV_KEY, KEY_S, (usb_buff[0] & (1<<1)) ? 1 : 0);
		input_sync(usb_mouse_dev);
	}

	if ((pre_val & (1<<2)) != (usb_buff[0] & (1<<2)))
	{
		input_event(usb_mouse_dev, EV_KEY, KEY_ENTER, (usb_buff[0] & (1<<2)) ? 1 : 0);
		input_sync(usb_mouse_dev);
	}
	
	pre_val = usb_buff[0];
#endif

//重新提交
	usb_submit_urb(usb_urb, GFP_KERNEL);
}



static int usb_mouse_driver_probe(struct usb_interface *intf,const struct usb_device_id *id)
{
    
   struct usb_device *usb_dev;
   //接口     端点 
   struct usb_host_interface *usb_interface;     /*一组    usb_interface_descriptor 描述符*/
   struct usb_endpoint_descriptor *usb_endpoint;

   	usb_interface = intf->cur_altsetting;       /* 当前使用的usb_interface_descriptor     */
	usb_endpoint = &usb_interface->endpoint[0].desc;


    //输入子系统
    //分配一个input_dev
    usb_mouse_dev = input_allocate_device();

    //初始化 input_dev 结构体
    set_bit(EV_KEY,usb_mouse_dev->evbit);
    set_bit(EV_REP,usb_mouse_dev->evbit);

    set_bit(KEY_L,usb_mouse_dev->keybit);
    set_bit(KEY_S,usb_mouse_dev->keybit);
    set_bit(KEY_ENTER,usb_mouse_dev->keybit);

    //注册 input_dev 结构体
    input_register_device(usb_mouse_dev);

    //USB 
    // 起点：usb_dev usb_pipe 终点：usb_urb usb_buff  长度：usb_len   函数：usb_mouse_int
    usb_dev = interface_to_usbdev(intf);
    usb_pipe = usb_rcvintpipe(usb_dev, usb_endpoint->bEndpointAddress);

	usb_len = usb_endpoint->wMaxPacketSize;

	usb_urb = usb_alloc_urb(0,GFP_KERNEL);
    usb_buff = usb_buffer_alloc(usb_dev, usb_len, GFP_ATOMIC,&usb_buff_phy);

   //设置    URB
    usb_fill_int_urb(usb_urb,usb_dev,usb_pipe,usb_buff, usb_len, usb_mouse_int, NULL, usb_endpoint->bInterval);

  	usb_urb->transfer_dma = usb_buff_phy;
	usb_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

   //提交 URB
   usb_submit_urb(usb_urb, GFP_KERNEL);
   return 0;
  
}


static void usb_mouse_driver_disconnect(struct usb_interface *intf)
{
    struct usb_device *usb_dev= interface_to_usbdev(intf);
   //usb
   usb_kill_urb(usb_urb);
   usb_free_urb(usb_urb);
   usb_buffer_free(usb_dev, usb_len, usb_buff, usb_buff_phy);  
   // input_dev
   input_unregister_device(usb_mouse_dev);
   input_free_device(usb_mouse_dev);
}


//定义一个usb_driver结构体
static struct usb_driver usb_mouse_driver =
{
   .name = "usb_mouse_driver",
   .probe =usb_mouse_driver_probe,
   .disconnect =usb_mouse_driver_disconnect,
   .id_table = usb_mouse_driver_id_table,
};


static int usb_mouse_driver_init(void)
{
    usb_register(&usb_mouse_driver);
    return 0;
}

static void usb_mouse_driver_exit(void)
{
  usb_deregister(&usb_mouse_driver);
}

module_init(usb_mouse_driver_init);
module_exit(usb_mouse_driver_exit);
MODULE_LICENSE("GPL");


