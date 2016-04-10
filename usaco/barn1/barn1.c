/*
ID:lxlenovos1
LANG:C
TASK:barn1
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

struct list_head {
	struct list_head *next, *prev;
};

static inline void
INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(struct list_head *new, struct list_head *prev,
	   struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/*
 *  list_add - add a new entry
 *  @new: new entry to be added
 *  @head: list head to add it after
 *      
 *  Insert a new entry after the specified head.
 *  This is good for implementing stacks.
 */
static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

/*
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

#define container_of(ptr, type, member) ({                      \
         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
         (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member) \
         container_of(ptr, type, member)

/*
 * list_for_each_entry  -   iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:   the head for your list.
 * @member: the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)              \
	for (pos = list_entry((head)->next, typeof(*pos), member);  \
		&pos->member != (head);    \
		pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 *list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 *@pos:    the type * to use as a loop cursor.
 *@n:      another type * to use as temporary storage
 *@head:   the head for your list.
 *@member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)          \
	for (pos = list_entry((head)->next, typeof(*pos), member),  \
		n = list_entry(pos->member.next, typeof(*pos), member); \
		&pos->member != (head);                    \
		pos = n, n = list_entry(n->member.next, typeof(*n), member))

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

# define POISON_POINTER_DELTA 0

/*
 *These are non-NULL pointers that will result in page faults
 *under normal circumstances, used to verify that nobody uses
 *non-initialized list entries.
 */
#define LIST_POISON1  ((void *) 0x00100100 + POISON_POINTER_DELTA)
#define LIST_POISON2  ((void *) 0x00200200 + POISON_POINTER_DELTA)

/*
 *Delete a list entry by making the prev/next entries
 *point to each other.
 *
 *This is only for internal list manipulation where we know
 *the prev/next entries already!
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

#define MAX_LIST_LENGTH_BITS 20
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
/*
  * Returns a list organized in an intermediate format suited
  * to chaining of merge() calls: null-terminated, no reserved or
  * sentinel head node, "prev" links not maintained.
  */
static struct list_head *merge(void *priv,
                                 int (*cmp)(void *priv, struct list_head *a,
                                         struct list_head *b),
                                 struct list_head *a, struct list_head *b)
{
    struct list_head head, *tail = &head;
 
    while (a && b) {
        /* if equal, take 'a' -- important for sort stability */
        if ((*cmp)(priv, a, b) <= 0) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
         tail->next = a?:b;
         return head.next;
}

/*
 * Combine final list merge with restoration of standard doubly-linked
 * list structure.  This approach duplicates code from merge(), but
 * runs faster than the tidier alternatives of either a separate final
 * prev-link restoration pass, or maintaining the prev links
 * throughout.
 */
static void merge_and_restore_back_links(void *priv,
                                 int (*cmp)(void *priv, struct list_head *a,
                                        struct list_head *b),
                                 struct list_head *head,
                                 struct list_head *a, struct list_head *b)
{
        struct list_head *tail = head;

        while (a && b) {
                /* if equal, take 'a' -- important for sort stability */
                 if ((*cmp)(priv, a, b) <= 0) {
                         tail->next = a;
                         a->prev = tail;
                         a = a->next;
                 } else {
                         tail->next = b;
                         b->prev = tail;
                         b = b->next;
                 }
                 tail = tail->next;
         }
         tail->next = a ? : b;
 
         do {
                 /*
                  * In worst cases this loop may run many iterations.
                  * Continue callbacks to the client even though no
                  * element comparison is needed, so the client's cmp()
                  * routine can invoke cond_resched() periodically.
                  */
                 (*cmp)(priv, tail->next, tail->next);
 
                 tail->next->prev = tail;
                 tail = tail->next;
         } while (tail->next);
 
