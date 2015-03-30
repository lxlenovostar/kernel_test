#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>
#include <errno.h>

#define UMSG      "42"
#define UMSG_LEN  sizeof("42")

#define SOCKET_OPS_IPCHG    135
#define SOCKET_OPS_IPCHG_CHK    (SOCKET_OPS_IPCHG + 1)
#define SOCKET_OPS_MIN      SOCKET_OPS_IPCHG
#define SOCKET_OPS_MAX      (SOCKET_OPS_IPCHG_CHK + 1)

char kmsg[64];

int
main(void)
{
	int sockfd;
	int len;
	int ret;

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if (sockfd < 0) {
		printf("can not create a socket\n");
		return -1;
	}

	/*call function set_ip() */
	ret = setsockopt(sockfd, IPPROTO_IP, SOCKET_OPS_IPCHG, UMSG, UMSG_LEN);
	printf("setsockopt: ret = %d. msg = %s\n", ret, UMSG);
	len = sizeof (char) * 64;

	/*call function get_ip() */
	ret = getsockopt(sockfd, IPPROTO_IP, SOCKET_OPS_IPCHG_CHK, kmsg, &len);
	printf("getsockopt: ret = %d. msg = %s\n", ret, kmsg);
	if (ret != 0) {
		printf("getsockopt error: errno = %d, errstr = %s\n", errno,
		       strerror(errno));
	}

	close(sockfd);
	return 0;
}
