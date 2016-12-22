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

pid_t
Fork(void)
{
    pid_t   pid;

    if ( (pid = fork()) == -1) 
        err_sys("fork error");
    return(pid);
}

/* include Listen */
void
Listen(int fd, int backlog)
{   
    char    *ptr;
        
        /*4can override 2nd argument with environment variable */
    if ( (ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);
    
    if (listen(fd, backlog) < 0)
        err_sys("listen error");
}
/* end Listen */

int
Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
    int     n;
        
again:
    if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef  EPROTO
        if (errno == EPROTO || errno == ECONNABORTED)
#else
        if (errno == ECONNABORTED)
#endif
            goto again;
        else
            err_sys("accept error");
    }
    return(n);
}

void
Close(int fd)
{
    if (close(fd) == -1)
        err_sys("close error");
}    

ssize_t                     /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char  *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;       /* and call write() again */
            else
                return(-1);         /* error */
        }   

        nleft -= nwritten;
        ptr   += nwritten;
    }   
    return(n);
}
/* end writen */

void
Writen(int fd, void *ptr, size_t nbytes)
{
    if (writen(fd, ptr, nbytes) != nbytes)
        err_sys("writen error");
}

void
Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
    if (connect(fd, sa, salen) < 0)
        err_sys("connect error");
}

int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
       struct timeval *timeout)
{
    int     n;

    if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
        err_sys("select error");
    return(n);      /* can return 0 on timeout */
} 
  
int
Fcntl(int fd, int cmd, int arg)
{
    int n;

    if ( (n = fcntl(fd, cmd, arg)) == -1) 
        err_sys("fcntl error");
    return(n);
}
