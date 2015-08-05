#include "minunit.h"
#include "../src/lcthw/keyvalue.h"

static keyvalue *kv = NULL;

char *
test_create()
{
	kv = keyvalue_create(0, 0);
	mu_assert(kv != NULL, "keyvalue_create failed.");
	mu_assert(kv->key != NULL, "keys are wrong in kv");
	mu_assert(kv->index == 0, "index is wrong in kv");
	mu_assert(kv->capacity == SHANUM*SHA, "capacity is wrong in kv");
	mu_assert(kv->expand_rate == SHANUM*SHA, "expand_rate is wrong in kv");

	return NULL;
}

char *
test_destroy()
{
	keyvalue_destroy(kv);
	return NULL;
}

char *
test_full()
{
	mu_assert(keyvalue_full(kv) == 0, "have a lot of space.");

	return NULL;
}

char *
test_expand()
{
	 int old_max = kv->capacity;
     keyvalue_expand(kv);
     mu_assert((unsigned int) kv->capacity == old_max + kv->expand_rate, "Wrong size after expand.");

	return NULL;
}

char *
test_push()
{
	 int i = 0;
	 char *sha = "fd3de00505e8a6fcae53c45c58a81f96cda64d29";
     for (i = 0; i < SHANUM; i++) {
         keyvalue_push(kv, sha);
     }
	
	debug("capacity is %lu and NUM is %d and index is %lu", kv->capacity, SHANUM, kv->index); 
    mu_assert(kv->capacity == (2*SHA*SHANUM), "Wrong max size.");

	return NULL;
}


char *
all_tests()
{
	mu_suite_start();

	mu_run_test(test_create);
	//mu_run_test(test_push);
	mu_run_test(test_full);
	mu_run_test(test_expand);
	mu_run_test(test_destroy);

	return NULL;
}

RUN_TESTS(all_tests);
