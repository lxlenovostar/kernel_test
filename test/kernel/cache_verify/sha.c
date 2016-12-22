#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/semaphore.h>
#include <net/tcp.h>
#include <net/ip.h>
#include <net/sock.h>
#include <linux/percpu.h>
#include "sha.h"

//spinlock_t sha_lock = SPIN_LOCK_UNLOCKED;
//DEFINE_MUTEX(sha_lock);
//DECLARE_MUTEX(hash_mutex);

/* - pool: digest algorithm, hash description and scratch buffer */
struct tcp_sha1sig_pool {
    struct hash_desc sha1_desc;
};

static unsigned long tcp_sha1sig_users;
static struct tcp_sha1sig_pool **tcp_sha1sig_pool;
static DEFINE_SPINLOCK(tcp_sha1sig_pool_lock);

static void __tcp_free_sha1sig_pool(struct tcp_sha1sig_pool **pool)
{
    int cpu;     
    for_each_possible_cpu(cpu) {
        struct tcp_sha1sig_pool *p = *per_cpu_ptr(pool, cpu);
        if (p) {
            if (p->sha1_desc.tfm)
                crypto_free_hash(p->sha1_desc.tfm);
            kfree(p);    
            p = NULL;
        }            
    }                
    free_percpu(pool);
}

void tcp_free_sha1sig_pool(void)
{
    struct tcp_sha1sig_pool **pool = NULL; 

    spin_lock_bh(&tcp_sha1sig_pool_lock);
    if (--tcp_sha1sig_users == 0) {
        pool = tcp_sha1sig_pool;
        tcp_sha1sig_pool = NULL;
    }        
    spin_unlock_bh(&tcp_sha1sig_pool_lock);
    if (pool)
        __tcp_free_sha1sig_pool(pool);
}

struct tcp_sha1sig_pool *__tcp_get_sha1sig_pool(int cpu) 
{
    struct tcp_sha1sig_pool **p; 
    spin_lock_bh(&tcp_sha1sig_pool_lock);
    p = tcp_sha1sig_pool;

    if (p) {
        tcp_sha1sig_users++;
	}
    spin_unlock_bh(&tcp_sha1sig_pool_lock);
    return (p ? *per_cpu_ptr(p, cpu) : NULL);
}

struct tcp_sha1sig_pool *tcp_get_sha1sig_pool(void)
{
    int cpu = get_cpu();
    struct tcp_sha1sig_pool *ret = __tcp_get_sha1sig_pool(cpu);
    if (!ret) {
        put_cpu();
	}
    return ret;
}

static struct tcp_sha1sig_pool **__tcp_alloc_sha1sig_pool(void) 
{
    int cpu; 
    struct tcp_sha1sig_pool **pool;

    pool = alloc_percpu(struct tcp_sha1sig_pool *);
    if (!pool)
        return NULL;

    for_each_possible_cpu(cpu) {
        struct tcp_sha1sig_pool *p;
        struct crypto_hash *hash;

        p = kzalloc(sizeof(*p), GFP_ATOMIC);
        if (!p) 
            goto out_free;
        *per_cpu_ptr(pool, cpu) = p; 

        hash = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);
        if (!hash || IS_ERR(hash))
            goto out_free;

		p->sha1_desc.tfm = hash;
    }    
    return pool;
out_free:
    __tcp_free_sha1sig_pool(pool);
    return NULL;
}

struct tcp_sha1sig_pool **tcp_alloc_sha1sig_pool(void) 
{
    struct tcp_sha1sig_pool **pool;
    int alloc = 0; 

retry:
    spin_lock_bh(&tcp_sha1sig_pool_lock);
    pool = tcp_sha1sig_pool;
    if (tcp_sha1sig_users++ == 0) {
        alloc = 1;
        spin_unlock_bh(&tcp_sha1sig_pool_lock);
    } else if (!pool) {
        tcp_sha1sig_users--;
        spin_unlock_bh(&tcp_sha1sig_pool_lock);
        cpu_relax();
        goto retry;
    } else
        spin_unlock_bh(&tcp_sha1sig_pool_lock);


    if (alloc) {
        /* we cannot hold spinlock here because this may sleep. */
        struct tcp_sha1sig_pool **p = __tcp_alloc_sha1sig_pool();
        spin_lock_bh(&tcp_sha1sig_pool_lock);
        if (!p) {
            tcp_sha1sig_users--;
            spin_unlock_bh(&tcp_sha1sig_pool_lock);
            return NULL;
        }
        pool = tcp_sha1sig_pool;
        if (pool) {
            /* oops, it has already been assigned. */
            spin_unlock_bh(&tcp_sha1sig_pool_lock);
            __tcp_free_sha1sig_pool(p);
        } else {
            tcp_sha1sig_pool = pool = p;
            spin_unlock_bh(&tcp_sha1sig_pool_lock);
        }
    }

    return pool;
}

void __tcp_put_sha1sig_pool(void)
{
    tcp_free_sha1sig_pool();
}   

static inline void tcp_put_sha1sig_pool(void)
{
    __tcp_put_sha1sig_pool();
    put_cpu();
}

int tcp_sha1_hash_data(struct tcp_sha1sig_pool *hp, char *src, int len) 
{
    struct scatterlist sg;
    int err;

	sg_init_one(&sg, (u8 *)src, len);
    err = crypto_hash_update(&hp->sha1_desc, &sg, len);
    return err;
}

int tcp_v4_sha1_hash_data(char *sha1_hash, char *src, int len)
{
    struct tcp_sha1sig_pool *hp; 
    struct hash_desc *desc;

    hp = tcp_get_sha1sig_pool();
    if (!hp)
        goto clear_hash_noput;
    desc = &hp->sha1_desc;

    if (crypto_hash_init(desc))
        goto clear_hash;

    if (tcp_sha1_hash_data(hp, src, len))
        goto clear_hash;
    
	if (crypto_hash_final(desc, sha1_hash))
        goto clear_hash;

    tcp_put_sha1sig_pool();
    return 0;

clear_hash:
    tcp_put_sha1sig_pool();
clear_hash_noput:
    memset(sha1_hash, 0, 20);
    return 1;
}

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
	//int i; 

	//mutex_lock(&sha_lock);
	//down(&hash_mutex);
	//spin_lock_bh(&sha_lock);

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
	//spin_unlock_bh(&sha_lock);
    return rc;
}

