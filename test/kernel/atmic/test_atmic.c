#include "atmic.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void handle(atomic_t *atom)
{
	if (atomic_add_return(1, atom) == 1)
	{
		//do something 
		printf("now get the atomic\n");
		
		atomic_dec(atom);
	}
	else
	{
		atomic_dec(atom);
		printf("can not get the atomic\n");
		return;
	}
}

int 
main()
{
	atomic_t *atom_value;
	atomic_set(atom_value, 0);

	pthread_t thread1, thread2;
	int ret_thrd1, ret_thrd2;

	ret_thrd1 = pthread_create(&thread1, NULL, (void *)&handle, (void *)&atom_value);
	ret_thrd2 = pthread_create(&thread2, NULL, (void *)&handle, (void *)&atom_value);
	
	void *retval;
	int tmp1, tmp2;

	tmp1 = pthread_join(thread1, &retval);
	
	printf("thread1 return value(retval) is %d\n", (int)retval);
	printf("thread1 return value(tmp) is %d\n", tmp1);
	
	if (tmp1 != 0) {
		printf("cannot join with thread1\n");
	}
	
	printf("thread1 end\n");

	tmp2 = pthread_join(thread2, &retval);
	
	printf("thread2 return value(retval) is %d\n", (int)retval);
	printf("thread2 return value(tmp) is %d\n", tmp1);
	
	if (tmp2 != 0) {
		printf("cannot join with thread2\n");
	}
	printf("thread2 end\n");

	return 0;
}
