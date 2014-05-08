#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <net/udp.h>
#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/inet.h>
#include <linux/netfilter.h>
#include <net/route.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <net/sock.h>

#ifndef __IMP2_H__
#define __IMP2_H__

#define IMP2_U_PID   0
#define IMP2_K_MSG   1
#define IMP2_CLOSE   2

#define NL_IMP2      31

struct packet_info
{
    __u32 src;
    __u32 dest;
    __u16 rate;
};

#endif


MODULE_LICENSE("GPL");
MODULE_AUTHOR("MI HU");
MODULE_DESCRIPTION("My hook to modify ACK");

#define MAX_LEN 3

struct mutex receive_sem;

static struct sock *nlfd;

struct
{
    __u32 pid;
    rwlock_t lock;
}user_proc;

typedef struct {
    __u32 sip;
    __u32 dip;
    __u16 rate;
}spd;
typedef spd * spd_p;

spd_p spd_buff;
struct packet_info info = {
    .src=22,
    .dest=22,
    .rate=2,
};

static int send_to_user(struct packet_info *info)
{
    int ret;
    int size;
    struct sk_buff *skb;
    sk_buff_data_t old_tail;
    struct nlmsghdr *nlh;
    struct packet_info *packet;

    size = NLMSG_SPACE(sizeof(*info));

    skb = alloc_skb(size, GFP_ATOMIC);
    old_tail = skb->tail;

    nlh = NLMSG_PUT(skb, 0, 0, IMP2_K_MSG, size-sizeof(*nlh));
    packet = NLMSG_DATA(nlh);
    memset(packet, 0, sizeof(struct packet_info));

    packet->src = info->src;
    packet->dest = info->dest;
    packet->rate = info->rate;

    nlh->nlmsg_len = skb->tail - old_tail;
    NETLINK_CB(skb).pid = 0;
    NETLINK_CB(skb).dst_group = 0;

    printk("send_to_user 1!!\n");

    read_lock_bh(&user_proc.lock);
    ret = netlink_unicast(nlfd, skb, user_proc.pid, MSG_DONTWAIT);
    read_unlock_bh(&user_proc.lock);

    printk("send_to_user 2!!\n");

    return ret;

    nlmsg_failure:
        if(skb)
            kfree_skb(skb);
        return -1;
}

static void kernel_receive(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;

    if(skb->len >= sizeof(struct nlmsghdr)) {
        nlh = (struct nlmsghdr *)skb->data;
        if((nlh->nlmsg_len >= sizeof(struct nlmsghdr)) && (skb->len >= nlh->nlmsg_len)) {
            if(nlh->nlmsg_type == IMP2_U_PID) {
                write_lock_bh(&user_proc.lock);
                user_proc.pid = nlh->nlmsg_pid;
                write_unlock_bh(&user_proc.lock);
            }
            else if(nlh->nlmsg_type == IMP2_CLOSE) {
                write_lock_bh(&user_proc.lock);
                if(nlh->nlmsg_pid == user_proc.pid)
                    user_proc.pid = 0;
                write_unlock_bh(&user_proc.lock);
            }
        }
    }
    printk("kernel_receive!!\n");

    send_to_user(&info);
}


static int __init myhook_init(void)
{
    rwlock_init(&user_proc.lock);

    spd_buff = (spd_p)kmalloc( sizeof(spd)*MAX_LEN, GFP_KERNEL);
    memset(spd_buff, 0, sizeof(spd)*MAX_LEN);

    nlfd = netlink_kernel_create(&init_net, NL_IMP2, 0, kernel_receive, NULL, THIS_MODULE);
    if(!nlfd) {
      printk("can not create a netlink socket\n");
      return -1;
    }
    printk("Create a netlink socket!!\n");
    //send_to_user(&info);

	//nf_register_hook(&nfho1);//将用户自己定义的钩子注册到内核中
	//nf_register_hook(&nfho2);
	return 0;
}

static void __exit myhook_fini(void)
{
    if(nlfd) {
      sock_release(nlfd->sk_socket);
    }

    kfree(spd_buff);
	//nf_unregister_hook(&nfho1);//将用户自己定义的钩子从内核中删除
	//nf_unregister_hook(&nfho2);

}

module_init(myhook_init);
module_exit(myhook_fini);
