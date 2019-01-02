#include <linux/module.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ip.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>


//定义一个 device_net结构体
static struct net_device *s3c_vir_net;

//netif_rx
static int sent_up_sk_buff(struct sk_buff *skb,struct net_device *dev)  //copy frome LLD3
{
        unsigned char *type;
		struct iphdr *ih;
		__be32 *saddr, *daddr, tmp;
		unsigned char	tmp_dev_addr[ETH_ALEN];
		struct ethhdr *ethhdr;
		
		struct sk_buff *rx_skb;
			
		// 从硬件读出/保存数据
		/* 对调"源/目的"的mac地址 */
		ethhdr = (struct ethhdr *)skb->data;
		memcpy(tmp_dev_addr, ethhdr->h_dest, ETH_ALEN);
		memcpy(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);
		memcpy(ethhdr->h_source, tmp_dev_addr, ETH_ALEN);
	
		/* 对调"源/目的"的ip地址 */    
		ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));
		saddr = &ih->saddr;
		daddr = &ih->daddr;
	
		tmp = *saddr;
		*saddr = *daddr;
		*daddr = tmp;
		
		//((u8 *)saddr)[2] ^= 1; /* change the third octet (class C) */
		//((u8 *)daddr)[2] ^= 1;
		type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
		//printk("tx package type = %02x\n", *type);
		
		// 修改类型, 原来0x8表示ping
		*type = 0; /* 0表示reply */
		
		ih->check = 0;		   /* and rebuild the checksum (ip needs it) */
		ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
		
		// 构造一个sk_buff
		rx_skb = dev_alloc_skb(skb->len + 2);
		skb_reserve(rx_skb, 2); /* align IP on 16B boundary */	
		memcpy(skb_put(rx_skb, skb->len), skb->data, skb->len);
	
		/* Write metadata, and then pass to the receive level */
		rx_skb->dev = dev;
		rx_skb->protocol = eth_type_trans(rx_skb, dev);
		rx_skb->ip_summed = CHECKSUM_UNNECESSARY; /* don't check it */
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += skb->len;
	
		// 提交sk_buff
		netif_rx(rx_skb);
}



//hard_start_xmit
static int receive_sk_buff(struct sk_buff *vir_sk_buff,struct net_device *vir_net_device)
{
   //发包计数
   static int cnt=0;
   printk("the number of sent pack is : %d \n",++cnt);

   //停止队列      发送sk_buff包到网卡中
    netif_stop_queue(vir_net_device);

   //从网卡中直接读取数据sk_buff，通过netif_rx发送给上层网络结构，（这里自己构造sk_buff包上报）
    sent_up_sk_buff(vir_sk_buff,vir_net_device);

   //清除sk_buff
    dev_kfree_skb(vir_sk_buff);

   //恢复队列
    netif_wake_queue(vir_net_device);

   //设置统计信息       包个数....
    vir_net_device->stats.tx_packets++;
    vir_net_device->stats.tx_bytes+=vir_sk_buff->len;

  return 0;
}



static int vir_net_init(void)
{
   //分配一个device_net结构体
   s3c_vir_net = alloc_netdev(0,"fake_net", ether_setup);
   
   //设置结构体函数         MAC地址    ping功能
   s3c_vir_net->hard_start_xmit = receive_sk_buff;

   s3c_vir_net->dev_addr[0] =0x08;
   s3c_vir_net->dev_addr[1] =0x89;
   s3c_vir_net->dev_addr[2] =0x89;
   s3c_vir_net->dev_addr[3] =0x89;
   s3c_vir_net->dev_addr[4] =0x89;
   s3c_vir_net->dev_addr[5] =0x11;

   s3c_vir_net->flags    |= IFF_NOARP;
   s3c_vir_net->features |= NETIF_F_NO_CSUM;	

   //注册结构体
   register_netdev(s3c_vir_net);
   return 0;
}

static void vir_net_exit(void)
{
   unregister_netdev(s3c_vir_net);
   free_netdev(s3c_vir_net);
}

module_init(vir_net_init);
module_exit(vir_net_exit);

MODULE_AUTHOR("ZSY");
MODULE_LICENSE("GPL");

