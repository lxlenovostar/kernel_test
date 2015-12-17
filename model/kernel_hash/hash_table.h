#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <linux/spinlock.h>

#define CACHE_NAME "hash_cache"
#define SHA1SIZE 20

#define HASH_JEN_MIX(a,b,c)                                                      \
do {                                                                             \
  a -= b; a -= c; a ^= ( c >> 13 );                                              \
  b -= c; b -= a; b ^= ( a << 8 );                                               \
  c -= a; c -= b; c ^= ( b >> 13 );                                              \
  a -= b; a -= c; a ^= ( c >> 12 );                                              \
  b -= c; b -= a; b ^= ( a << 16 );                                              \
  c -= a; c -= b; c ^= ( b >> 5 );                                               \
  a -= b; a -= c; a ^= ( c >> 3 );                                               \
  b -= c; b -= a; b ^= ( a << 10 );                                              \
  c -= a; c -= b; c ^= ( b >> 15 );                                              \
} while (0)

#define HASH_JEN(key,keylen,num_bkts,hashv,bkt)                                  \
do {                                                                             \
  unsigned _hj_i,_hj_j,_hj_k;                                                    \
  unsigned const char *_hj_key=(unsigned const char*)(key);                      \
  hashv = 0xfeedbeefu;                                                           \
  _hj_i = _hj_j = 0x9e3779b9u;                                                   \
  _hj_k = (unsigned)(keylen);                                                    \
  while (_hj_k >= 12U) {                                                         \
    _hj_i +=    (_hj_key[0] + ( (unsigned)_hj_key[1] << 8 )                      \
        + ( (unsigned)_hj_key[2] << 16 )                                         \
        + ( (unsigned)_hj_key[3] << 24 ) );                                      \
    _hj_j +=    (_hj_key[4] + ( (unsigned)_hj_key[5] << 8 )                      \
        + ( (unsigned)_hj_key[6] << 16 )                                         \
        + ( (unsigned)_hj_key[7] << 24 ) );                                      \
    hashv += (_hj_key[8] + ( (unsigned)_hj_key[9] << 8 )                         \
        + ( (unsigned)_hj_key[10] << 16 )                                        \
        + ( (unsigned)_hj_key[11] << 24 ) );                                     \
                                                                                 \
     HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                          \
                                                                                 \
     _hj_key += 12;                                                              \
     _hj_k -= 12U;                                                               \
  }                                                                              \
  hashv += (unsigned)(keylen);                                                   \
  switch ( _hj_k ) {                                                             \
     case 11: hashv += ( (unsigned)_hj_key[10] << 24 ); /* FALLTHROUGH */        \
     case 10: hashv += ( (unsigned)_hj_key[9] << 16 );  /* FALLTHROUGH */        \
     case 9:  hashv += ( (unsigned)_hj_key[8] << 8 );   /* FALLTHROUGH */        \
     case 8:  _hj_j += ( (unsigned)_hj_key[7] << 24 );  /* FALLTHROUGH */        \
     case 7:  _hj_j += ( (unsigned)_hj_key[6] << 16 );  /* FALLTHROUGH */        \
     case 6:  _hj_j += ( (unsigned)_hj_key[5] << 8 );   /* FALLTHROUGH */        \
     case 5:  _hj_j += _hj_key[4];                      /* FALLTHROUGH */        \
     case 4:  _hj_i += ( (unsigned)_hj_key[3] << 24 );  /* FALLTHROUGH */        \
     case 3:  _hj_i += ( (unsigned)_hj_key[2] << 16 );  /* FALLTHROUGH */        \
     case 2:  _hj_i += ( (unsigned)_hj_key[1] << 8 );   /* FALLTHROUGH */        \
     case 1:  _hj_i += _hj_key[0];                                               \
  }                                                                              \
  HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                             \
  bkt = hashv & (num_bkts-1U);                                                   \
} while(0)

#define HASH_FCN HASH_JEN


struct hashinfo_item
{
	/*
	 * get_hash_item
	 */
	atomic_t refcnt;
	
	/*
	 * the other cpu maybe use it.
	 */
	atomic_t share_ref;	//maybe different data struct use this hashinfo_item.  

	/*
	 * hash_new_item and bucket_clear_item(write lock)
	 */  
	struct list_head c_list;
	int len;				//the length of data
	uint8_t sha1[SHA1SIZE];
	
	char *data;				//store data in memory 	
	
	/*
     * 0: kmalloc
     * 1: SLAB CHUNKSTEP
	 * 2: SLAB CHUNKSTEP*2
     * 3: SLAB CHUNKSTEP*3 
     */
	atomic_t mem_style;         
	spinlock_t data_lock;

	/*
	 * wr_file update it.
	 */
	int cpuid;				//store file: newfile0
	unsigned long start;	//the start position in bitmap

	/*
	 * state machine
	 * 0: just in memory, and will move it.
     * 1: just in disk.
     * 2: in memory, will write to disk. 
	 * 3: always in memory.
	 * 4: read from file, so in memory and file.
     */
	atomic_t flag_cache; 	
	
	/*
	 * 0: data not in memory.
	 * 1: data in memory.
	 */
	atomic_t flag_mem;

	/* 
	 * just for statistics
	 * 0:the initial state.
	 * 1:data should write file.
	 * 2:data will write to file.
	 */
	int store_flag;
};

struct hashtable_del
{
	struct timer_list flush_timer; /*not release item immediatly, because we hold the lock now.*/
	struct list_head list;
};

void alloc_data_memory(struct hashinfo_item *cp, size_t length);
int initial_hash_table_cache(void);
void release_hash_table_cache(void);   
struct hashinfo_item *get_hash_item(uint8_t *info);
int add_hash_info(uint8_t *info, char *value, size_t len_value);
#endif 
