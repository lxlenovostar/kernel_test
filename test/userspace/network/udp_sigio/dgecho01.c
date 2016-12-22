/* include dgecho1 */
#include	"unp.h"

static int		sockfd;

#define	QSIZE	   8		/* size of input queue */
#define	MAXDG	4096		/* max datagram size */

typedef struct {
  void		*dg_data;		/* ptr to actual datagram */
  size_t	dg_len;			/* length of datagram */
  struct sockaddr  *dg_sa;	/* ptr to sockaddr{} w/client's address */
  socklen_t	dg_salen;		/* length of sockaddr{} */
} DG;

static DG	dg[QSIZE];			/* queue of datagrams to process */
static long	cntread[QSIZE+1];	/* diagnostic counter */

static int	iget;		/* next one for main loop to process */
static int	iput;		/* next one for signal handler to read into */
static int	nqueue;		/* # on queue for main loop to process */

static socklen_t clilen;/* max length of sockaddr{} */

static void	sig_io(int);
static void	sig_hup(int);
/* end dgecho1 */

/* include dgecho2 */
void
dg_echo(int sockfd_arg, SA *pcliaddr, socklen_t clilen_arg)
{
	int			i;
	const int	on = 1;
	sigset_t	zeromask, newmask, oldmask;

	sockfd = sockfd_arg;
	clilen = clilen_arg;

	for (i = 0; i < QSIZE; i++) {	/* init queue of buffers */
		dg[i].dg_data = Malloc(MAXDG);
		dg[i].dg_sa = Malloc(clilen);
		dg[i].dg_salen = clilen;
	}
	iget = iput = nqueue = 0;

	Signal(SIGHUP, sig_hup);
	Signal(SIGIO, sig_io);

	Fcntl(sockfd, F_SETOWN, getpid());

	/* 设置信号驱动 */
	Ioctl(sockfd, FIOASYNC, &on);
	/* 设置非阻塞 */
	Ioctl(sockfd, FIONBIO, &on);

	Sigemptyset(&zeromask);		/* init three signal sets */
	/* contain the old signal mask when we block SIGIO. */
	Sigemptyset(&oldmask);
	Sigemptyset(&newmask);

	Sigaddset(&newmask, SIGIO);	/* signal we want to block */

	/* Block SIGIO, save previous signal mask */
	Sigprocmask(SIG_BLOCK, &newmask, &oldmask);
	for ( ; ; ) {
		/*
         测试nqueue的时候，需要对SIGIO进行阻塞，因为其被主循环和信号处理函数所共享。
         假如没有没有被阻塞，我们可能测试 nqueue时发现它为0，但是刚测试完毕SIGIO信号
         就递交了，导致nqueue被设置为1，我们接着调用sigsuspend进入睡眠，这样实际上就
         错过了这个信号，除非另有其他信号发生，否则我们将永远不能从sigsuspend调用中
         被唤醒。
         */
		while (nqueue == 0)
			/* 
             a means of atomically unblocking a signal and suspending the process. 
               
             Calling sigsuspend() is equivalent to atomically performing these operations:
			 sigprocmask(SIG_SETMASK, &mask, &prevMask); // Assign new mask 
			 pause();
			 sigprocmask(SIG_SETMASK, &prevMask, NULL);  // Restore old mask 

			 sigsuspend returns after a signal has been caught and signal handler returns.
             */
			sigsuspend(&zeromask);	/* wait for datagram to process */

		/* unblock SIGIO */
		Sigprocmask(SIG_SETMASK, &oldmask, NULL);

		Sendto(sockfd, dg[iget].dg_data, dg[iget].dg_len, 0, dg[iget].dg_sa, dg[iget].dg_salen);

		if (++iget >= QSIZE)
			iget = 0;

		/* block SIGIO */
        /*
         We must block the signal while modifying this variable since it is shared between the  
         main loop and the signal handler. Also, we need SIGIO blocked when we test nqueue at 
         the top of the loop.
         */
		Sigprocmask(SIG_BLOCK, &newmask, &oldmask);
		nqueue--;
	}
}
/* end dgecho2 */

/* include sig_io */
static void
sig_io(int signo)
{
	ssize_t		len;
	int			nread;
	DG			*ptr;

	for (nread = 0; ; ) {
		if (nqueue >= QSIZE)
			err_quit("receive overflow");

		ptr = &dg[iput];
		ptr->dg_salen = clilen;
		len = recvfrom(sockfd, ptr->dg_data, MAXDG, 0, ptr->dg_sa, &ptr->dg_salen);
		if (len < 0) {
			if (errno == EWOULDBLOCK)
				break;		/* all done; no more queued to read */
			else
				err_sys("recvfrom error");
		}
		ptr->dg_len = len;

		nread++;
		nqueue++;
		if (++iput >= QSIZE)
			iput = 0;

	}
	cntread[nread]++;		/* histogram of # datagrams read per signal */
}
/* end sig_io */

/* include sig_hup */
static void
sig_hup(int signo)
{
	int		i;

	for (i = 0; i <= QSIZE; i++)
		printf("cntread[%d] = %ld\n", i, cntread[i]);
}
/* end sig_hup */
