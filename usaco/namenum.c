/*
 ID:lxlenovos1
 LANG:C
 TASK:namenum
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define NUM 13
struct list_head {
	struct list_head *next, *prev;
};

static inline void
INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline void
__list_add(struct list_head *new, struct list_head *prev,
	   struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void
list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

#define container_of(ptr, type, member) ({                      \
				         const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
				         (type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member) \
		         container_of(ptr, type, member)

#define list_for_each_entry_safe(pos, n, head, member)          \
		    for (pos = list_entry((head)->next, typeof(*pos), member),  \
							        n = list_entry(pos->member.next, typeof(*pos), member); \
							        &pos->member != (head);                    \
							        pos = n, n = list_entry(n->member.next, typeof(*n), member))

#define POISON_POINTER_DELTA 0
#define LIST_POISON1  ((void *) 0x00100100 + POISON_POINTER_DELTA)
#define LIST_POISON2  ((void *) 0x00200200 + POISON_POINTER_DELTA)

static inline void
__list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void
list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

#define list_for_each_entry_safe_reverse(pos, n, head, member)      \
		    for (pos = list_entry((head)->prev, typeof(*pos), member),  \
		n = list_entry(pos->member.prev, typeof(*pos), member);      \
		&pos->member != (head);                                       \
		pos = n, n = list_entry(n->member.prev, typeof(*n), member))

struct inode
{
	char name[NUM];
	struct list_head list;
};

char value[12];
void get_num(char *tmp) {
	int i, j;
	/*
	 * 注意这里是列是4，因为包括一个'\0'
	 */
	char keyvalue[8][4] = {
	"ABC", "DEF", "GHI", "JKL", "MNO", "PRS", "TUV", "WXY"};

	for (i = 0; i < strlen(tmp); ++i) {
		for (j = 0; j < 8; ++j) {
			if (strchr
			    ((char const *) (keyvalue + j),
			     (int) (*(tmp + i))) != NULL) {
				value[i] = j + 2 + '0';
				break;
			}
		}
	}
	//printf("value is %s\n", value);
	/*
	   for(i = strlen(tmp) - 1; i >= 0; --i){
	   //printf("index is %d and i is %d\n", index[i], i);
	   num += index[i]*base;
	   base *= 10;
	   //printf("num is %d\n", num);
	   } */

	//printf("index is %\n", index);

	/*
	 *  也许给的数字不对，其中1，2个数字不对，先不考虑
	 * **/
	//num = atoi(index);
	//printf("num is %d\n", num);
}

int
 main() {
	FILE *fin, *fout, *fdict;
	int i, len;
	char buffer[NUM];
	char num[NUM];
	char temp[NUM];
	struct list_head head_list[NUM];
	struct inode *at;
	struct inode *tmp;
	int index;
	char result[3 * NUM][NUM];
	int j = 0;
	
	for (i = 0; i < NUM; ++i)
		INIT_LIST_HEAD(&head_list[i]);

	fin = fopen("namenum.in", "r");
	fout = fopen("namenum.out", "w");
	fdict = fopen("dict.txt", "r");

	fscanf(fin, "%s", (char *) &num);
	while (fgets(buffer, NUM, fdict) != NULL) {
		if (sscanf(buffer, "%s", temp) != 1) {
			continue;
			printf("error\n");
		}
		//printf("%s, %d and %d\n", temp, sizeof(temp), strlen(temp));
		len = (int) strlen(temp) - 1;
		struct inode *ptr =
		    (struct inode *) malloc(sizeof (struct inode));
		strcpy(ptr->name, temp);
		list_add(&ptr->list, &head_list[len]);
	}

	index = strlen(num) - 1;

	//printf("num is %s\n", num);   
	//printf("index is %d\n", index);

	list_for_each_entry_safe_reverse(at, tmp, &head_list[index], list) {
		get_num(at->name);
		//printf("value is %s, num is %s, result is %d\n", value, num, strcmp(value, num));
		if (strcmp(value, num) == 0) {
			strcpy((char *) (result + j), at->name);
			++j;
		}
	}

	for (i = 0; i < NUM; ++i) {
		list_for_each_entry_safe(at, tmp, &head_list[i], list) {
			//printf("%s, %d\n", at->name, (int)strlen(at->name));
			list_del(&at->list);
			free(at);
		}
		//printf("next %d\n", i);
	}

	//printf("%d\n", j);
	if (j == 0) {
		fprintf(fout, "%s\n", "NONE");
	} else {
		for (i = 0; i < j; ++i) {
			fprintf(fout, "%s\n", (char *) (result + i));
		}
	}

	fclose(fin);
	fclose(fout);
	fclose(fdict);
	exit(0);
}
