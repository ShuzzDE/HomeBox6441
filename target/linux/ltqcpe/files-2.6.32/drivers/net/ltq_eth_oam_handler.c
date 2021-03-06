/******************************************************************************
**
** FILE NAME    : ltq_eth_oam_handler.c
** PROJECT      : UGW
** MODULES      : Ethernet OAM
**
** DATE         : 5 APRIL 2013
** AUTHOR       : Purnendu Ghosh
** DESCRIPTION  : Driver for Ethernet OAM handling
** COPYRIGHT    :   Copyright (c) 2013
**                LANTIQ DEUTSCHLAND GMBH,
**                Lilienthalstrasse 15, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author                $Comment
** 5 APRIL 2013 PURNENDU GHOSH         Initiate Version
*******************************************************************************/

/*
 * ####################################
 *              Version No.
 * ####################################
 */
#define VER_MAJOR       0
#define VER_MID         0
#define VER_MINOR       3

#define INLINE                                inline


/*
 * ####################################
 *              Head File
 * ####################################
 */

/*
 *  Common Head File
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/etherdevice.h>  /*  eth_type_trans  */
#include <linux/ethtool.h>      /*  ethtool_cmd     */
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <net/xfrm.h>
#include <linux/if_vlan.h>
#include <../net/8021q/vlan.h>
#include <net/ltq_eth_oam_handler.h>
/*
 *  Chip Specific Head File
 */
#include <asm/ifx/ifx_types.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx/common_routines.h>
#include <asm/ifx/ifx_pmu.h>
#include <asm/ifx/ifx_rcu.h>
#include <asm/ifx/ifx_gpio.h>
#include <asm/ifx/ifx_dma_core.h>
#include <asm/ifx/ifx_clk.h>
#include <switch_api/ifx_ethsw_kernel_api.h>
#include <switch_api/ifx_ethsw_api.h>
#ifdef CONFIG_IFX_ETH_FRAMEWORK
#include <asm/ifx/ifx_eth_framework.h>
#endif

/*
 * ####################################
 *   Parameters to Configure PPE
 * ####################################
 */

static char*  ethwan = "eth1";

//module_param(ethwan, charp, 0000);
module_param(ethwan, charp, S_IRUGO);
MODULE_PARM_DESC(ethwan, "WAN mode, 2 - ETH WAN, 1 - ETH0 mixed.");

#define MY_ETH0_ADDR                            g_my_ethaddr

#ifndef CONFIG_IFX_ETH_FRAMEWORK
  #define ifx_eth_fw_alloc_netdev(size, ifname, dummy)      alloc_netdev(size, ifname, ether_setup)
  #define ifx_eth_fw_free_netdev(netdev, dummy)             free_netdev(netdev)
  #define ifx_eth_fw_register_netdev(netdev)                register_netdev(netdev)
  #define ifx_eth_fw_unregister_netdev(netdev, dummy)       unregister_netdev(netdev)
  #define ifx_eth_fw_netdev_priv(netdev)                    netdev_priv(netdev)
#endif

#define ETH_WATCHDOG_TIMEOUT                    (10 * HZ)
#define my_memcpy             memcpy
#define dbg   printk


/*
 *  Internal Structure of Devices (ETH/ATM)
 */
struct eth_priv_data {
    struct  net_device_stats        stats;

    unsigned int                    rx_packets;
    unsigned int                    rx_bytes;
    unsigned int                    rx_dropped;
    unsigned int                    tx_packets;
    unsigned int                    tx_bytes;
    unsigned int                    tx_errors;
    unsigned int                    tx_dropped;

    unsigned int                    dev_id;
};


/*
 * ####################################
 *             Declaration
 * ####################################
 */
/*
 *  Network Operations
 */
static void eth_setup(struct net_device *);
static struct net_device_stats *eth_oam_get_stats(struct net_device *);
static int eth_oam_open(struct net_device *);
static int eth_stop(struct net_device *);
static int eth_oam_hard_start_xmit(struct sk_buff *, struct net_device *);
static int eth_set_mac_address(struct net_device *, void *);
static int eth_ioctl(struct net_device *, struct ifreq *, int);
//static int eth_change_mtu(struct net_device *, int);
static void eth_tx_timeout(struct net_device *);

