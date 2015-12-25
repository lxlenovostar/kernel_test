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
    replace_data = kmem_cache_create(CACHE_REPLACE, 
            sizeof(struct replace_item),
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    replace_data = kmem_cache_create(CACHE_REPLACE,
           	sizeof(struct replace_item),
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!replace_data) {
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create for hash item  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

	#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,6,25) )
    mtu_data = kmem_cache_create(CACHE_MTU, 
            MTU,
            0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	#else
    mtu_data = kmem_cache_create(CACHE_MTU,
           	MTU,
            0, SLAB_HWCACHE_ALIGN, NULL);
	#endif

    if (!mtu_data) {
        DEBUG_LOG(KERN_ERR "****** %s : kmem_cache_create for hash item  error\n",
                __FUNCTION__);
        return -ENOMEM;
    }

	return 0;
}

void free_slab()
{
	if (sha_data)
    	kmem_cache_destroy(sha_data);
	if (replace_data)
    	kmem_cache_destroy(replace_data);
	if (mtu_data)
    	kmem_cache_destroy(mtu_data);
}
