/*
 * store last remaining data. 
 */
#include "chunk.h"

chunk *
chunk_create()
{
	chunk *tmp = calloc(1, sizeof(chunk));
    check_mem(tmp);

    tmp->max = 0;
	tmp->limit = LEN;

	tmp->content = malloc(LEN * sizeof(char));
    check_mem(tmp->content); 

    return tmp; 

    error:
		if (tmp) {
    		chunk_destroy(tmp);
		}

    return NULL;
}

void 
chunk_destroy(chunk *ck)
{
	if (ck)
    {   
        free(ck->content);
        free(ck);
    }   
}

void 
chunk_clean(chunk *ck)
{
	if (ck)
	{
		ck->max = 0;
	}
}

int 
chunk_store(chunk *ck, char *src, long begin, long end)
{
	int len = end - begin + 1;
	debug("max is:%lu, limit is:%lu, begin is:%ld, end is:%ld", ck->max, ck->limit, begin, end);
	check_mem(src);
	check(end >= begin, "error end value.");
	memcpy(ck->content, src + begin, len);
	ck->max = (len - 1);
	
    return (len - 1);

	error:
		return -1; 
}

int 
chunk_merge(chunk *ck, char *src, long begin, long end)
{
	int len = end - begin + 1;
	size_t old_max = ck->max;
	check_mem(src);
	check(end >= begin, "error end value.");
	
	if (len > (ck->limit - ck->max))
	{
		ck->limit = len + ck->max;
		char *tmp = realloc(ck->content, ck->limit * sizeof(char));
		check_mem(tmp);
		ck->content = tmp; 

		/*
         * Fixup: +1 or not ?
         */
		memset(ck->content + old_max, 0, (ck->limit - old_max));
	}	
	
	memcpy(ck->content + ck->max, src + begin, len);
	ck->max += len;
    
	return len;

	error:
		return -1; 
}


