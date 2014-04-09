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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MI HU");
MODULE_DESCRIPTION("My hook to modify ACK");

#define MAXLEN 16

struct spd {
    __u32 sip;
    __u32 dip;
    __u16 rate;
};
struct spd *spd_buff;

spd_buff = (struct spd *)kmalloc( sizeof(struct spd)*MAXLEN, GFP_KERNEL);

void update_speed(struct iphdr * iph)
{
    //return the position the given ip and if none then return the last
    int i;
    for(i=0; i < MAXLEN; i++) {
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
    for(i=0; i < MAXLEN; i++) {
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

static unsigned int hook_func1(unsigned int hooknum,
			 struct sk_buff *skb,
			 const struct net_device *in,
			 const struct net_device *out,
			 int (*okfn)(struct sk_buff *))
{
    //Receive a packet.
	struct iphdr    * iph;
	struct tcphdr   * tcph;
	unsigned char   * http_port = "\x00\x50";
	char            * data;

	if (skb) {
		iph = ip_hdr(skb);

		if (iph && iph->protocol && (iph->protocol == IPPROTO_TCP)) {
			tcph = (struct tcphdr *)((__u32 *)iph + iph->ihl);

			if (tcph->ack) {
                //It is an ACK.
                //Retrieve the id(L3), s_ip and d_ip. Send it to the user space to limite rate.
                __u16: iph->id;
                __be32: iph->daddr;
                __be32: iph->saddr;
			}
			else {
                //It isn't an ACK.
                //Retrieve the id(L3), and update the data structure.
                update_speed(iph);
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
	char            * data;

	if (skb) {
		iph = ip_hdr(skb);

		if (iph && iph->protocol && (iph->protocol == IPPROTO_TCP)) {
			tcph = (struct tcphdr *)((__u32 *)iph + iph->ihl);

			if (tcph->ack) {
                //It is an ACK to send.
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
    }

    return NF_ACCEPT;
}

static struct nf_hook_ops nfho1={
	.hook           = hook_func1,
	.owner          = THIS_MODULE,
	.pf             = PF_INET,
	.hooknum        = NF_INET_PRE_ROUTING,//对每一个进入这台主机的数据包进行操作
	.priority       = NF_IP_PRI_FIRST,
};

static struct nf_hook_ops nfho2={
	.hook           = hook_func2,
	.owner          = THIS_MODULE,
	.pf             = PF_INET,
	.hooknum        = NF_INET_POST_ROUTING,//对每一个从这台主机出去的数据包进行操作
	.priority       = NF_IP_PRI_FIRST,
};

static int __init myhook_init(void)
{
	nf_register_hook(&nfho1);//将用户自己定义的钩子注册到内核中
	nf_register_hook(&nfho2);
	return 0;
}

static void __exit myhook_fini(void)
{
	nf_unregister_hook(&nfho1);//将用户自己定义的钩子从内核中删除
	nf_unregister_hook(&nfho2);
}

module_init(myhook_init);
module_exit(myhook_fini);
