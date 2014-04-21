#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x67730ce8, "module_layout" },
	{ 0x6e8b4ab9, "nf_unregister_hook" },
	{ 0x37a0cba, "kfree" },
	{ 0xf5f171eb, "sock_release" },
	{ 0x3fa77cce, "nf_register_hook" },
	{ 0x27e1a049, "printk" },
	{ 0x329381f7, "netlink_kernel_create" },
	{ 0x841e2e7d, "init_net" },
	{ 0x8afa47cb, "kmem_cache_alloc_trace" },
	{ 0x3458763f, "kmalloc_caches" },
	{ 0xe113bbbc, "csum_partial" },
	{ 0x78f19814, "netlink_unicast" },
	{ 0x688c218, "__nlmsg_put" },
	{ 0xf397a5c0, "__alloc_skb" },
	{ 0xf37260ab, "_raw_read_unlock_bh" },
	{ 0xfdee7d42, "_raw_read_lock_bh" },
	{ 0x5c3edd59, "_raw_write_unlock_bh" },
	{ 0x32eeaded, "_raw_write_lock_bh" },
	{ 0x8b7d3851, "skb_dequeue" },
	{ 0xe5601624, "kfree_skb" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "98663E439A62F68548BEF04");
