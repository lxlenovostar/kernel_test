#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <linux/spinlock.h>

#define CACHE_NAME "hash_cache"
#define SHA1SIZE 20

struct hashinfo_item
{
	struct list_head c_list;
	uint8_t sha1[SHA1SIZE];
};

int initial_hash_table_cache(void);

#endif 
