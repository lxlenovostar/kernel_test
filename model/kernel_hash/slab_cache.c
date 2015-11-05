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

	/*
	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    slab_work = kmem_cache_create(CACHE_SLAB_WORK, 
            SHALEN,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    slab_work = kmem_cache_create(CACHE_SLAB_WORK,
           	SHALEN,
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!slab_work) {
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create for workqueue  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }
	*/

	return 0;
}

void free_slab()
{
    kmem_cache_destroy(hash_item_data);
    //kmem_cache_destroy(slab_work);
}
