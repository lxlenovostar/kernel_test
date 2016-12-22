#include <stdio.h>
#include <stdlib.h>

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>

#define MY_MSG_TYPE (0x10 + 2)  // + 2 is arbitrary but is the same for kern/usr

int 
rece_from_kernel(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *r_nlh = nlmsg_hdr(msg);
	
 	printf("Received message payload:%s\n", (char *)NLMSG_DATA(r_nlh));

	nlmsg_free(msg);	
	return 0;
}

int
main(int argc, char *argv[])
{
    struct nl_sock *nls;
    char msg[] = "give your present";
    int ret;

	/*
     The application must allocate an instance of struct nl_sock for each netlink socket 
	 it wishes to use. 
     */
    nls = nl_socket_alloc();
    if (!nls) {
        printf("bad nl_socket_alloc\n");
        return EXIT_FAILURE;
    }

	/*
 	 * Notifications do not use sequence numbers, disable sequence number
 	 * checking.
 	 */
	 nl_socket_disable_seq_check(nls);

	/*
 	 * Define a callback function, which will be called for each notification
 	 * received.
     * NL_CB_VALID : Message is valid.
     * NL_CB_CUSTOM : Customized handler specified by the user. 
 	 */
	 //nl_socket_modify_cb(nls, NL_CB_VALID, NL_CB_CUSTOM, rece_from_kernel, NULL);
	 nl_socket_modify_cb(nls, NL_CB_MSG_IN, NL_CB_CUSTOM, rece_from_kernel, NULL);

	/*
     Creates a new Netlink socket using socket() and binds the socket to the protocol 
     and local port specified in the sk socket object. Fails if the socket is already connected. 

     NETLINK_USERSOCK : Reserved for user-mode socket protocols.
     */
    ret = nl_connect(nls, NETLINK_USERSOCK);
    if (ret < 0) {
        nl_perror(ret, "nl_connect");
        nl_socket_free(nls);
        return EXIT_FAILURE;
    }

	/*
     Allocates a new Netlink message based on type and flags. If buf points to payload of length 
     size that payload will be appended to the message.
     */
    ret = nl_send_simple(nls, MY_MSG_TYPE, 0, msg, sizeof(msg));
    if (ret < 0) {
        nl_perror(ret, "nl_send_simple");
        nl_close(nls);
        nl_socket_free(nls);
        return EXIT_FAILURE;
    } else {
        printf("sent %d bytes\n", ret);
    }

	while(1) {	
	ret = nl_recvmsgs_default(nls); 
	//ret = nl_recvmsgs(nls, rece_from_kernel); 
 	if (ret < 0) {
        nl_perror(ret, "nl_recvmsgs_default");
        nl_close(nls);
        nl_socket_free(nls);
        return EXIT_FAILURE;
    } else {
        printf("receive bytes\n");
    }
	
	}

    nl_close(nls);
    nl_socket_free(nls);

    return EXIT_SUCCESS;
}
