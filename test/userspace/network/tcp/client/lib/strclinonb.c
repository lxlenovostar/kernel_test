#include	"lx_sock.h"
#include 	"error.h"

/*
 How does non-blocking I/O provide better throughput? The OS schedules the 
 user process differently in the case of blocking and non-blocking I/O. When 
 you block, the process sleeps, which leads to a context switch. When you us
 use non-blocking sockets, this problem is avoided.
 */
void
str_cli(FILE *fp, int sockfd)
{
	int			maxfdp1, val, stdineof;
	ssize_t		n, nwritten;
	fd_set		rset, wset;
	char		to[MAXLINE], fr[MAXLINE];
	char		*toiptr, *tooptr, *friptr, *froptr;
	
	/* 三个文件描述符设置为nonblock. */
	val = Fcntl(sockfd, F_GETFL, 0);
	Fcntl(sockfd, F_SETFL, val | O_NONBLOCK);

	val = Fcntl(STDIN_FILENO, F_GETFL, 0);
	Fcntl(STDIN_FILENO, F_SETFL, val | O_NONBLOCK);

	val = Fcntl(STDOUT_FILENO, F_GETFL, 0);
	Fcntl(STDOUT_FILENO, F_SETFL, val | O_NONBLOCK);

	toiptr = tooptr = to;	/* initialize buffer pointers */
	friptr = froptr = fr;
	stdineof = 0;

	maxfdp1 = max(max(STDIN_FILENO, STDOUT_FILENO), sockfd) + 1;
	for ( ; ; ) {
		/* FD_ZERO() initializes the set pointed to by fdset to be empty. */
		FD_ZERO(&rset);
		FD_ZERO(&wset);

		if (stdineof == 0 && toiptr < &to[MAXLINE])
			/* FD_SET() adds the file descriptor fd to the set pointed to by fdset. */
			FD_SET(STDIN_FILENO, &rset);	/* read from stdin */

		if (friptr < &fr[MAXLINE])
			FD_SET(sockfd, &rset);			/* read from socket */

		if (tooptr != toiptr)
			FD_SET(sockfd, &wset);			/* data to write to socket */

		if (froptr != friptr)
			FD_SET(STDOUT_FILENO, &wset);	/* data to write to stdout */

		Select(maxfdp1, &rset, &wset, NULL, NULL);

		/* 
         FD_ISSET() returns true if the file descriptor fd is a member of the set pointed to
		 by fdset.
         */
		if (FD_ISSET(STDIN_FILENO, &rset)) {
			/* The third argument is the amount of available space in the to buffer. */
			if ( (n = read(STDIN_FILENO, toiptr, &to[MAXLINE] - toiptr)) < 0) {
				if (errno != EWOULDBLOCK)
					err_sys("read error on stdin");
			} else if (n == 0) {
				//fprintf(stderr, "%s: EOF on stdin\n", gf_time());
				stdineof = 1;			/* all done with stdin */
				if (tooptr == toiptr)
					Shutdown(sockfd, SHUT_WR);/* send FIN */

			} else {
				//fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), n);
				/*
				 When read returns data, we increment toiptr accordingly. We also turn on 
                 the bit corresponding to the socket in the write set, to cause the test for 
                 this bit to be true later.
                 */
				toiptr += n;			/* # just read */
				FD_SET(sockfd, &wset);	/* try and write to socket below */
			}
		}

		if (FD_ISSET(sockfd, &rset)) {
			if ( (n = read(sockfd, friptr, &fr[MAXLINE] - friptr)) < 0) {
				if (errno != EWOULDBLOCK)
					err_sys("read error on socket");

			} else if (n == 0) {
				//fprintf(stderr, "%s: EOF on socket\n", gf_time());
				if (stdineof)
					return;		/* normal termination */
				else
					err_quit("str_cli: server terminated prematurely");

			} else {
				//fprintf(stderr, "%s: read %d bytes from socket\n", gf_time(), n);
				friptr += n;		/* # just read */
				FD_SET(STDOUT_FILENO, &wset);	/* try and write below */
			}
		}

		if (FD_ISSET(STDOUT_FILENO, &wset) && ( (n = friptr - froptr) > 0)) {
			if ( (nwritten = write(STDOUT_FILENO, froptr, n)) < 0) {
				if (errno != EWOULDBLOCK)
					err_sys("write error to stdout");

			} else {
				//fprintf(stderr, "%s: wrote %d bytes to stdout\n", gf_time(), nwritten);
				froptr += nwritten;		/* # just written */
				
				/*
                 If the output pointer has caught up with the input pointer, both pointers are 
  				 reset to point to the beginning of the buffer. 
                 */
				if (froptr == friptr)
					froptr = friptr = fr;	/* back to beginning of buffer */
			}
		}

		if (FD_ISSET(sockfd, &wset) && ( (n = toiptr - tooptr) > 0)) {
			if ( (nwritten = write(sockfd, tooptr, n)) < 0) {
				if (errno != EWOULDBLOCK)
					err_sys("write error to socket");

			} else {
				//fprintf(stderr, "%s: wrote %d bytes to socket\n", gf_time(), nwritten);
				tooptr += nwritten;	/* # just written */
				if (tooptr == toiptr) {
					toiptr = tooptr = to;	/* back to beginning of buffer */
					if (stdineof)
						Shutdown(sockfd, SHUT_WR);	/* send FIN */
				}
			}
		}
	}
}
