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
/*
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
*/
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
	char plaintext[139] = {0x00,0x22,0x66,0x6c,0x22,0x3e,0x3c,0x61,0x20,0x74,0x61,0x72,0x67,0x65,0x74,0x3d,0x22,0x5f,0x62,0x6c,0x61,0x6e,0x6b,0x22,0x20,0x68,0x72,0x65,0x66,0x3d,0x22,0x68,0x74,0x74,0x70,0x3a,0x2f,0x2f,0x70,0x6c,0x61,0x79,0x2e,0x66,0x70,0x74,0x2e,0x76,0x6e,0x2f,0x6c,0x69,0x76,0x65,0x74,0x76,0x2f,0x22,0x20,0x74,0x69,0x74,0x6c,0x65,0x3d,0x22,0x46,0x50,0x54,0x20,0x50,0x4c,0x41,0x59,0x22,0x20,0x63,0x6c,0x61,0x73,0x73,0x3d,0x22,0x6e,0x61,0x76,0x5f,0x63,0x68,0x65,0x63,0x6b,0x5f,0x68,0x72,0x65,0x66,0x22,0x3e,0x46,0x50,0x54,0x20,0x50,0x4c,0x41,0x59,0x3c,0x2f,0x61,0x3e,0x3c,0x2f,0x73,0x70,0x61,0x6e,0x3e,0x0d,0x0a,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x09,0x3c,0x73,0x70,0x61,0x6e,0x20,0x63,0x6c,0x61,0x73,0x73};	
	//printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
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
   
	char *right_value = "02:a3:ea:d0:1a:1a:de:a3:e8:5b:9d:79:d5:50:de:b4:d3:57:4c:8f:";
 
    printk("\n");
	for (i = 0; i < 20; i++) {
        printk("%02x:", hashtext[i]&0xff);
    }
    printk("\n%s\n", right_value);

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
