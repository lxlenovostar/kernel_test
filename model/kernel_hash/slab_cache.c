#include <linux/version.h>
#include <linux/slab.h>
#include "chunk.h"
#include "debug.h"
#include "slab_cache.h"

int alloc_slab(void)
{
	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    sha_data = kmem_cache_create(CACHE_NAME_ITEM, 
            SHALEN,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    sha_data = kmem_cache_create(CACHE_NAME_ITEM,
           	SHALEN,
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!sha_data) {
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create for hash item  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    slab_chunk1 = kmem_cache_create(CACHE_SLAB_CHUNK1, 
            CHUNKSTEP,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    slab_chunk1 = kmem_cache_create(CACHE_SLAB_CHUNK1,
           	CHUNKSTEP,
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!slab_chunk1) {
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create for chunk1  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    slab_chunk2 = kmem_cache_create(CACHE_SLAB_CHUNK2, 
            CHUNKSTEP*2,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    slab_chunk2 = kmem_cache_create(CACHE_SLAB_CHUNK2,
           	CHUNKSTEP*2,
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!slab_chunk2) {
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create for chunk2  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    slab_chunk3 = kmem_cache_create(CACHE_SLAB_CHUNK3, 
            CHUNKSTEP*3,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    slab_chunk3 = kmem_cache_create(CACHE_SLAB_CHUNK3,
           	CHUNKSTEP*3,
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!slab_chunk3) {
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create for chunk3  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    reskb_cachep = kmem_cache_create(CACHE_NAME_SKB,
            sizeof(struct reject_skb),
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
#else
    reskb_cachep = kmem_cache_create(CACHE_NAME_SKB,
            sizeof(struct reject_skb),
            0, SLAB_HWCACHE_ALIGN, NULL);
#endif

    if (!reskb_cachep) {
        printk(KERN_ERR "****** %s : kmem_cache_create  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

	return 0;
}

void free_slab()
{
	if (sha_data)
    	kmem_cache_destroy(sha_data);
    if (slab_chunk1)
		kmem_cache_destroy(slab_chunk1);
    if (slab_chunk2)
		kmem_cache_destroy(slab_chunk2);
    if (slab_chunk3)
    	kmem_cache_destroy(slab_chunk3);
    if (reskb_cachep)
		kmem_cache_destroy(reskb_cachep);
}
