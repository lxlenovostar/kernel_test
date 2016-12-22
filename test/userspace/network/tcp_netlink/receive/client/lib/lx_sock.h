#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <string.h>
#include    <sys/types.h>   /* basic system data types */
#include    <signal.h>
#include 	<time.h>
#include 	<pthread.h>
#include    <stdio.h>       /* for convenience */
#include    <stdlib.h>      /* for convenience */
#include    <string.h>      /* for convenience */
#include    <unistd.h>      /* for convenience */

/*
struct in_addr {
	in_addr_t s_addr;	//32-bit IPv4 address, network byte ordered 
};

struct sockaddr_in {
	uint8_t sin_len; // length of structure (16) 
	sa_family_t sin_family; // AF_INET 
	in_port_t sin_port; // 16-bit TCP or UDP port number, network byte ordered 
	struct in_addr sin_addr; // 32-bit IPv4 address, network byte ordered 
	char sin_zero[8]; // unused 
};

struct sockaddr {
	uint8_t sa_len;
	sa_family_t sa_family; // address family: AF_xxx value 
	char sa_data[14]; // protocol-specific address 
};
*/

#define SERV_PORT        9877           /* TCP and UDP */
#define MAXLINE     4096    /* max text line length */
/* Following shortens all the typecasts of pointer arguments: */
#define SA  struct sockaddr
#define bzero(ptr,n)        memset(ptr, 0, n)

/* Following could be derived from SOMAXCONN in <sys/socket.h>, but many
   kernels still #define it as 5, while actually supporting many more */
#define LISTENQ     1024    /* 2nd argument to listen() */

typedef void    Sigfunc(int);   /* for signal handlers */

#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))

void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Socket(int family, int type, int protocol);
ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr);
void Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen);
const char* Inet_ntop(int family, const void *addrptr, char *strptr, size_t len);
void Inet_pton(int family, const char *strptr, void *addrptr);
pid_t Fork(void);
void Listen(int fd, int backlog);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Close(int fd);
void Writen(int fd, void *ptr, size_t nbytes);
void Connect(int fd, const struct sockaddr *sa, socklen_t salen);
ssize_t Readline(int fd, void *ptr, size_t maxlen);
FILE * Fopen(const char *filename, const char *mode);
char * Fgets(char *ptr, int n, FILE *stream);
void Fputs(const char *ptr, FILE *stream);
void sig_chld(int signo);
Sigfunc * Signal(int signo, Sigfunc *func);
void str_cli(FILE *fp, int sockfd);
int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
ssize_t Read(int fd, void *ptr, size_t nbytes);
void Write(int fd, void *ptr, size_t nbytes);
void Shutdown(int fd, int how);
void Pthread_cond_signal(pthread_cond_t *cptr);
void Pthread_cond_wait(pthread_cond_t *cptr, pthread_mutex_t *mptr);
void Pthread_mutex_lock(pthread_mutex_t *mptr);
void Pthread_mutex_unlock(pthread_mutex_t *mptr);
