#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <asm/io.h>

MODULE_AUTHOR("author");
MODULE_DESCRIPTION("lx module\n");
MODULE_LICENSE("GPL");

void *mmap_buf = 0;
void *mmap_buf_pend = 0;
#define SLOT 4
unsigned long mmap_size = 4096 + 1024 * SLOT;

int
mmap_free(void)
{
	struct page *page;

	for (page = virt_to_page(mmap_buf);
	     page < virt_to_page(mmap_buf + mmap_size); page++) {
		ClearPageReserved(page);
	}

	kfree((void *) mmap_buf);
	mmap_buf = NULL;

	return 0;
}

int
mmap_alloc(void)
{
	struct page *page;
	mmap_size = PAGE_ALIGN(mmap_size);
	mmap_buf = kmalloc(mmap_size, GFP_ATOMIC);

	printk("kmalloc mmap_buf=%p, mmap_size=%ld\n",
	       (void *) mmap_buf, mmap_size);
	if (!mmap_buf) {
		printk("kmalloc failed!\n");
		return -1;
	}

	for (page = virt_to_page(mmap_buf);
	     page < virt_to_page(mmap_buf + mmap_size); page++) {
		SetPageReserved(page);
	}

	return 0;
}

static int
ws_mmap(struct file *f, struct vm_area_struct *vma)
{
	unsigned long pfn;
	unsigned long start = vma->vm_start;
	unsigned long size = PAGE_ALIGN(vma->vm_end - vma->vm_start);
	if (size > mmap_size || !mmap_buf) {
		return -EINVAL;
	}

	pfn = virt_to_phys(mmap_buf) >> PAGE_SHIFT;
	return remap_pfn_range(vma, start, pfn, size, PAGE_SHARED);
}

static const struct file_operations my_ops = {
	.owner = THIS_MODULE,
	.mmap = ws_mmap,
};

static struct miscdevice hello_dev = {
	MISC_DYNAMIC_MINOR,
	"eudyptula",
	&my_ops
};

static int __init
lx_init(void)
{
	int result, i;

	/*
	 *Create the "hello" device in the /sys/class/misc directory.
	 *Udev will automatically create the /dev/hello device using
	 *the default rules.
	 */
	result = misc_register(&hello_dev);
	if (result) {
		printk("Unable to register misc device\n");
		return result;
	}

	result = mmap_alloc();
	if (result) {
		printk("wsmmap: mmap alloc failed!\n");
		return result;
	}

	printk("wsmmap is insmoded!\n");

	/*
	 * just for test 
	 * **/
	for (i = 0; i < mmap_size; i += PAGE_SIZE) {
		memset(mmap_buf + i, 'a', PAGE_SIZE);
		memset(mmap_buf + i + PAGE_SIZE - 1, '\0', 1);
	}

	return result;
}

static void __exit
lx_exit(void)
{
	int result;

	misc_deregister(&hello_dev);

	result = mmap_free();
	if (result) {
		printk("wsmmap: mmap free failed!\n");
		goto free_failed;
	}
	printk("wsmmap: exit\n");
	return;

      free_failed:
	msleep(3000);
	mmap_free();
	printk("wsmmap: exit\n");
}

module_init(lx_init);
module_exit(lx_exit);
