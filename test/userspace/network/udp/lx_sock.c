#include <arpa/inet.h>
#include "lx_sock.h"
#include "error.h"

void
Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	if (bind(fd, sa, salen) < 0)
		err_sys("bind error");
}

/* include Socket */
int
Socket(int family, int type, int protocol)
{
    int     n;

    if ( (n = socket(family, type, protocol)) < 0)
    	err_sys("socket error");

    return(n);
}
/* end Socket */

ssize_t
Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr)
{
	ssize_t     n;
 
	if ( (n = recvfrom(fd, ptr, nbytes, flags, sa, salenptr)) < 0)
		err_sys("recvfrom error");
		return(n);
}

void
Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen)
{
    if (sendto(fd, ptr, nbytes, flags, sa, salen) != (ssize_t)nbytes)
    	err_sys("sendto error");
}

const char *
Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{   
	const char  *ptr;

    if (strptr == NULL)     /* check for old code */
    	err_quit("NULL 3rd argument to inet_ntop");
    if ( (ptr = inet_ntop(family, addrptr, strptr, len)) == NULL)
    	err_sys("inet_ntop error");     /* sets errno */
    return(ptr);
}   
         
void
Inet_pton(int family, const char *strptr, void *addrptr)
{
	int     n;
 
	if ( (n = inet_pton(family, strptr, addrptr)) < 0)
    	err_sys("inet_pton error for %s", strptr);  /* errno set */
    else if (n == 0)
    	err_quit("inet_pton error for %s", strptr); /* errno not set */
     
    /* nothing to return */
}


