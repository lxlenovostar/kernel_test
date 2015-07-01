/*
 * store the key and value  of hashmap. 
 */
#include "keyvalue.h"

keyvalue * 
keyvalue_create()
{
	keyvalue *kv = calloc(1, sizeof(keyvalue));
	check_mem(kv);

	kv->key = calloc(NUM, SHA);	
	check_mem(kv->key);
	kv->index = 0;
	kv->capacity = NUM;
	kv->expand_rate = NUM;

	return kv;

	error:
		if (kv) {
			keyvalue_destroy(kv);
		}   

	return NULL;
}

void  
keyvalue_destory(keyvalue *kv)
{
	if (key)
	{
		free(kv->key);
	}
}

/*
 * 0 : stands for not full capacity.
 * 1 : stands for full capacity.
 */
int 
keyvalue_full(keyvalue *kv)
{
	if (kv->index <= key->capacity - 1)
		return 0;
	else
		return 1;
}

static inline int 
keyvalue_resize(keyvalue *kv, size_t newsize)
{
    kv->capacity = newsize;
    check(kv->capacity > 0, "The newsize must be > 0.");

    void *contents = realloc(kv->key, kv->capacity * SHA);
    // check contents and assume realloc doesn't harm the original on error
    check_mem(contents);

    kv->key = contents;

    return 0;
	
	error:
    	return -1; 
}

int 
keyvalue_expand(keyvalue *kv)
{
    size_t old_max = kv->capacity;
    check(keyvalue_resize(kv, kv->capacity + kv->expand_rate) == 0,
            "Failed to expand kv to new size: %d",
            kv->capacity + kv->expand_rate);

    memset(kv->key + old_max, 0, array->expand_rate);
 
    return 0;

	error:
    	return -1; 
}




