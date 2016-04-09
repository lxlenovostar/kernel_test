/*
ID:lxlenovos1
LANG:C
TASK:barn1
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

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
    struct section *sec;
	struct list_head *tmp_list;
    int max_gap;
    int distance;
    int begin, end;
    int last_max_gap = 1000;
    int last_begin = -1;

	INIT_LIST_HEAD(&cow_list);
	INIT_LIST_HEAD(&section_list);

	fin = fopen("barn1.in", "r");
	fout = fopen("barn1.out", "w");
	fscanf(fin, "%d %d %d", &board_num, &stall_num, &cow_num);

	for (i = 0; i < cow_num; ++i) {
		fscanf(fin, "%d", &num);
		ptr = (struct cow_home *) malloc(sizeof (struct cow_home));
		ptr->num = num;
		list_add_tail(&ptr->list, &cow_list);
	}

    for (i = 0; i < board_num; i++) {
        max_gap =  -1; 
	    list_for_each_entry(ptr, &cow_list, list) {
		    if ((&ptr->list)->next != &cow_list) {
			    at = list_entry((&ptr->list)->next, struct cow_home, list);
			    distance = at->num - ptr->num + 1;
                
                if (distance <= last_max_gap) {                
                    if (ptr->num == last_begin)
                        continue;

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
        last_begin = begin;
		list_add_tail(&sec->list, &section_list);
    }	

	list_for_each_entry(sec, &section_list, list) {
		printf("begin is:%d, end is:%d, dis is:%d\n", sec->begin, sec->end, sec->distance);
	}
	//fprintf(fout, "%d\n", total_price);
	exit(0);
}
