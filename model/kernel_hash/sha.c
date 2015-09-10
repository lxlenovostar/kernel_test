#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/semaphore.h>
#include "sha.h"

spinlock_t sha_lock = SPIN_LOCK_UNLOCKED;
//DEFINE_MUTEX(sha_lock);
//DECLARE_MUTEX(hash_mutex);

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
     * http://lxr.oss.org.cn/source/net/ipv4/tcp.c?v=2.6.30#L2667
	 */
	struct scatterlist sg;
	struct hash_desc desc;
	
	int rc = 0;
	int i; 

	//mutex_lock(&sha_lock);
	//down(&hash_mutex);
	spin_lock_bh(&sha_lock);

	sg_init_one(&sg, (u8 *)src, len);
	desc.tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);
	desc.flags = 0;
	
	rc = crypto_hash_init(&desc);
	if (rc) {
		printk(KERN_ERR "%s: Error initializing crypto hash; rc = [%d]\n", __func__, rc);
		BUG();
     	goto out;
    }
	rc = crypto_hash_update(&desc, &sg, len);
	if (rc) {
    	printk(KERN_ERR "%s: Error updating crypto hash; rc = [%d]\n", __func__, rc);
		BUG();
        goto out;
    }
	rc = crypto_hash_final(&desc, dst);
	if (rc) {
    	printk(KERN_ERR "%s: Error finalizing crypto hash; rc = [%d]\n", __func__, rc);
		BUG();
        goto out;
    }
	crypto_free_hash(desc.tfm);
   
out:
	//mutex_unlock(&sha_lock);
	//up(&hash_mutex);
	spin_unlock_bh(&sha_lock);
    return rc;
}

