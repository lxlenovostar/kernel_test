#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/mm.h>

#define SHA1_LENGTH 20
static inline int phys_addr_valid(resource_size_t addr)
{
#ifdef CONFIG_PHYS_ADDR_T_64BIT
          return !(addr >> boot_cpu_data.x86_phys_bits);
  #else
          return 1;
  #endif
}

static int constvalue = 10;

bool __virt_addr_valid(unsigned long x)
{
		unsigned long old_x;
		old_x = x;
		//The kernel text is mapped into the region starting from__START_KERNEL_MAP
        if (x >= __START_KERNEL_map) {
                if (x >= MODULES_VADDR && x <= MODULES_END) {
					printk(KERN_INFO "right here");
				}
                x -= __START_KERNEL_map;
                 if (x >= KERNEL_IMAGE_SIZE) {
						  printk(KERN_INFO "error1 old_x is:%x, start_kernel_map is:%x, new_x is:%d, SIZE is:%d", old_x, __START_KERNEL_map, x, KERNEL_IMAGE_SIZE);
                          return false;
					}
                  x += phys_base;
          } else {
                  if (x < PAGE_OFFSET) {
						  printk(KERN_INFO "error2");
                          return false;
					}
                  x -= PAGE_OFFSET;
                  if (!phys_addr_valid(x)) {
						  printk(KERN_INFO "error3");
                          return false;
					}
          }
  
          return pfn_valid(x >> PAGE_SHIFT);
}

static int minit(void)
{
	/*
     * http://lxr.oss.org.cn/source/fs/ecryptfs/crypto.c?v=2.6.30
     */
	struct scatterlist sg;
	struct hash_desc desc;
	
	int rc = 0;
	int i; 
	char hashtext[SHA1_LENGTH];

	//way 1		
	
	char *plaintext = NULL;
	printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
	plaintext = "c";
	printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
	size_t len = strlen(plaintext);
	

	//way 2
	/*
	char *plaintext = kmalloc(sizeof(char), GFP_KERNEL);
	printk(KERN_INFO "valid=%s\n", __virt_addr_valid(plaintext) ? "true" : "false");
	plaintext = "c"; 	
	printk(KERN_INFO "valid=%s\n", __virt_addr_valid(plaintext) ? "true" : "false");
	if (plaintext >= MODULES_VADDR && plaintext <= MODULES_END)
		printk(KERN_INFO "valid ok\n");
	size_t len = 1;
	printk(KERN_INFO "valid=%s\n", __virt_addr_valid(constvalue) ? "true" : "false");
	*/

	// way 3.
	/*	
	char plaintext[1] = {'c'};
	size_t len = 1;
	*/

	// way 4.
	/*
	char *plaintext = (char *)__get_free_page(GFP_KERNEL);
	printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
	memcpy(plaintext, "c", 1);
	printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
	size_t len = 1;
	*/

    //memset(hashtext, 0x00, SHA1_LENGTH);
    //printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
	//printk(KERN_INFO "valid=%s PAGE=%lu, plaintext=%lu, %d , %s\n", virt_addr_valid(plaintext) ? "true" : "false", PAGE_OFFSET, (unsigned long)plaintext, (unsigned long)plaintext - PAGE_OFFSET, ((unsigned long)plaintext > __START_KERNEL_map) ? "true" : "false");

	sg_init_one(&sg, (u8 *)plaintext, len);
	desc.tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);
	desc.flags = 0;
	
	rc = crypto_hash_init(&desc);
	if (rc) {
		printk(KERN_ERR "%s: Error initializing crypto hash; rc = [%d]\n", __func__, rc);
     	goto out;
    }
	rc = crypto_hash_update(&desc, &sg, len);
	if (rc) {
    	printk(KERN_ERR "%s: Error updating crypto hash; rc = [%d]\n", __func__, rc);
        goto out;
    }
	rc = crypto_hash_final(&desc, hashtext);
	if (rc) {
    	printk(KERN_ERR "%s: Error finalizing crypto hash; rc = [%d]\n", __func__, rc);
        goto out;
    }
	crypto_free_hash(desc.tfm);
    
	for (i = 0; i < 20; i++) {
        //printk(KERN_INFO "%02x-%d\n", hashtext[i]&0xff, i);
    }

out:
    return rc;
}

static void mexit(void)
{
	printk("Exit %s.\n", THIS_MODULE->name);
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lix");
#ifdef DEBUG
MODULE_VERSION("1.4.1.debug");
#else
MODULE_VERSION("1.4.1");
#endif
