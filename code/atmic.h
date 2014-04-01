  /*
   * Atomic operations that C can't guarantee us.  Useful for
   * resource counting etc..
   */
  
#ifdef CONFIG_SMP
#define LOCK "lock ; "
#else
#define LOCK ""
#endif

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
typedef struct { volatile int counter; } atomic_t;
  

 /**
   * atomic_set - set atomic variable
   * @v: pointer of type atomic_t
   * @i: required value
   * 
   * Atomically sets the value of @v to @i.
   */ 
 #define atomic_set(v,i)         (((v)->counter) = (i))
 

/**
 * atomic_add_return - add and return
 * @i: integer value to add
 * @v: pointer of type atomic_t
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static __inline__ int atomic_add_return(int i, atomic_t *v)
{
         int __i = i;
         __asm__ __volatile__(
                 LOCK "xaddl %0, %1;"
                 :"=r"(i)
                 :"m"(v->counter), ""(i));
         return i + __i;
}

/**
  * atomic_dec - decrement atomic variable
  * @v: pointer of type atomic_t
  * 
  * Atomically decrements @v by 1.
  */ 
static __inline__ void atomic_dec(atomic_t *v)
{
         __asm__ __volatile__(
                 LOCK "decl %0"
                 :"=m" (v->counter)
                 :"m" (v->counter));
}

/**
  * atomic_inc - increment atomic variable
  * @v: pointer of type atomic_t
  * 
  * Atomically increments @v by 1.
  */ 
static __inline__ void atomic_inc(atomic_t *v)
{
         __asm__ __volatile__(
                 LOCK "incl %0"
                 :"=m" (v->counter)
                 :"m" (v->counter));
}


 
