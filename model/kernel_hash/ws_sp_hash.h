/*VincentR2D2&liyi*/

#ifndef _WS_SP_HASH_H_
#define _WS_SP_HASH_H_

#include <linux/spinlock.h>
#include <linux/jhash.h>


//#define DEBUG_FLAG_FUN
//#define DEBUG_FLAG_INFO
//#define DEBUG_FLAG_ERROR

//////////////////////////////////////////////////////////////////////////
#if defined (DEBUG_FLAG_FUN)
#define  DEBUG_FUNC(format, args...)	printk(format , ## args)
#else
#define   DEBUG_FUNC(format, args...)
#endif


#if defined (DEBUG_FLAG_INFO)
#define  DEBUG_INFO(format, args...)	printk(format , ## args)
#else
#define   DEBUG_INFO(format, args...)
#endif

//////////////////////////////////////////////////////////////////////////
#if defined (DEBUG_FLAG_ERROR)
#define DEBUG_ERROR(format, args...)  printk(format , ## args)
#else
#define  DEBUG_ERROR(format, args...)
#endif

/*
 *  Fine locking granularity for big connection hash table
 */
#define CT_LOCKARRAY_BITS  10
#define CT_LOCKARRAY_SIZE  (1<<CT_LOCKARRAY_BITS)
#define CT_LOCKARRAY_MASK  (CT_LOCKARRAY_SIZE-1)

struct ws_sp_aligned_lock
{
	rwlock_t	l;
} __attribute__((__aligned__(SMP_CACHE_BYTES)));

static inline void ct_read_lock(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	read_lock(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_read_unlock(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	read_unlock(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_write_lock(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	write_lock(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_write_unlock(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	write_unlock(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_read_lock_bh(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	read_lock_bh(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_read_unlock_bh(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	read_unlock_bh(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}

static inline void ct_write_lock_bh(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	write_lock_bh(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}
static inline void ct_write_unlock_bh(unsigned key, struct ws_sp_aligned_lock *__ws_lvs_conntbl_lock_array)
{
	write_unlock_bh(&__ws_lvs_conntbl_lock_array[key&CT_LOCKARRAY_MASK].l);
}

#endif
/*VincentR2D2&liyi*/


