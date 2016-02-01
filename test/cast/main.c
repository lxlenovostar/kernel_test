#include <stdio.h>

typedef unsigned short u16;

int main() {
	u16 a[5] = {0x1, 0x2, 0x3, 0x4, 0x5};
	char *b = (char *)a;
	int i;

	for (i = 0; i < 10; i+=2) {
		printf("a is:0x%x, 0x%x\n", *(b+i+1)&0xff, *(b+i)&0xff);
	} 		
	
	u16 *c;
	c = (u16 *)b;
	
	for (i = 0; i < 5; i++) 
		printf("c is:0x%x\n", *(c+i)&0xff);

	return 0;
}
