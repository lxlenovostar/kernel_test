#include	"lx_sock.h"
#include 	"error.h"

void
str_cli(int fd, int sockfd)
{
	int maxfdp1, stdineof;
	fd_set rset;
	char buf[MAXLINE];
	int n;

	stdineof = 0;
	FD_ZERO(&rset);
	
	for ( ; ;) {
		printf("fuck end1.\n");
		if (stdineof == 0)
			FD_SET(fd, &rset);

		FD_SET(sockfd, &rset);

		maxfdp1 = max(fd, sockfd) + 1;

		printf("fuck end2.\n");
		Select(maxfdp1, &rset, NULL, NULL, NULL);

		/* socket is readable */
		if (FD_ISSET(sockfd, &rset)) {
			printf("fuck end3.\n");
			if ((n = Read(sockfd, buf, MAXLINE)) == 0) {
				if (stdineof == 1)
					return;	/* normal termination. */
				else
					err_quit("str_cli: server terminated prematurely");
			}
			Write(fileno(stdout), buf, n);
		}

		/* input is readable */
		if (FD_ISSET(fd, &rset)) {
			printf("fuck end4.\n");
			//if ((n = Read(fd, buf, MAXLINE)) == 0) {
			if ((n = Read(fd, buf, 3)) == 0) {
				stdineof = 1;
				Shutdown(sockfd, SHUT_WR);	/* send FIN */
				FD_CLR(fd, &rset);
				continue;
			}

			Writen(sockfd, buf, n);
		}

		printf("fuck end.\n");
	}
}
