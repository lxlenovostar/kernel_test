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
	check_mem(src);

	memcpy(ck->content, src + begin, len);
	ck->max = len;
	
    return len;

	error:
		return -1; 
}

int 
chunk_merge(chunk *ck, char *src, long begin, long end)
{
	int len = end - begin + 1;
	check_mem(src);
/*
	考虑是否重新设计。
	if (len > (ck->limit - ck->max))
	{
		ck->limit = len + ck->max;
		ck->contents = realloc() 
 42     
 43     void *contents = realloc(array->contents, array->max * sizeof(void *));
	}	
*/				
	memcpy(ck->content + ck->max, src + begin, len);
	ck->max += len;
    
	return len;

	error:
		return -1; 
}


