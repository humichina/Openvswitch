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
#include <linux/netlink.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <net/sock.h>
#include <net/tcp.h>
#include <net/dst.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ipt_REJECT.h>
#ifdef CONFIG_BRIDGE_NETFILTER
#include <linux/netfilter_bridge.h>
#endif

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

#define MAX_LEN 64

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
}

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

    read_lock_bh(&user_proc.lock);
    ret = netlink_unicast(nlfd, skb, user_proc.pid, MSG_DONTWAIT);
    read_unlock_bh(&user_proc.lock);

    return ret;

    nlmsg_failure:
        if(skb)
            kfree_skb(skb);
        return -1;
}

void update_speed(struct iphdr * iph)
{
    //return the position the given ip and if none then return the last
    int i;
    for(i=0; i < MAX_LEN; i++) {
        if (spd_buff[i].sip) {
            if ((spd_buff[i].sip==iph->saddr)&&(spd_buff[i].dip==iph->daddr)) {
                spd_buff[i].rate = iph->id;
                return;
            }
        }
        else {
            spd_buff[i].sip = iph->saddr;
            spd_buff[i].dip = iph->daddr;
            spd_buff[i].rate = iph->id;
            return;
        }
    }

    return;
}

void update_ack_rate(struct iphdr * iph)
{
    int i;
    for(i=0; i < MAX_LEN; i++) {
        if (spd_buff[i].sip) {
            if ((spd_buff[i].sip==iph->saddr)&&(spd_buff[i].dip==iph->daddr)) {
                //static inline void csum_replace2(__sum16 *sum, __be16 from, __be16 to);
                csum_replace2(&iph->check, iph->id, spd_buff[i].rate);
                iph->id = spd_buff[i].rate;
                return;
            }
        }
        else {
            //can't find it then do nothing
            return;
        }
    }

    return;
}

static unsigned int send_udp_ack(struct sk_buff *oldskb, __u16 rate)
{
    struct sk_buff * newskb = NULL;
	struct iphdr * oldiph = NULL;
	struct iphdr * niph;
	struct udphdr * oldudph = NULL;
	struct udphdr _oudph;
	struct tcphdr *tcph;
	struct ethhdr *eth_header, *oethdr;

	newskb = alloc_skb(sizeof(struct iphdr) + sizeof(struct tcphdr), GFP_ATOMIC);

	if (!newskb)
		return 0;

	skb_reserve(newskb, 0);

	skb_reset_network_header(newskb);

	oldiph = ip_hdr(oldskb);
	oldudph = skb_header_pointer(oldskb, ip_hdrlen(oldskb),sizeof(_oudph), &_oudph);

	newskb->dev = oldskb->dev;
	newskb->pkt_type = PACKET_OTHERHOST;
	newskb->protocol = oldskb->protocol;
	newskb->ip_summed = CHECKSUM_NONE;
	newskb->priority = 0;

	/* construct TCP header in skb */
	tcph = (struct tcphdr *)skb_put(newskb, sizeof(struct tcphdr));
	memset(tcph, 0, sizeof(struct tcphdr));
	tcph->source = oldudph->dest;
	tcph->dest = oldudph->source;
	tcph->ack = 1;
	tcph->ece = 1; /*UDP_ACK*/
	tcph->doff	= sizeof(struct tcphdr) / 4;

	/* construct ip header in skb */
	niph = (struct iphdr *)skb_put(newskb, sizeof(struct iphdr));
	niph->version = 4;
	niph->ihl = sizeof(struct iphdr)>>2;
	niph->frag_off = 0;
	niph->protocol = IPPROTO_TCP;
	niph->tos = 0;
	niph->daddr = oldiph->saddr;
	niph->saddr = oldiph->daddr;
	niph->ttl = oldiph->ttl;
	niph->tot_len = htons(newskb->len);
	niph->id = rate;
	niph->check = 0;
	niph->check = ip_fast_csum((unsigned char *)niph,niph->ihl);

	/* construct ethernet header in skb */
	eth_header = (struct ethhdr *) skb_put(newskb, sizeof(struct ethhdr));
	oethdr = eth_hdr(oldskb);
	memcpy(eth_header->h_dest, oethdr->h_source, ETH_ALEN);
	memcpy(eth_header->h_source, oethdr->h_dest, ETH_ALEN);
	eth_header->h_proto = htons(ETH_P_IP);

	newskb->csum = skb_checksum(newskb, niph->ihl*4, newskb->len - niph->ihl * 4, 0);
	//tcph->check = csum_tcpudp_magic(niph->saddr, niph->daddr, newskb->len - niph->ihl * 4, IPPROTO_TCP, newskb->csum);
	tcph->check	= tcp_v4_check(sizeof(struct tcphdr),niph->saddr, niph->daddr,csum_partial(tcph, sizeof(struct tcphdr), 0));

	/* send packet */
	if(0 > dev_queue_xmit(newskb))
		goto free_nskb;

free_nskb:
	kfree_skb(newskb);
	return 0;
}

