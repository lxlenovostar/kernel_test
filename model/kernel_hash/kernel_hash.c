#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
MODULE_LICENSE("Dual BSD/GPL");

#define SHA1_LENGTH     20

static int hello_init(void)
{
    struct scatterlist sg;
    struct crypto_hash *tfm;
    struct hash_desc desc;
    unsigned char output[SHA1_LENGTH];
    unsigned char buf = "plaintext goes here";
    int len = strlen(buf);    
    int i;

    printk(KERN_INFO "sha1: %s\n", __FUNCTION__);

    memset(buf, 'A', 10);
    memset(output, 0x00, SHA1_LENGTH);

    tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);

    desc.tfm = tfm;
    desc.flags = 0;

    sg_init_one(&sg, buf, len);
    crypto_hash_init(&desc);

    crypto_hash_update(&desc, &sg, len);
    crypto_hash_final(&desc, output);

    for (i = 0; i < 20; i++) {
        printk(KERN_DEBUG "%02x:", output[i]&0xff);
    }

    crypto_free_hash(tfm);

    /*
    struct scatterlist sg;
    struct hash_desc desc;
    char *plaintext = "laintext goes here";    
    size_t len = strlen(plaintext);
    unsigned char output[SHA1_LENGTH];
    int i;

    printk(KERN_ALERT "Hello, world\n");
    printk(KERN_INFO "sha1: %s\n", __FUNCTION__);

    sg_init_one(&sg, plaintext, len);
    desc.tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);

    crypto_hash_init(&desc);
    crypto_hash_update(&desc, &sg, len);
    crypto_hash_final(&desc, output);
    crypto_free_hash(desc.tfm);

    for (i = 0; i < 20; i++) {
        printk("%02x:", output[i]&0xff);
    }
    printk(KERN_ERR "\n");
    
    memset(buf, 'A', 10);
    memset(output, 0x00, SHA1_LENGTH);


    tfm = crypto_alloc_hash("sha1", 0, CRYPTO_ALG_ASYNC);

    desc.tfm = tfm;
    desc.flags = 0;

    sg_init_one(&sg, buf, 10);
    crypto_hash_init(&desc);
    crypto_hash_update(&desc, &sg, 10);
    crypto_hash_final(&desc, output);

    for (i = 0; i < 20; i++) {
        printk(KERN_ERR "%d-%d\n", output[i], i);
    }

    crypto_free_hash(tfm);
    */
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);
