#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PAYLOAD 128  /* maximum payload size*/
#define NETLINK_USER 31

/*
struct sockaddr_nl
{
  sa_family_t    nl_family;  // AF_NETLINK   
  unsigned short nl_pad;     // zero         
  __u32          nl_pid;     // process pid 
  __u32          nl_groups;  // mcast groups mask 
} nladdr;

struct nlmsghdr
{
  __u32 nlmsg_len;   // Length of message 
  __u16 nlmsg_type;  // Message type
  __u16 nlmsg_flags; // Additional flags 
  __u32 nlmsg_seq;   // Sequence number 
  __u32 nlmsg_pid;   // Sending process PID 
};
*/

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct msghdr msg;
struct iovec iov;
int sock_fd;

int main() {
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if (sock_fd < 0)
		return -1;

 	memset(&src_addr, 0, sizeof(src_addr));
 	src_addr.nl_family = AF_NETLINK;
 	src_addr.nl_pid = getpid();  /* self pid */
 	src_addr.nl_groups = 0;  /* not in mcast groups */

 	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

 	memset(&dest_addr, 0, sizeof(dest_addr));
 	dest_addr.nl_family = AF_NETLINK;
 	dest_addr.nl_pid = 0;   /* For Linux Kernel */
 	dest_addr.nl_groups = 0; /* unicast */

	/*
     NLMSG_SPACE()
     Return the number of bytes that a netlink message with payload
     of len would occupy.
     */
 	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	/* Fill the netlink message header */
 	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
 	nlh->nlmsg_pid = getpid();  /* self pid */
 	nlh->nlmsg_flags = 0;

 	/* Fill in the netlink message payload */
 	strcpy(NLMSG_DATA(nlh), "Hello you!");

	/* set struct iovec iov */
 	iov.iov_base = (void *)nlh;		/* starting address of buffer */
 	iov.iov_len = nlh->nlmsg_len;	/* size of buffer */

	/* struct msghdr msg; */
	/*
     The msg_name and msg_namelen members are used when the socket is not connected(eg, and 
     unconnected UDP socket).
     */
 	msg.msg_name = (void *)&dest_addr;	/* protocol address */
 	msg.msg_namelen = sizeof(dest_addr); /* size of protocol address */
	/*
	 The msg_iov and msg_iovlen members specify the array of input or output buffers(the array of 
	 iovec structs).
     */
 	msg.msg_iov = &iov; /* scatter/gather array */
 	msg.msg_iovlen = 1;	/* elements in msg_iov */

	printf("Sending message to kernel\n");
 	sendmsg(sock_fd, &msg, 0);
	printf("Waiting for message from kernel\n");

 	/* Read message from kernel */
 	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
 	recvmsg(sock_fd, &msg, 0);
	/*  
     NLMSG_DATA()
     Return a pointer to the payload associated with the passed nlmsghdr.
     */
 	printf("Received message payload:%s\n", (char *)NLMSG_DATA(nlh));

 	/* Close Netlink Socket */
    close(sock_fd);

	return 0;
}
