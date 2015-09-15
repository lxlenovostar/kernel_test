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
	{ 0x14522340, "module_layout" },
	{ 0x5a34a45c, "__kmalloc" },
	{ 0xcfadd723, "__percpu_counter_add" },
	{ 0x9a9985be, "percpu_counter_destroy" },
	{ 0xd42b7232, "_write_unlock_bh" },
	{ 0x25ec1b28, "strlen" },
	{ 0xd691cba2, "malloc_sizes" },
	{ 0x2bb6fde2, "__kfifo_put" },
	{ 0xc0a3d105, "find_next_bit" },
	{ 0x55f2580b, "__alloc_percpu" },
	{ 0x8dca832f, "__percpu_counter_sum" },
	{ 0xca975b7a, "nf_register_hook" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0xc9ec4e21, "free_percpu" },
	{ 0xaa1b9b4e, "__pskb_pull_tail" },
	{ 0x9629486a, "per_cpu__cpu_number" },
	{ 0xfe7c4287, "nr_cpu_ids" },
	{ 0xde0bdcff, "memset" },
	{ 0xe4c1df3e, "_read_lock_bh" },
	{ 0xa2a1e5c9, "_write_lock_bh" },
	{ 0x3da5eb6d, "kfifo_alloc" },
	{ 0xea147363, "printk" },
	{ 0xb4390f9a, "mcount" },
	{ 0xb4ca9447, "__kfifo_get" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0x5240ee7, "percpu_counter_batch" },
	{ 0x76a495c1, "cpu_possible_mask" },
	{ 0x7c2458f8, "crypto_destroy_tfm" },
	{ 0x5a57d155, "__percpu_counter_init" },
	{ 0x49da9a9a, "_read_unlock_bh" },
	{ 0x2044fa9e, "kmem_cache_alloc_trace" },
	{ 0x32047ad5, "__per_cpu_offset" },
	{ 0x7e5a6ea3, "nf_unregister_hook" },
	{ 0x3aa1dbcf, "_spin_unlock_bh" },
	{ 0xb6244511, "sg_init_one" },
	{ 0x37a0cba, "kfree" },
	{ 0x93cbd1ec, "_spin_lock_bh" },
	{ 0x57b1dc2b, "crypto_alloc_base" },
	{ 0x15ef2dd9, "kfifo_free" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "A4DBC7B4DCC5049D4197FAA");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 5,
};
