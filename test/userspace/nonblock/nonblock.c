/* http://stackoverflow.com/questions/717572/how-do-you-do-non-blocking-console-i-o-on-linux-in-c */
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

int main()
{
	char buffer[128];
	int nread;

	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    sleep(4);
	nread = read(0, buffer, 128);
	if (nread == -1)
		write(2, "A read error has occurrd\n", 26);

	if ((write(1, buffer, nread)) != nread)
		write(2, "A write error has occurrd\n", 27);

	exit(0);
}
