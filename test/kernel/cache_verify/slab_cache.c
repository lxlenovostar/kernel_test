#include <linux/version.h>
#include <linux/slab.h>
#include "chunk.h"
#include "debug.h"
#include "slab_cache.h"

int alloc_slab(void)
{
	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    hash_item_data = kmem_cache_create(CACHE_NAME_ITEM, 
            SHALEN,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    hash_item_data = kmem_cache_create(CACHE_NAME_ITEM,
           	SHALEN,
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!hash_item_data) {
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


	return 0;
}

void free_slab()
{
    kmem_cache_destroy(hash_item_data);
    kmem_cache_destroy(slab_chunk1);
    kmem_cache_destroy(slab_chunk2);
    kmem_cache_destroy(slab_chunk3);
}
