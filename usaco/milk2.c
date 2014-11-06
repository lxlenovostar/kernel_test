/*
ID:lxlenovos1
LANG:C
TASK:milk2
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

struct attime
{
		int begin;
		int end;
		struct list_head list;
};

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

struct list_head list;

int 
main()
{
		int num, i,j, low, high;
		struct attime *ptr;
		struct attime *at;
		FILE *fin,*fout;
		struct list_head *tmp_list;
		int long_distance = 0;
		int  intevel = 0;
		int last_begin = 0;
		int last_end = 0;
		int prev_end = 0;
		fin=fopen("milk2.in","r");
		fout=fopen("milk2.out","w");
		
		INIT_LIST_HEAD(&list);
		//scanf("%d", &num);
		fscanf(fin, "%d", &num);
		//printf("what1\n");
		for (i = 0; i < num; ++i){
			//scanf("%d %d", &low, &high);
			fscanf(fin, "%d %d", &low, &high);
			ptr = (struct attime*)malloc(sizeof(struct attime));
			ptr->begin = low;
			ptr->end = high;

			//printf("what2\n");
			tmp_list = (&list)->next;
			do {
				if (list_empty(&list)){
					//printf("what3\n");
					list_add(&ptr->list, &list);
				}
				else{
					//printf("what3.5\n");
					at = list_entry(tmp_list, struct attime, list);
					
					if (ptr->begin <= at->begin){
						//printf("what3.6\n");
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
					
					if (tmp_list->next == &list){
						//printf("what3.7\n");
						list_add(&ptr->list, tmp_list);
						break;	
					}
					tmp_list = tmp_list->next;
						
				}
			} while(tmp_list != &list);
			//printf("what4\n");
		}
		/*for (tmp_list = (&list)->next; tmp_list != &list; tmp_list = tmp_list->next)
		{
			t = list_entry(tmp_list, struct attime, list);
			printf("begin is %d and end is %d\n", t->begin, t->end);
		}*/

		for (tmp_list = (&list)->next; tmp_list != &list; tmp_list = tmp_list->next)
		{
			at = list_entry(tmp_list, struct attime, list);
			//printf("begin is %d and end is %d; last_begin is %d and last_end is %d\n", at->begin, at->end, last_begin, last_end);
			if (tmp_list->prev == &list){
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
			//last_begin = t->begin;
			//last_end = t->end;
			/*printf("tmp_long is %d, tmp_intevel is %d\n", long_distance, intevel);
			if (at->end - at->begin >= 335){
			<F4>	printf("end - begin is %d\n", (at->end - at->begin));
				break;
			}*/

		}
		//printf("long is %d, intevel is %d\n", long_distance, intevel);
		fprintf(fout, "%d %d\n", long_distance, intevel);
		exit (0);
}
