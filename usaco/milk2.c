#include <stdio.h>
#include <stdlib.h>

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
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *    
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
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

struct attime
{
		int begin;
		int end;
		struct list_head list;
};

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

int 
main()
{
		int num, i,j, low, high, long_distance, intevel;
		struct attime *ptr;
		FILE *fin,*fout;
		fin=fopen("milk2.in","r");
		fout=fopen("milk2.out","w");
		
		scanf("%d", &num);

		for (i = 0; i < NUM; ++i)
			for (j = 0; j < 2; ++j){
				scanf("%d %d", &low, &high);
				ptr = (struct attime*)malloc(sizeof(struct attime));
				ptr->begin = low;
				ptr->end = high;
				INIT_LIST_HEAD(ptr->list);
			}

}
