#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>

#include "lx_netlink.h"
#include "unp.h"

#define PING_PONG_TYPE (0x10 + 3)  // + 2 is arbitrary but is the same for kern/usr
    
struct nl_sock *nls;
int insmod_flag = 0;	/* check kernel module insmod or not. */

int 
ping_from_kernel(struct nl_msg *msg, void *arg)
{
    const char *c_msg = "get your pid";
	struct nlmsghdr *r_nlh = nlmsg_hdr(msg);

 	printf("Received message payload:%s, type is:0x%x\n", (char *)NLMSG_DATA(r_nlh), (r_nlh->nlmsg_type)&0xff);

	/* a ACK, or other thing. */
	if (((r_nlh->nlmsg_type)&0xff) != NLMSG_DONE)
		goto out;

	if (strcmp(c_msg, (char *)NLMSG_DATA(r_nlh)) == 0)  
		insmod_flag = 1;
	else  
		insmod_flag = 0;
out:	
	return NL_OK;
}

int 
data_from_kernel(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *r_nlh = nlmsg_hdr(msg);

 	printf("%s:Received message payload:%s, type is:0x%x\n", __FUNCTION__, (char *)NLMSG_DATA(r_nlh), (r_nlh->nlmsg_type)&0xff);

	/* a ACK, or other thing. */
	if (((r_nlh->nlmsg_type)&0xff) != NLMSG_DONE)
		goto out;
	
	strcpy(buffer_libnl_libevent, (char *)NLMSG_DATA(r_nlh));

out:
	return NL_OK;
}

int 
init_sock(void) 
{
	int res = 0;

    nls = nl_socket_alloc();
    if (!nls) {
        printf("bad nl_socket_alloc\n");
        return EXIT_FAILURE;
    }

	 nl_socket_disable_seq_check(nls);
	 nl_socket_disable_auto_ack(nls);

	 nl_socket_modify_cb(nls, NL_CB_MSG_IN, NL_CB_CUSTOM, ping_from_kernel, NULL);

    res = nl_connect(nls, NETLINK_USERSOCK);
    if (res < 0) {
        nl_perror(res, "nl_connect");
        nl_socket_free(nls);
        return EXIT_FAILURE;
    }

	return res;
}

int 
send_to_kernel(void) 
{
	int ret = 0;
    char msg[] = "give your present";

	printf("Sending message to kernel\n");
    ret = nl_send_simple(nls, PING_PONG_TYPE, 0, msg, sizeof(msg));
    if (ret < 0) {
        nl_perror(ret, "nl_send_simple");
        nl_close(nls);
        nl_socket_free(nls);
        return EXIT_FAILURE;
    } else {
        printf("sent %d bytes\n", ret);
    }

	return ret;
}

int 
rece_from_kernel(void)
{
	int ret;
	ret = nl_recvmsgs_default(nls); 
 	if (ret < 0) {
        nl_perror(ret, "nl_recvmsgs_default");
        nl_close(nls);
        nl_socket_free(nls);
        return EXIT_FAILURE;
    } else {
        printf("receive bytes\n");
    }

	return ret;
}

int 
check_netlink_status(void) 
{
	int count = 0;

restart:
	send_to_kernel();
	rece_from_kernel();

	if (insmod_flag == 1) {
		/*
		 we need change this, libevent need it.
         */
		nl_socket_modify_cb(nls, NL_CB_MSG_IN, NL_CB_CUSTOM, data_from_kernel, NULL);
		netlink_fd = nl_socket_get_fd(nls);
		return 0;
	}
	else {
		count++;
		
		if (count < 3) {
			sleep(10);
			goto restart;
		}
		else 
			return 1;
	}

	return 1;
}

void 
free_netlink_resource(void) 
{
    nl_close(nls);
    nl_socket_free(nls);
}