         tail->next = head;
         head->prev = tail;
}

/**
  * list_sort - sort a list
  * @priv: private data, opaque to list_sort(), passed to @cmp
  * @head: the list to sort
  * @cmp: the elements comparison function
  *
  * This function implements "merge sort", which has O(nlog(n))
  * complexity.
  *
  * The comparison function @cmp must return a negative value if @a
  * should sort before @b, and a positive value if @a should sort after
  * @b. If @a and @b are equivalent, and their original relative
  * ordering is to be preserved, @cmp must return 0.
  */
void list_sort(void *priv, struct list_head *head,
                  int (*cmp)(void *priv, struct list_head *a,
                          struct list_head *b))
{
         struct list_head *part[MAX_LIST_LENGTH_BITS+1]; /* sorted partial lists
                                                 -- last slot is a sentinel */
         int lev;  /* index into part[] */
         int max_lev = 0;
         struct list_head *list;
 
         if (list_empty(head))
                 return;
 
         memset(part, 0, sizeof(part));
 
         head->prev->next = NULL;
         list = head->next;
 
         while (list) {
                 struct list_head *cur = list;
                 list = list->next;
                 cur->next = NULL;
 
                 for (lev = 0; part[lev]; lev++) {
                         cur = merge(priv, cmp, part[lev], cur);
                         part[lev] = NULL;
                 }
                 if (lev > max_lev) {
                         if (lev >= ARRAY_SIZE(part)-1) {
                                 printf("list passed to"
                                         " list_sort() too long for"
                                         " efficiency\n");
                                 lev--;
                         }
                         max_lev = lev;
                 }
                 part[lev] = cur;
         }
 
         for (lev = 0; lev < max_lev; lev++)
                 if (part[lev])
                         list = merge(priv, cmp, part[lev], list);
 
