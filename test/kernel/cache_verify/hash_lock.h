#include <linux/spinlock.h>

#define CT_LOCKARRAY_BITS  10
#define CT_LOCKARRAY_SIZE  (1<<CT_LOCKARRAY_BITS)
#define CT_LOCKARRAY_MASK  (CT_LOCKARRAY_SIZE-1)

struct _aligned_lock
{
	rwlock_t	l;
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

static inline void ct_read_lock(unsigned key, struct _aligned_lock *_lock_array)
{
	read_lock(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_read_unlock(unsigned key, struct _aligned_lock *_lock_array)
{
	read_unlock(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_write_lock(unsigned key, struct _aligned_lock *_lock_array)
{
	write_lock(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_write_unlock(unsigned key, struct _aligned_lock *_lock_array)
{
	write_unlock(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_read_lock_bh(unsigned key, struct _aligned_lock *_lock_array)
{
	read_lock_bh(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_read_unlock_bh(unsigned key, struct _aligned_lock *_lock_array)
{
	read_unlock_bh(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_write_lock_bh(unsigned key, struct _aligned_lock *_lock_array)
{
	write_lock_bh(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}
static inline void ct_write_unlock_bh(unsigned key, struct _aligned_lock *_lock_array)
{
	write_unlock_bh(&_lock_array[key&CT_LOCKARRAY_MASK].l);
}

//struct _aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];
//extern struct _aligned_lock *hash_lock_array;

//void initial_sp_hash_table_cache(void);
/*
extern struct _aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];
//static struct _aligned_lock hash_lock_array[CT_LOCKARRAY_SIZE];
*/

/*
void initial_sp_hash_table_cache(void) {
	int idx;

	 for (idx=0; idx<CT_LOCKARRAY_SIZE; idx++)
     	rwlock_init(&hash_lock_array[idx].l);
}*/
