#include <stdio.h>
#include <stdlib.h>

#include <netlink/netlink.h>

#define MY_MSG_TYPE (0x10 + 2)  // + 2 is arbitrary but is the same for kern/usr

int
main(int argc, char *argv[])
{
    struct nl_sock *nls;
    char msg[] = { 0xde, 0xad, 0xbe, 0xef, 0x90, 0x0d, 0xbe, 0xef };
    int ret;

    nls = nl_socket_alloc();
    if (!nls) {
        printf("bad nl_socket_alloc\n");
        return EXIT_FAILURE;
    }

    ret = nl_connect(nls, NETLINK_USERSOCK);
    if (ret < 0) {
        nl_perror(ret, "nl_connect");
        nl_socket_free(nls);
        return EXIT_FAILURE;
    }

    ret = nl_send_simple(nls, MY_MSG_TYPE, 0, msg, sizeof(msg));
    if (ret < 0) {
        nl_perror(ret, "nl_send_simple");
        nl_close(nls);
        nl_socket_free(nls);
        return EXIT_FAILURE;
    } else {
        printf("sent %d bytes\n", ret);
    }

    nl_close(nls);
    nl_socket_free(nls);

    return EXIT_SUCCESS;
}
