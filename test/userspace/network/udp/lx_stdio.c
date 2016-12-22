#include "lx_stdio.h"

char *
Fgets(char *ptr, int n, FILE *stream)
{
	char    *rptr;
 
    if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
    	err_sys("fgets error");
     
     return (rptr);
}
     
FILE *
Fopen(const char *filename, const char *mode)
{   
	FILE    *fp;
 
	if ( (fp = fopen(filename, mode)) == NULL)
    	err_sys("fopen error");
 
	return(fp);
}   
         
void
Fputs(const char *ptr, FILE *stream)
{
	if (fputs(ptr, stream) == EOF)
    	err_sys("fputs error");
}