/*
 *  Network operations help functions
 */
static INLINE int eth_xmit(struct sk_buff *, unsigned int, int, int, int);


/*
 * ####################################
 *            Local Variable
 * ####################################
 */

struct net_device *ethoam_net_dev = NULL;
static struct net_device *real_net_dev = NULL;
static struct net_device *base_net_dev = NULL;

#if !defined(CONFIG_IFX_ETH_FRAMEWORK) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
static struct net_device_ops g_eth_netdev_ops[2] = {
    {
    .ndo_open = eth_oam_open,
    .ndo_stop = eth_stop,
    .ndo_set_mac_address = eth_set_mac_address,
    .ndo_do_ioctl = eth_ioctl,
    .ndo_tx_timeout =  eth_tx_timeout,
    .ndo_get_stats = eth_oam_get_stats,
    },
    {
    .ndo_open = eth_oam_open,
    .ndo_stop = eth_stop,
    .ndo_set_mac_address = eth_set_mac_address,
    .ndo_do_ioctl = eth_ioctl,
    .ndo_tx_timeout =  eth_tx_timeout,
    .ndo_get_stats = eth_oam_get_stats,
    },
};
#endif

static u8 g_my_ethaddr[MAX_ADDR_LEN * 2] = {0};

/*
 * ####################################
 *            Local Function
 * ####################################
 */

struct net_device * eth_get_oam_dev(void)
{
    return ethoam_net_dev;
}


static INLINE void skb_dump(struct sk_buff *skb, u32 len, char *title, int port, int ch, int is_tx)
{
    int i;

    //if ( g_dump_cnt > 0 )
        //g_dump_cnt--;

    if ( skb->len < len )
        len = skb->len;

    if ( len > 1600 )
    {
        printk("too big data length: skb = %08x, skb->data = %08x, skb->len = %d\n", (u32)skb, (u32)skb->data, skb->len);
        return;
    }

    for ( i = 1; i <= len; i++ )
    {
        if ( i % 16 == 1 )
            printk("  %4d:", i - 1);
        printk(" %02X", (int)(*((char*)skb->data + i - 1) & 0xFF));
        if ( i % 16 == 0 )
            printk("\n");
    }
    if ( (i - 1) % 16 != 0 )
        printk("\n");
}


static void eth_setup(struct net_device *dev)
{
    int port =0;
    u8 *ethaddr;
    u32 val;
    int i;
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    

#ifndef CONFIG_IFX_ETH_FRAMEWORK
    /*  hook network operations */
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,32)
    dev->get_stats           = eth_oam_get_stats;
    dev->open                = eth_oam_open;
    dev->stop                = eth_stop;
    dev->hard_start_xmit = eth_oam_hard_start_xmit;
    dev->set_mac_address     = eth_set_mac_address;
    dev->do_ioctl            = eth_ioctl;
    dev->tx_timeout          = eth_tx_timeout;
  #endif
    dev->watchdog_timeo      = ETH_WATCHDOG_TIMEOUT;
#endif
    priv->dev_id = port;

    ethaddr = MY_ETH0_ADDR + (port ? MAX_ADDR_LEN : 0);

    /*  read MAC address from the MAC table and put them into device    */
    for ( i = 0, val = 0; i < 6; i++ )
        val += dev->dev_addr[i];
    if ( val == 0 )
    {
        for ( i = 0, val = 0; i < 6; i++ )
            val += ethaddr[i];
        if ( val == 0 )
        {
            /*  ethaddr not set in u-boot   */
            dev->dev_addr[0] = 0x00;
            dev->dev_addr[1] = 0x20;
            dev->dev_addr[2] = 0xda;
            dev->dev_addr[3] = 0x86;
            dev->dev_addr[4] = 0x23;
            dev->dev_addr[5] = 0x74 + port;
        }
        else
        {
            for ( i = 0; i < 6; i++ )
                dev->dev_addr[i] = ethaddr[i];
        }
    }
}


