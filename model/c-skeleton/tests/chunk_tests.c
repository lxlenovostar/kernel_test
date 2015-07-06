#include "minunit.h"
#include <string.h>
#include "../src/lcthw/chunk.h"
#include "../src/lcthw/dbg.h"

static chunk *ck = NULL;

char *
test_create()
{
	ck = chunk_create();
	mu_assert(ck != NULL, "chunk_create failed.");
	mu_assert(ck->end == 0, "max are wrong in chunk");

	return NULL;
}

char *
test_destroy()
{
	chunk_destroy(ck);
	return NULL;
}

char *
test_clean()
{
	chunk_clean(ck);
	mu_assert(ck->end == 0, "max is not equal 0.");

	return NULL;
}

char *
test_store()
{
	char *tmp = "abcdefghijklmn";
	int len = strlen(tmp);
    mu_assert(chunk_store(ck, tmp, 0 ,(len-1)) == (len), "Wrong size after store.");

	debug("after store is:%s", ck->content);
	return NULL;
}

char *
test_merge()
{
	 //char *sha = "fd3de00505e8a6fcae53c45c58a81f96cda64d29";
	char sha[LEN];
	sha[0] = 'f';
	sha[LEN-2] = '9';	
	sha[LEN-1] = '\0';
	int i;
	for (i = 1; i < (LEN-2); ++i)
		sha[i] = 'd';
	
	int len = strlen(sha);	
	
	debug("before merge is:%s", ck->content);
	mu_assert(chunk_merge(ck, sha, 0, (len-1)) == (len), "Wrong size after merge.");
	debug("after merge is:%s", ck->content);
	return NULL;
}


char *
all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	mu_run_test(test_clean);
	mu_run_test(test_store);
	mu_run_test(test_merge);
	mu_run_test(test_destroy);

	return NULL;
}

RUN_TESTS(all_tests);
