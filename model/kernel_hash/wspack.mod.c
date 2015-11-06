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
	{ 0x1887ec80, "module_layout" },
	{ 0x806e575f, "kmem_cache_destroy" },
	{ 0x5a34a45c, "__kmalloc" },
	{ 0xcfadd723, "__percpu_counter_add" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x9a9985be, "percpu_counter_destroy" },
	{ 0xd42b7232, "_write_unlock_bh" },
	{ 0x950ffff2, "cpu_online_mask" },
	{ 0xd691cba2, "malloc_sizes" },
	{ 0x2bb6fde2, "__kfifo_put" },
	{ 0xc0a3d105, "find_next_bit" },
	{ 0xcfaaa0af, "queue_work" },
	{ 0x55f2580b, "__alloc_percpu" },
	{ 0x8dca832f, "__percpu_counter_sum" },
	{ 0x6a9f26c9, "init_timer_key" },
	{ 0x2cb81736, "nf_register_hook" },
	{ 0x999e8297, "vfree" },
	{ 0x712aa29b, "_spin_lock_irqsave" },
	{ 0x1b9aca3f, "jprobe_return" },
	{ 0xb94db510, "register_jprobe" },
	{ 0x7d11c268, "jiffies" },
	{ 0xc9ec4e21, "free_percpu" },
	{ 0x343a1a8, "__list_add" },
	{ 0xa9e23b31, "__pskb_pull_tail" },
	{ 0x9629486a, "per_cpu__cpu_number" },
	{ 0xfe7c4287, "nr_cpu_ids" },
	{ 0xe83fea1, "del_timer_sync" },
	{ 0xde0bdcff, "memset" },
	{ 0xe4c1df3e, "_read_lock_bh" },
	{ 0xa2a1e5c9, "_write_lock_bh" },
	{ 0x3da5eb6d, "kfifo_alloc" },
	{ 0xea147363, "printk" },
	{ 0x479c3c86, "find_next_zero_bit" },
	{ 0xb4390f9a, "mcount" },
	{ 0x7329e40d, "kmem_cache_free" },
	{ 0xb4ca9447, "__kfifo_get" },
	{ 0x54410fd6, "destroy_workqueue" },
	{ 0x521445b, "list_del" },
	{ 0x4b07e779, "_spin_unlock_irqrestore" },
	{ 0x45450063, "mod_timer" },
	{ 0x46085e4f, "add_timer" },
	{ 0x5240ee7, "percpu_counter_batch" },
	{ 0xbf71e6d4, "__create_workqueue_key" },
	{ 0xdfd9ed5f, "flush_workqueue" },
	{ 0xee065ced, "kmem_cache_alloc" },
	{ 0x3a3f86d, "unregister_jprobe" },
	{ 0x93fca811, "__get_free_pages" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x76a495c1, "cpu_possible_mask" },
	{ 0x8b6e6108, "crypto_destroy_tfm" },
	{ 0x5a57d155, "__percpu_counter_init" },
	{ 0x49da9a9a, "_read_unlock_bh" },
	{ 0x2044fa9e, "kmem_cache_alloc_trace" },
	{ 0x32047ad5, "__per_cpu_offset" },
	{ 0xe4a639f8, "kmem_cache_create" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x466cf2bc, "nf_unregister_hook" },
	{ 0x3aa1dbcf, "_spin_unlock_bh" },
	{ 0xb6244511, "sg_init_one" },
	{ 0x37a0cba, "kfree" },
	{ 0x6067a146, "memcpy" },
	{ 0x9edbecae, "snprintf" },
	{ 0x93cbd1ec, "_spin_lock_bh" },
	{ 0x8f1953fa, "crypto_alloc_base" },
	{ 0x15ef2dd9, "kfifo_free" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "76FB3FB8771043A8D2C560E");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 4,
};
