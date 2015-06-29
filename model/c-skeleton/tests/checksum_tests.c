#include "minunit.h"
#include "../src/lcthw/checksum.h"
#include <string.h>

char *
test_xor_checksum()
{
		char *Buff0 = "GPGGA,204502.00,5106.9813,N,11402.2921,W,1,09,0.9,1065.02,M,-16.27,M,,";
		int len0 = strlen(Buff0);
		char *Buff1 = "1GPGGA,204502.00,5106.9813,N,11402.2921,W,1,09,0.9,1065.02,M,-16.27,M,,";
		int len1 = strlen(Buff1);
		char *Buff2 = "00";
		int len2 = strlen(Buff2);
		char *Buff3 = "100";
		int len3 = strlen(Buff3);
		char *Buff4 = "010";
		int len4 = strlen(Buff4);

		mu_assert(xor_checksum(Buff0, len0) == 0x6f, "test0: error hint.");
		mu_assert(xor_checksum(Buff1, len1) == 0x5e, "test1: error hint.");
		mu_assert(xor_checksum(Buff2, len2) == 0x0, "test2: error hint.");
		mu_assert(xor_checksum(Buff3, len3) == 0x31, "test3: error hint.");
		mu_assert(xor_checksum(Buff4, len4) == 0x31, "test3: error hint.");

		return NULL;
}

char *
all_tests()
{
	mu_suite_start();

	mu_run_test(test_xor_checksum);

	return NULL;
}

RUN_TESTS(all_tests);
