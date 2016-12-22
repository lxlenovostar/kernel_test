/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2016.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 6-5 */

#include <setjmp.h>
#include "tlpi_hdr.h"

/*
 关于env的没有理解。
 Along with other information, env stores a copy of the program counter register
(which points to the currently executing machine-language instruction) and the
stack pointer register (which marks the top of the stack) at the time of the call to
setjmp(). This information enables the subsequent longjmp() call to accomplish two
key steps:
z Strip off the stack frames for all of the intervening functions on the stack
between the function calling longjmp() and the function that previously called
setjmp(). This procedure is sometimes called “unwinding the stack,” and is
accomplished by resetting the stack pointer register to the value saved in the
env argument.
z Reset the program counter register so that the program continues execution
from the location of the initial setjmp() call. Again, this is accomplished using
the value saved in env.
 */
static jmp_buf env;

static void
f2(void)
{
    longjmp(env, 2);
}

static void
f1(int argc)
{
    if (argc == 1)
        //longjmp(env, 0);
        longjmp(env, 1);
    f2();
}

int
main(int argc, char *argv[])
{
    switch (setjmp(env)) {
    case 0:     /* This is the return after the initial setjmp() */
        printf("Calling f1() after initial setjmp()\n");
        f1(argc);               /* Never returns... */
        break;                  /* ... but this is good form */

    case 1:
        printf("We jumped back from f1()\n");
        break;

    case 2:
        printf("We jumped back from f2()\n");
        break;
    }

    exit(EXIT_SUCCESS);
}
