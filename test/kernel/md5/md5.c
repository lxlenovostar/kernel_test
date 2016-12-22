#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/mm.h>

#define MD5_LENGTH 16

static int minit(void)
{
	/*
     * http://lxr.oss.org.cn/source/fs/ecryptfs/crypto.c?v=2.6.30
     */
	struct scatterlist sg;
	struct hash_desc desc;
	
	int rc = 0;
	int i; 
	char hashtext[MD5_LENGTH];

	char *plaintext = kmalloc(4096, GFP_KERNEL);
	memcpy(plaintext, "abcd", strlen(plaintext));
	size_t len = strlen(plaintext);
	
	sg_init_one(&sg, (u8 *)plaintext, len);
	desc.tfm = crypto_alloc_hash("md5", 0, CRYPTO_ALG_ASYNC);
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
 
    printk("\n");
	for (i = 0; i < MD5_LENGTH; i++) {
        printk("%02x:", hashtext[i]&0xff);
    }

out:
	kfree(plaintext);
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
MODULE_VERSION("2.0.0.debug");
#else
MODULE_VERSION("2.0.0");
#endif
