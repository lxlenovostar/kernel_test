/*
 * Standard I/O wrapper functions.
 */

#include	"lx_sock.h"
#include    "error.h"

void
Fclose(FILE *fp)
{
	if (fclose(fp) != 0)
		err_sys("fclose error");
}

FILE *
Fdopen(int fd, const char *type)
{
	FILE	*fp;

	if ( (fp = fdopen(fd, type)) == NULL)
		err_sys("fdopen error");

	return(fp);
}

char *
Fgets(char *ptr, int n, FILE *stream)
{
	char	*rptr;

	if ( (rptr = fgets(ptr, n, stream)) == NULL && ferror(stream))
		err_sys("fgets error");

	return (rptr);
}

FILE *
Fopen(const char *filename, const char *mode)
{
	FILE	*fp;

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

ssize_t
Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t     n;  

    if ( (n = read(fd, ptr, nbytes)) == -1) 
        err_sys("read error");
    return(n);
}

void
Write(int fd, void *ptr, size_t nbytes)
{
    if (write(fd, ptr, nbytes) != nbytes)
        err_sys("write error");
}
    
void
Shutdown(int fd, int how)
{       
    if (shutdown(fd, how) < 0)
        err_sys("shutdown error");
}

