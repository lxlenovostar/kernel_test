#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include "sha.h"

/**
 * ecryptfs_calculate_sha - calculates the SHA-1 of @src
 * @dst: Pointer to 20 bytes of allocated memory
 * @src: Data to be SHA-1'd
 * @len: Length of @src
 *
 * Uses the allocated crypto context that crypt_stat references to
 * generate the MD5 sum of the contents of src.
 */
int ecryptfs_calculate_sha1(char *dst, char *src, int len) {
	/*
     * http://lxr.oss.org.cn/source/fs/ecryptfs/crypto.c?v=2.6.30
     */
	struct scatterlist sg;
	struct hash_desc desc;
	
	int rc = 0;
	int i; 
	//char hashtext[SHA1_LENGTH];

	//way 1		
	/*
	char *plaintext = NULL;
	printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
	plaintext = "c";
	size_t len = strlen(plaintext);
	*/

	//way 2
	/*
	char *plaintext = kmalloc(sizeof(char), GFP_KERNEL);
	printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
	*plaintext = 'c'; 	
	printk(KERN_INFO "valid=%s\n", virt_addr_valid(plaintext) ? "true" : "false");
	size_t len = 1;
	*/

	// way 3.
	/*	
	char plaintext[1] = {'c'};
	size_t len = 1;
	*/

	// way 4.
	/*
	char *plaintext = (char *)__get_free_page(GFP_KERNEL);
	memcpy(plaintext, "c", 1);
	size_t len = 1;
	*/

    //memset(hashtext, 0x00, SHA1_LENGTH);
    //printk(KERN_INFO "sha1: %s\n", __FUNCTION__);
	//printk(KERN_INFO "valid=%s PAGE=%lu, plaintext=%lu, %d , %s\n", virt_addr_valid(plaintext) ? "true" : "false", PAGE_OFFSET, (unsigned long)plaintext, (unsigned long)plaintext - PAGE_OFFSET, ((unsigned long)plaintext > __START_KERNEL_map) ? "true" : "false");

	sg_init_one(&sg, (u8 *)src, len);
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
	rc = crypto_hash_final(&desc, dst);
	if (rc) {
    	printk(KERN_ERR "%s: Error finalizing crypto hash; rc = [%d]\n", __func__, rc);
        goto out;
    }
	crypto_free_hash(desc.tfm);
    
	for (i = 0; i < 20; i++) {
        printk(KERN_INFO "%02x-%d\n", dst[i]&0xff, i);
    }

out:
    return rc;
}