         merge_and_restore_back_links(priv, cmp, head, part[max_lev], list);
}

struct cow_home {
	int num;
	struct list_head list;
};

//分配board_num个section 存放切割木板的位置。 
struct section {
    int begin;
    int end;
    int distance;
	struct list_head list;
};

struct list_head cow_list;
struct list_head section_list;

/*
 * section_compare
 * @priv: unused
 * @lh_a: list_head for first item
 * @lh_b: list_head for second item
 */
static int section_compare(void *priv, struct list_head *lh_a, struct list_head *lh_b)
{
        struct section *a = list_entry(lh_a, struct section, list);
        struct section *b = list_entry(lh_b, struct section, list);

        return a->begin - b->begin;
}

static int cow_compare(void *priv, struct list_head *lh_a, struct list_head *lh_b)
{
        struct cow_home *a = list_entry(lh_a, struct cow_home, list);
        struct cow_home *b = list_entry(lh_b, struct cow_home, list);

        return a->num - b->num;
}

void free_cow_list(struct list_head *cow_head)
{
	struct cow_home *ptr, *cow_next;

	list_for_each_entry_safe(ptr, cow_next, cow_head, list) { 
        list_del(&ptr->list);
        free(ptr); 
    }
}

void free_section_list(struct list_head *sec_head)
{
	struct section *ptr, *sec_next;

	list_for_each_entry_safe(ptr, sec_next, sec_head, list) { 
        list_del(&ptr->list);
        free(ptr); 
    }
}

int check_used(const int * used_index, const int length, int check_value) {
    int i;

    for (i = 0; i < length; ++i) {
        if (check_value == used_index[i])
            return 1;
    } 

    return 0;
}

/*
 *@board_num ：the maximum number of boards that can be purchased   (1 <= board_num <= 50)
 *@stall_num ：the total number of stalls   (1 <= stall_num <= 200)
 *@cow_num ：the number of cows in the stalls.  (1 <= cow_num <= stall_num)
 */
int main()
{
	FILE *fin, *fout;
	int i, board_num, stall_num, cow_num, num;
	struct cow_home *ptr;
	struct cow_home *at;
    struct section *sec, *sec_next;
	struct list_head *tmp_list;
    int max_gap;
    int distance;
    int begin, end;
    int last_max_gap = 1000;
    int last_begin = -1;
    int stall_begin = 0;
    int stall_end = 0;
    int result = 0;
    int used_index[50]; 
    size_t index = 0;

    memset(used_index, -1, ARRAY_SIZE(used_index));
	
    INIT_LIST_HEAD(&cow_list);
	INIT_LIST_HEAD(&section_list);

	fin = fopen("barn1.in", "r");
	fout = fopen("barn1.out", "w");
	fscanf(fin, "%d %d %d", &board_num, &stall_num, &cow_num);

	for (i = 0; i < cow_num; ++i) {
		fscanf(fin, "%d", &num);
		ptr = (struct cow_home *)malloc(sizeof (struct cow_home));
		ptr->num = num;
		list_add_tail(&ptr->list, &cow_list);
	}

    if (board_num >= cow_num) {
        result += cow_num;
	    fprintf(fout, "%d\n", result);
        //just debug
        //printf("result is:%d\n", result);
        free_cow_list(&cow_list);
	    exit(0);
    }

    /* cow section. */
    list_sort(NULL, &cow_list, cow_compare);

	list_for_each_entry(ptr, &cow_list, list) { 
         if ((&ptr->list)->prev == &cow_list && (&ptr->list)->next == &cow_list) { /* only one node */
            stall_begin = ptr->num;
            stall_end = ptr->num;
        } else if ((&ptr->list)->prev == &cow_list) { /* head */
            stall_begin = ptr->num;
        } else if ((&ptr->list)->next == &cow_list) { /* tail */
            stall_end = ptr->num;
        }
        else {
            //do nothing.
        }
        
        //just debug
        //printf("cow num is:%d\n", ptr->num);
    }

    if (board_num == 1) {
        result += stall_end - stall_begin + 1;
	    fprintf(fout, "%d\n", result);
        //just debug
        //printf("result is:%d\n", result);
        free_cow_list(&cow_list);
	    exit(0);
    }
    
    for (i = 0; i < (board_num - 1); i++) {
        max_gap = -1; 
	    list_for_each_entry(ptr, &cow_list, list) {
		    if ((&ptr->list)->next != &cow_list) {
			    at = list_entry((&ptr->list)->next, struct cow_home, list);
			    distance = at->num - ptr->num + 1;
                
                if (distance <= last_max_gap) {                
                    //just debug 
                    //printf("last_begin is:%d", last_begin);
                    if (check_used(used_index, index, ptr->num)) {
                        printf("last_begin is:%d and num is:%d, last_max_gap is:%d\n", last_begin, ptr->num, last_max_gap);
                        continue;
                    }

                    if (distance >= max_gap) {
                        max_gap = distance; 
                        begin = ptr->num;
                        end = at->num;
                    }
                }
		    }
	    }
        
        sec = (struct section*) malloc(sizeof (struct section));
        sec->distance = max_gap;
        sec->begin = begin;
        sec->end = end;
        last_max_gap = max_gap;
        used_index[index] = begin;        
        ++index;

        //just debug
        //printf("result last_begin is:%d, end is:%d, last_max_gap is:%d\n", last_begin, end, last_max_gap);
		list_add_tail(&sec->list, &section_list);
    }	

    /* sort section. */
    list_sort(NULL, &section_list, section_compare);
	
    list_for_each_entry(sec, &section_list, list) {
        if ((&sec->list)->prev == &section_list && (&sec->list)->next == &section_list) { /* only one node */
            result += sec->begin - stall_begin + 1;  
            result += stall_end - sec->end + 1;
        } else if ((&sec->list)->prev == &section_list) { /* head */
            result += sec->begin - stall_begin + 1;  
            last_begin = sec->end;
        } else if ((&sec->list)->next == &section_list) { /* tail */
            result += sec->begin - last_begin + 1;
            result += stall_end - sec->end + 1;
        }
        else {
            result += sec->begin - last_begin + 1;
            last_begin = sec->end;
        }
        //just debug
		//printf("begin is:%d, end is:%d, dis is:%d\n", sec->begin, sec->end, sec->distance);
	}

	fprintf(fout, "%d\n", result);
    //just debug
    //printf("result is:%d\n", result);
    free_cow_list(&cow_list);
    free_section_list(&section_list);
	exit(0);
}
