#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int 
main()
{
		// add to 1024
		char a[3] = "aa";
		char *b = "aa";	
		char fix_buffer[1024];
		printf("%s\n", a);
		printf("%d\n", strlen(a));
		printf("%d\n", strlen(b));
		printf("%d\n", sizeof(a));
		printf("%d\n", sizeof(fix_buffer));
		memcpy(fix_buffer, a, strlen(a));
		printf("%s\n", fix_buffer);
		memset(fix_buffer + strlen(a), '\0', sizeof(fix_buffer) - strlen(a));
		printf("%s\n", fix_buffer);

		// big than 1024
		/*if (len > 1024)
		{
		memcpy(fix_buffer, a, 1023);
		memset(fix_buffer + 1023, '\0', 1);
		}*/

		return 0;
}
