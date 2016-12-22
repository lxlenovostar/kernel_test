#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include	"../lib/lx_sock.h"
#include	"../lib/error.h"

int
main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;
	//int 				w_fd;

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	/*
	w_fd = open("tempfile_for_select", O_RDWR | O_CREAT);
	if (w_fd == -1) {
		close(sockfd);
		err_quit("open file failed.");
	}
	
	str_cli(w_fd, sockfd);		// do it all 
	close(w_fd);
    */

	str_cli(NULL, sockfd);		/* do it all */

	exit(0);
}
