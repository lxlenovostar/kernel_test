#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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


void Bind(int fd, const struct sockaddr *sa, socklen_t salen);
int Socket(int family, int type, int protocol);
ssize_t Recvfrom(int fd, void *ptr, size_t nbytes, int flags, struct sockaddr *sa, socklen_t *salenptr);
void Sendto(int fd, const void *ptr, size_t nbytes, int flags, const struct sockaddr *sa, socklen_t salen);
const char* Inet_ntop(int family, const void *addrptr, char *strptr, size_t len);
void Inet_pton(int family, const char *strptr, void *addrptr);
