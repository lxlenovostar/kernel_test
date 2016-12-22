#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/vmalloc.h>

#define MAX_LIST_LENGTH_BITS 20

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
 
         head->prev->next = NULL; //双向链表变为单向链表   
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
                         if (unlikely(lev >= ARRAY_SIZE(part)-1)) {
                                 printk_once(KERN_DEBUG "list passed to"
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

struct hashinfo_item
{
	/*
     * hash_new_item and bucket_clear_item(write lock)
     */
     struct list_head c_list;
	 unsigned long start;    //the start position in bitmap
};

/**
 * file_poisit_compare
 * @priv: unused
 * @lh_a: list_head for first item
 * @lh_b: list_head for second item
 */
static int file_poisit_compare(void *priv, struct list_head *lh_a, struct list_head *lh_b)
{
        struct hashinfo_item *a = list_entry(lh_a, struct hashinfo_item, c_list);
        struct hashinfo_item *b = list_entry(lh_b, struct hashinfo_item, c_list);

		return a->start - b->start;
}

static int minit(void)
{
	struct hashinfo_item *cp, *next;
	printk(KERN_INFO "Start %s", THIS_MODULE->name);

	LIST_HEAD(head_list);

	struct hashinfo_item *one = kmalloc(sizeof(struct hashinfo_item), GFP_ATOMIC);
	INIT_LIST_HEAD(&one->c_list);
	one->start = 7;
	list_add(&one->c_list, &head_list);
	
	struct hashinfo_item *two = kmalloc(sizeof(struct hashinfo_item), GFP_ATOMIC);
	INIT_LIST_HEAD(&two->c_list);
	two->start = 2;
	list_add(&two->c_list, &head_list);
	
	struct hashinfo_item *three = kmalloc(sizeof(struct hashinfo_item), GFP_ATOMIC);
	INIT_LIST_HEAD(&three->c_list);
	three->start = 9;
	list_add(&three->c_list, &head_list);

    list_for_each_entry_safe(cp, next, &head_list, c_list) {
		printk(KERN_INFO "start is:%lu", cp->start);
	}
    
	list_sort(NULL, &head_list, file_poisit_compare);
	printk(KERN_INFO "");
	
	list_for_each_entry_safe(cp, next, &head_list, c_list) {
		printk(KERN_INFO "start is:%lu", cp->start);
	}


	kfree(one);
	kfree(two);
	kfree(three);

	return 0;
}

static void mexit(void)
{
	printk("Exit %s.\n", THIS_MODULE->name);
}

module_init(minit);
module_exit(mexit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("lix");
#ifdef DEBUG
MODULE_VERSION("1.4.1.debug");
#else
MODULE_VERSION("1.4.1");
#endif
