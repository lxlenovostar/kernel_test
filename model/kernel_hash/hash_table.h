#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <linux/spinlock.h>
#include "hash_lock.h"

#define CACHE_NAME "hash_cache"
#define SHA1SIZE 20

struct hashinfo_item
{
	struct list_head c_list;
	uint8_t sha1[SHA1SIZE];
	atomic_t refcnt;
};

void set_sp_timeout(unsigned long timeout);
unsigned long get_sp_timeout(void);

int get_total_sp_hash_count(void);

int initial_sp_hash_table_cache(void);
void release_sp_hash_table_cache(void);
void reset_sp_hash_table_cache(void);

int add_tcp_hash_info(struct sp_tcp_hashinfo *tcp_hash_info);
struct sp_tcp_hashinfo_item* get_tcp_hash_item_by_syn(struct sp_tcp_hashinfo *tcp_hash_info);
struct sp_tcp_hashinfo_item* get_tcp_hash_item_by_ack(struct sp_tcp_hashinfo *tcp_hash_info);
void put_tcp_hash_info(struct sp_tcp_hashinfo_item* tcp_hash_info_item, int delete, unsigned long timeout);

void init_tcp_item(struct sp_tcp_hashinfo_item *tcp_item);
void debug_tcp_info(const char *func, struct sp_tcp_hashinfo *tcp_info);

#endif 
