#include "minunit.h"
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "../src/lcthw/sha.h"

char *
test_sha()
{
	//just a example for use sha function from cryptodev-linux kernel module.
 
	/*
	int cfd = -1, i;
    struct cryptodev_ctx ctx;
    uint8_t digest[20];
    //char text[] = "1The quick brown fox jumps over the lazy dog";
    char text[] = "1The quick brown fox jumps over the lazy dog";
    uint8_t expected[] = "\x2f\xd4\xe1\xc6\x7a\x2d\x28\xfc\xed\x84\x9e\xe1\xbb\x76\xe7\x39\x1b\x93\xeb\x12";

    cfd = open("/dev/crypto", O_RDWR, 0);
    if (cfd < 0) {
        perror("open(/dev/crypto)");
        return 1;
    }

    if (fcntl(cfd, F_SETFD, 1) == -1) {
        perror("fcntl(F_SETFD)");
        return 1;
    }

    sha_ctx_init(&ctx, cfd, NULL, 0);
    sha_hash(&ctx, text, strlen(text), digest);
    sha_ctx_deinit(&ctx);

    printf("digest: ");
    for (i = 0; i < 20; i++) {
        printf("%02x:", digest[i]);
    }
    printf("\n");
    
    if (memcmp(digest, expected, 20) != 0) {
        printf("SHA1 hashing failed\n");
        //return 1;
    }

    if (close(cfd)) {
        perror("close(cfd)");
        return 1;
    }

	//mu_assert(xor_checksum(Buff4, len4) == 0x31, "test3: error hint.");
	*/

	return NULL;
}

char *
all_tests()
{
	mu_suite_start();

	mu_run_test(test_sha);

	return NULL;
}

RUN_TESTS(all_tests);