static struct net_device_stats *eth_oam_get_stats(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);
    return &priv->stats;

}

static int eth_oam_open(struct net_device *dev)
{
   
    printk("open eth_oam dev->name %s\n", dev->name);
   
#ifndef CONFIG_IFX_ETH_FRAMEWORK
    netif_start_queue(dev);
#endif

    return 0;
}

static int eth_stop(struct net_device *dev)
{
    
#ifndef CONFIG_IFX_ETH_FRAMEWORK
    netif_stop_queue(dev);
#endif

    return 0;
}

static int eth_oam_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
  int ret =0;
  struct ethhdr *eth;
  struct eth_priv_data *priv = NULL;
  if(!skb)
   return ret;

  skb_set_mac_header(skb,0);

  eth = eth_hdr(skb);
  if(eth == NULL)
    return ret;
  //printk("<eth_oam_hard_start_xmit> ethwan  %s %x\n",ethwan,eth->h_proto);
  //skb_dump(skb, 100, "EOAM Packets Tx", 0, 0,0);  
  
  if(eth->h_proto != 0x8902)
     return ret;
  if(real_net_dev == NULL)
  {
     real_net_dev = dev_get_by_name(&init_net, ethwan);
     /*if(real_net_dev) {
        if ( (real_net_dev->priv_flags & IFF_802_1Q_VLAN) ) {
           printk("VLAN Flag found!!\n");
           real_net_dev =  vlan_dev_info(real_net_dev)->real_dev;
        }
     }
     else
        return ret;*/
     if(!real_net_dev) 
        return ret;
  }
  //base_net_dev = dev_get_by_name(&init_net, "eth1");
  //printk("real_net_dev %p base_net_dev %p\n",real_net_dev, base_net_dev);
  priv = ifx_eth_fw_netdev_priv(skb->dev);
  priv->stats.tx_packets++;
  priv->stats.tx_bytes += skb->len;

  skb->dev = real_net_dev;
  priv = ifx_eth_fw_netdev_priv(skb->dev);
  ret = dev_queue_xmit(skb);

  if (likely(ret == NET_XMIT_SUCCESS)) {
    priv->stats.tx_packets++;
    priv->stats.tx_bytes += skb->len;
	} else
    priv->stats.tx_dropped++;

  return NETDEV_TX_OK;

}

static INLINE int eth_xmit(struct sk_buff *skb, unsigned int port, int ch, int spid, int class)
{
  return 0;

}

static int eth_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    return 0;
}

static void eth_tx_timeout(struct net_device *dev)
{
    struct eth_priv_data *priv = ifx_eth_fw_netdev_priv(dev);

    priv->tx_errors++;

    netif_wake_queue(dev);
    
    return;
}

static int eth_set_mac_address(struct net_device *dev, void *p)
{
    struct sockaddr *addr = (struct sockaddr *)p;

    dbg("%s: change MAC from %02X:%02X:%02X:%02X:%02X:%02X to %02X:%02X:%02X:%02X:%02X:%02X", dev->name,
        (u32)dev->dev_addr[0] & 0xFF, (u32)dev->dev_addr[1] & 0xFF, (u32)dev->dev_addr[2] & 0xFF, (u32)dev->dev_addr[3] & 0xFF, (u32)dev->dev_addr[4] & 0xFF, (u32)dev->dev_addr[5] & 0xFF,
        (u32)addr->sa_data[0] & 0xFF, (u32)addr->sa_data[1] & 0xFF, (u32)addr->sa_data[2] & 0xFF, (u32)addr->sa_data[3] & 0xFF, (u32)addr->sa_data[4] & 0xFF, (u32)addr->sa_data[5] & 0xFF);


    my_memcpy(dev->dev_addr, addr->sa_data, dev->addr_len);

    return 0;
}




static INLINE int rx_int_handler(struct net_device *dev)
{
    struct sk_buff *skb = NULL;
    int netif_rx_ret;
    struct eth_priv_data *priv_oam;

    
    priv_oam = ifx_eth_fw_netdev_priv(ethoam_net_dev);
    priv_oam->rx_packets++;
    priv_oam->rx_bytes += skb->len;
    

    netif_rx_ret = netif_rx(skb);    
    return 0;
}


