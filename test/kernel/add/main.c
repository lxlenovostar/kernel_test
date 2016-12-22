#include <stdio.h>

/* ++ 和 * 运算符同级，但是从右到左执行。 */
int main( ) {
	int a = 0;
	int b = 0;
	int *i = &a;
	int *j = &b;
	
	printf("begin i is:%d, j is:%d\n", *i, *j);
	*i++;
	(*j)++;
	printf("after i is:%d, j is:%d\n", *i, *j);

   return 0;
}