static unsigned int hook_func1(unsigned int hooknum,
			 struct sk_buff *skb,
			 const struct net_device *in,
			 const struct net_device *out,
			 int (*okfn)(struct sk_buff *))
{
    //Receive a packet.
	struct iphdr    * iph;
	struct tcphdr   * tcph;
    struct packet_info info;
	//unsigned char   * http_port = "\x00\x50";
	//char            * data;

	if (skb) {
		iph = ip_hdr(skb);
        if (((iph->saddr<<24))>>24 == 10){
            if (iph && iph->protocol && (iph->protocol == IPPROTO_UDP)) {
                read_lock_bh(&user_proc.lock);
                if(user_proc.pid != 0) {
                    read_unlock_bh(&user_proc.lock);
                    info.rate = iph->id;
                    info.dest = iph->saddr;
                    info.src = iph->daddr;
                    send_to_user(&info);
                }
                else
                    read_unlock_bh(&user_proc.lock);
            }

            if (iph && iph->protocol && (iph->protocol == IPPROTO_TCP)) {
                tcph = (struct tcphdr *)((__u32 *)iph + iph->ihl);

                if (tcph->ack && (tcph->psh == 0) && ((iph->daddr>>24)<(iph->saddr>>24))) {
                    //It is an ACK.
                    //Retrieve the id(L3), s_ip and d_ip. Send it to the user space to limite rate.
                    read_lock_bh(&user_proc.lock);
                    if(user_proc.pid != 0) {
                        read_unlock_bh(&user_proc.lock);
                        info.rate = iph->id;
                        info.dest = iph->daddr;
                        info.src = iph->saddr;
                        send_to_user(&info);
                    }
                    else
                        read_unlock_bh(&user_proc.lock);
                }
                else {
                    //It isn't an ACK.
                    //Retrieve the id(L3), and update the data structure.
                    update_speed(iph);
                }
            }
        }
    }

    return NF_ACCEPT;
}

static unsigned int hook_func2(unsigned int hooknum,
			 struct sk_buff *skb,
			 const struct net_device *in,
			 const struct net_device *out,
			 int (*okfn)(struct sk_buff *))
{
    //Send a packet.
	struct iphdr    * iph;
	struct tcphdr   * tcph;
	//unsigned char   * http_port = "\x00\x50"; //port 80
	//char            * data;

	if (skb) {
		iph = ip_hdr(skb);

        if(((iph->saddr<<24))>>24 == 10) {
            if (iph && iph->protocol && (iph->protocol == IPPROTO_TCP)) {
                tcph = (struct tcphdr *)((__u32 *)iph + iph->ihl);

                if (tcph->ack && (tcph->psh == 0) && ((iph->daddr>>24)<(iph->saddr>>24))) {
                    //It is an ACK to send (of TCP).
                    //According to the s_ip and d_ip write the premiter into the id(L3)
                    update_ack_rate(iph);
                }
                else {
                    //It isn't an ACK.
                    //Set the id(L3) to be max.
                    csum_replace2(&iph->check, iph->id, 65535);
                    iph->id = 65535;
                }
            }
            if (iph && iph->protocol && (iph->protocol == IPPROTO_UDP)) {
                //It isn't an ACK.
                //Set the id(L3) to be max.
                csum_replace2(&iph->check, iph->id, 65534);
                iph->id = 65534;
            }
        }
    }

    return NF_ACCEPT;
}

static struct nf_hook_ops nfho1={
	.hook           = hook_func1,
	.owner          = THIS_MODULE,
	.pf             = PF_INET,
	.hooknum        = NF_INET_LOCAL_IN,//路由后，进入本机的数据
	.priority       = NF_IP_PRI_FIRST,
};

static struct nf_hook_ops nfho2={
	.hook           = hook_func2,
	.owner          = THIS_MODULE,
	.pf             = PF_INET,
	.hooknum        = NF_INET_LOCAL_OUT,//路由前，本机本地进程发出的数据
	.priority       = NF_IP_PRI_FIRST,
};

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

	nf_register_hook(&nfho1);//将用户自己定义的钩子注册到内核中
	nf_register_hook(&nfho2);
	return 0;
}

static void __exit myhook_fini(void)
{
    if(nlfd) {
      sock_release(nlfd->sk_socket);
    }

    kfree(spd_buff);
	nf_unregister_hook(&nfho1);//将用户自己定义的钩子从内核中删除
	nf_unregister_hook(&nfho2);

}

module_init(myhook_init);
module_exit(myhook_fini);
