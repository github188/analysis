#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_LICENSE("Dual BSD/GPL");

// struct holding set of hook function options
static struct nf_hook_ops nfops;

//function to be called by hook
unsigned int hook_func(unsigned int hooknum,
                       struct sk_buff *skb,
                       const struct net_device *in,
                       const struct net_device *out,
                       int (*okfn)(struct sk_buff *))
{
    printk(KERN_INFO "packet dropped");

    return NF_DROP;
}

static int __init hello_init(void)
{
    nfops.hook = &hook_func;           //function to call when conditions below met
    nfops.hooknum = NF_INET_PRE_ROUTING; //called right after packet recieved, first hook in Netfilter
    nfops.pf = PF_INET;                //IPV4 packets
    nfops.priority = NF_IP_PRI_FIRST;  //set to highest priority over all other hook functions
    nf_register_hook(&nfops);          //register hook

    return 0;
}

static void __exit hello_exit(void)
{
    nf_unregister_hook(&nfops);        //cleanup – unregister hook

    return;
}

module_init(hello_init);
module_exit(hello_exit);
