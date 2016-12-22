//#include	"unp.h"

#include	<stdarg.h>		/* ANSI C header file */
#include	<syslog.h>		/* for syslog() */
#include    <errno.h>
#include    <stdlib.h>
#include    <string.h>
#include    <stdio.h>

#define MAXLINE     4096    /* max text line length */
int		daemon_proc;		/* set nonzero by daemon_init() */

/* Nonfatal error related to system call
 * Print message and return */
void err_ret(const char *fmt, ...);

/* Fatal error related to system call
 * Print message and terminate */
void err_sys(const char *fmt, ...);

/* Fatal error related to system call
 * Print message, dump core, and terminate */
void err_dump(const char *fmt, ...);

/* Nonfatal error unrelated to system call
 * Print message and return */
void err_msg(const char *fmt, ...);

/* Fatal error unrelated to system call
 * Print message and terminate */
void err_quit(const char *fmt, ...);