/*
 * ####################################
 *           Init/Cleanup API
 * ####################################
 */

static int __init eth_oam_init(void)
{
    int ret = 0;
    char buf[512];
    struct eth_priv_data *priv;

#ifdef CONFIG_IFX_ETH_FRAMEWORK
    struct ifx_eth_fw_netdev_ops eth_ops = {
        .get_stats      = eth_oam_get_stats,
        .open           = eth_oam_open,
        .stop           = eth_stop,
        .start_xmit     = NULL,
        .set_mac_address= eth_set_mac_address,
        .do_ioctl       = eth_ioctl,
        .tx_timeout     = eth_tx_timeout,
        .watchdog_timeo = ETH_WATCHDOG_TIMEOUT,
  
    };
#endif

    printk("Loading ETH OAM driver for WAN interface....%s.. \n",ethwan);
	
    {
#ifdef CONFIG_IFX_ETH_FRAMEWORK
        eth_ops.start_xmit = eth_oam_hard_start_xmit;
        eth_ops.open = eth_oam_open;
        eth_ops.get_stats= eth_oam_get_stats;
#endif

        strcpy(buf, "eoam");
        ethoam_net_dev = ifx_eth_fw_alloc_netdev(sizeof(struct eth_priv_data), buf, &eth_ops);
        if ( ethoam_net_dev == NULL )
        {
            ret = -ENOMEM;
            goto ALLOC_ETHOAM_NETDEV_FAIL;
        }
        eth_setup(ethoam_net_dev);
        ret = ifx_eth_fw_register_netdev(ethoam_net_dev);
        if ( ret )
        {
            ifx_eth_fw_free_netdev(ethoam_net_dev, 0);
            ethoam_net_dev = NULL;
            goto ALLOC_ETHOAM_NETDEV_FAIL;
        }
        printk("ethoam_net_dev registered/n");
#ifdef CONFIG_IFX_ETH_FRAMEWORK
        priv = ifx_eth_fw_netdev_priv(ethoam_net_dev);
        priv->rx_packets = priv->stats.rx_packets;
        priv->rx_bytes   = priv->stats.rx_bytes;
        priv->rx_dropped = priv->stats.rx_dropped;
        priv->tx_packets = priv->stats.tx_packets;
        priv->tx_bytes   = priv->stats.tx_bytes;
        priv->tx_errors  = priv->stats.tx_errors;
        priv->tx_dropped = priv->stats.tx_dropped;
#endif
    }


    fp_ltq_eth_oam_dev = eth_get_oam_dev;
    //fp_ltq_eth_oam_dev = ethoam_net_dev;
    real_net_dev = dev_get_by_name(&init_net, ethwan);
    if (real_net_dev){
       if(real_net_dev->priv_flags & IFF_802_1Q_VLAN) {
          real_net_dev =  vlan_dev_info(real_net_dev)->real_dev;
       }
    }
    //base_net_dev = dev_get_by_name(&init_net, "eth1");
    printk("Succeeded!\n");
    //printk("real_net_dev %p\n",real_net_dev);
    return 0;

ALLOC_ETHOAM_NETDEV_FAIL:
    printk("ALLOC_ETHOAM_NETDEV_FAIL\n");
    ifx_eth_fw_unregister_netdev(ethoam_net_dev, 0);
    ifx_eth_fw_free_netdev(ethoam_net_dev, 0);
    ethoam_net_dev = NULL;

    return ret;
}

static void __exit eth_oam_exit(void)
{
    ifx_eth_fw_unregister_netdev(ethoam_net_dev, 0);
    ifx_eth_fw_free_netdev(ethoam_net_dev, 0);
    ethoam_net_dev = NULL;
    real_net_dev = NULL;
    fp_ltq_eth_oam_dev = NULL;
    printk("Bye eth_oam_drv\n");
    
}



module_init(eth_oam_init);
module_exit(eth_oam_exit);




MODULE_LICENSE("GPL");
