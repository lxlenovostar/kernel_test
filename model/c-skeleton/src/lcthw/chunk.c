/*
 * store last remaining data. 
 */
#include "chunk.h"

chunk *
chunk_create()
{
	chunk *tmp = calloc(1, sizeof(chunk));
    check_mem(tmp);

    tmp->end = 0;
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
		ck->end = 0;
	}
}

/*
 * store the data in content, and cover the old data.
 * src: the source data.
 * begin: the begin point.
 * end: the end point.
 * return value: the number of copy value.
 */
int 
chunk_store(chunk *ck, char *src, long begin, long end)
{
	int len = end - begin + 1;
	debug("max is:%lu, limit is:%lu, begin is:%ld, end is:%ld", ck->end, ck->limit, begin, end);
	check_mem(src);
	check(end >= begin, "error end value. begin is %ld, end is %ld", begin, end);
	memcpy(ck->content, src + begin, len);
	ck->end = len;
	
    return len;

	error:
		return -1; 
}

/*
 * store the data in content, and merge with the old data.
 * src: the source data.
 * begin: the begin point.
 * end: the end point.
 * return value: the number of copy value.
 */
int 
chunk_merge(chunk *ck, char *src, long begin, long end)
{
	int len = end - begin + 1;
	size_t old_max = ck->end;
	check_mem(src);
	check(end >= begin, "error end value.");
	
	if ((ck->end - 1  + len) > ck->limit)
	{
		ck->limit = len + ck->end - 1;
		char *tmp = realloc(ck->content, ck->limit * sizeof(char));
		check_mem(tmp);
		ck->content = tmp; 
		// + 1 for old_max
		memset(ck->content + old_max, 0, (len+1));
	}	
	
	memcpy(ck->content + ck->end, src + begin, len);
	ck->end += len;
    
	return len;

	error:
		return -1; 
}


