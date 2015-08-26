/*VincentR2D2&liyi*/
/* Copyright (C) 2011  wangsu Corporation.  All Rights Reserved. */

#ifndef __SP_HASH_TABLE_H__
#define __SP_HASH_TABLE_H__

#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include "ws_sp_hash.h"

#define CACHE_NAME "wsipchg_sp_hash_cache"

extern void get_random_bytes(void *, int);

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#pragma pack(2)

#define TIMEOUT_HASH_SYN	(6*HZ)
#define TIMEOUT_HASH_FIN	(6*HZ)
#define TIMEOUT_HASH_NOR	(5*60*HZ)

struct sp_tcp_hashinfo
{
	uint32_t seq;
	uint32_t ack_seq;
	uint32_t saddr;
	uint32_t daddr;
	uint16_t sport;
	uint16_t dport;
};

struct sp_tcp_hashinfo_item
{
	struct list_head c_list;
	struct sp_tcp_hashinfo tcp_info;
	atomic_t refcnt;
	struct timer_list timer; /* Expiration timer */

	unsigned long fakeip;
	unsigned long realip;
};

#pragma pack()

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

#endif // endif

/*VincentR2D2&liyi*/

