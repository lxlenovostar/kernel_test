#include <linux/percpu.h>

extern int ecryptfs_calculate_sha1(char *dst, char *src, int len);
extern void tcp_free_sha1sig_pool(void);
extern struct tcp_sha1sig_pool **tcp_alloc_sha1sig_pool(void);
extern int tcp_v4_sha1_hash_data(char *sha1_hash, char *src, int len);
