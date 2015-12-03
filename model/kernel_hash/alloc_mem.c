#include "slab_cache.h"
#include "chunk.h"
#include "debug.h"

void alloc_temp_memory(char *tmp_dst, size_t length)
{
	if (length <= CHUNKSTEP) {
		tmp_dst  = kmem_cache_zalloc(slab_chunk1, GFP_ATOMIC);  
		if (!tmp_dst) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc tmp_dst error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		return;
	} else if (length <= CHUNKSTEP*2) {
		tmp_dst  = kmem_cache_zalloc(slab_chunk2, GFP_ATOMIC);  
		if (!tmp_dst) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc tmp_dst error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		return;
	} else if (length <= CHUNKSTEP*3) {
		tmp_dst  = kmem_cache_zalloc(slab_chunk3, GFP_ATOMIC);  
		if (!tmp_dst) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc tmp_dst error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		return;
	} else {	
		tmp_dst = kmalloc(length, GFP_ATOMIC);
		if (!tmp_dst) {
        	DEBUG_LOG(KERN_ERR"****** %s : malloc tmp_dst error\n", __FUNCTION__);
			BUG();	//TODO:	maybe other good way fix it.
		}
		return;
	}
}

void free_temp_memory(char *tmp_dst, size_t length) 
{
	if (length <= CHUNKSTEP) {
		kmem_cache_free(slab_chunk1, tmp_dst);
		return;
	} else if (length <= CHUNKSTEP*2) {
		kmem_cache_free(slab_chunk2, tmp_dst);
		return;
	} else if (length <= CHUNKSTEP*3) {
		kmem_cache_free(slab_chunk3, tmp_dst);
		return;
	} else {	
		kfree(tmp_dst);	
		return;
	}
}

