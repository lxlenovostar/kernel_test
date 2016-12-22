#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void ouch(int sig)
{
	printf("OUCH! - I got signal %d\n", sig);
	(void) signal(SIGINT, SIG_DFL);
}

int main()
{
	/*
     We don’t recommend that you use the signal interface for catching signals. We include it here because
	 you will find it in many older programs. You’ll see sigaction, a more cleanly defined and reliable
	 interface later, which you should use in all new programs.
     */
	(void) signal(SIGINT, ouch);

	while(1) {
		printf("hello world!\n");
		sleep(1);
	}
}
