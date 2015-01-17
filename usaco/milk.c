/*
ID:lxlenovos1
LANG:C
TASK:milk
*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

struct list_head {
	struct list_head *next, *prev;
};

static inline void INIT_LIST_HEAD(struct list_head *list)
{   
		list->next = list;
		list->prev = list;
}

static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
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

/**
 *  * list_for_each_entry  -   iterate over list of given type
 *   * @pos:    the type * to use as a loop cursor.
 *    * @head:   the head for your list.
 *     * @member: the name of the list_struct within the struct.
 *      */
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
static inline void __list_del(struct list_head * prev, struct list_head * next)
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

struct supply
{
		int price;
		int num;
};



struct list_head head_list; 
int 
main()
{
		FILE *fin,*fout;
		int milk_num, farmer_num, i, price, num;
		struct supply *ptr;
		struct supply *at;
		struct list_head *tmp_list;

		fin = fopen("milk.in","r");
		fout = fopen("milk.out","w");
		
		INIT_LIST_HEAD(&head_list);
		fscanf(fin, "%d %d", &milk_num, &farmer_num);

		for (i = 0; i < farmer_num; ++i){
			fscanf(fin, "%d %d", &price, &num);
			ptr = (struct suppy*)malloc(sizeof(struct suppy));
			ptr->price = price;
			ptr->num = num;

			/*
			 *
			 * 这插入排序写的太难懂了
			 * */
			tmp_list = (&head_list)->next;
			do {
				if (list_empty(&head_list)){
					list_add(&ptr->list, &head_list);
				}
				else{
					at = list_entry(tmp_list, struct supply, list);
					
					if (ptr->price <= at->prince){
						if (ptr->end < at->begin){
							list_add(&ptr->list, tmp_list->prev);
						}
						else if (ptr->end >= at->begin && ptr->end < at->end)
						{
								at->begin = ptr->begin;
						}
						else
						{
								at->begin = ptr->begin;
								at->end = ptr->end;
						}	
						break;	
					}
					
					if (tmp_list->next == &head_list){
						list_add(&ptr->list, tmp_list);
						break;	
					}
					tmp_list = tmp_list->next;
						
				}
			} while(tmp_list != &head_list);
		}
		/*
		list_for_each_entry(at, &head_list, list){
			printf("begin is %d and end is %d\n", at->begin, at->end);
			printf("end1\n");
		}
		*/	
		list_for_each_entry_safe(at, tmp_at, &head_list, list){
				if (&tmp_at->list == &head_list)
						break;
				if (at->begin <= tmp_at->begin && at->end >= tmp_at->begin && at->end <= tmp_at->end){
						tmp_at->begin = at->begin;
						list_del(&at->list);
						free(at);
						continue;
				}
				
				if (at->begin <= tmp_at->begin && at->end > tmp_at->end){
						tmp_at->end = at->end;
						tmp_at->begin = at->begin;
						list_del(&at->list);
						free(at);
						continue;
				}
		}
		
		for (tmp_list = (&head_list)->next; tmp_list != &head_list; tmp_list = tmp_list->next){
			at = list_entry(tmp_list, struct attime, list);
			if (tmp_list->prev == &head_list){
				long_distance  = at->end - at->begin;
				intevel = 0;
				last_begin = at->begin;
				last_end = at->end;
			}
			else {
				if (at->begin >= last_begin && at->end <= last_end){
					continue;
                }
								
				if (at->begin == last_begin){
					if (at->end > last_end){
						long_distance  = at->end - at->begin;
						last_end = at->end;
					}
				} else if (at->begin > last_begin && at->begin <= last_end){
					if (at->end > last_end){
						long_distance += at->end - last_end;
						last_end = at->end;
					}
				} else {
					if ((at->begin - prev_end) > intevel){
						intevel = at->begin - prev_end;
					}
					if (long_distance <= (at->end - at->begin)){
						long_distance = at->end - at->begin;
						last_begin = at->begin;
						last_end = at->end;
					}
				}
			}	
		
			prev_end = at->end;	

		}
		//printf("long is %d, intevel is %d\n", long_distance, intevel);
		fprintf(fout, "%d %d\n", long_distance, intevel);
		exit (0);
}
