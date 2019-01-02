#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>




//定义一个资源结构体
static struct resource led_drv_resource[]=
{
   [0] = {.start =0x56000050 , .end = 0x56000050 + 8 - 1 , .flags = IORESOURCE_MEM, },
   [1] = {.start = 5, .end  = 5,.flags = IORESOURCE_IRQ,}
};


static void led_platform_release(struct device * dev)
{}


//定义一个 device_platform 结构体
static struct platform_device led_dev_platform =
{
  .name = "platform_led",
  .id = -1,
  .num_resources = ARRAY_SIZE(led_drv_resource),
  .resource = led_drv_resource,
  .dev = { .release = led_platform_release,  },
};


static int led_device_init(void)
{
  platform_device_register(&led_dev_platform);
  return 0;
}

static void led_device_exit(void)
{
   platform_device_unregister(&led_dev_platform);
}

module_init(led_device_init);
module_exit(led_device_exit);
MODULE_LICENSE("GPL");
