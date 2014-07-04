#define _POSIX_SOURCE
#define _BSD_SOURCE /* At least glibc on Linux requires this... ./configure for 
Berkeley DB, and the default glibc macros contradict:
1. using the -ansi option on GCC for compiling, AND/OR
2. Defining _POSIX_SOURCE for POSIX compliance.
This application does both :P.

POSIX doesn't require u_int, u_long, etc, but Berkeley DB uses them internally,
defining them with typedefs if they do not exist in db.h. BSD libc sys/types.h defines 
them for SysV compatibility, and by default glibc does as well by default (i.e.
without any special -ansi, -std=c99, etc dialect flags to gcc or Feature Test Macros). 

_BSD_SOURCE implies _POSIX_SOURCE/_POSIX_C_SOURCE, so any other POSIX platform 
with an ANSI C compiler should compile just fine by using the _POSIX_SOURCE macro 
(provided Berkeley DB was ./configured or ported to provide the SysV u_int, 
u_long, etc data types internally).

A better solution may be to check the host platform prior to compiling and add 
a macro if needed, or change the standard/compiler switches for this one file- as long
as this application code remains ANSI (i.e. does not depend on gcc), either should work
for other platforms. */
#include <db.h>

#include "users.h"

int register_user(char * nickname, char * fullname)
{
  return 0;
}

int join_game(char * nickname, USER_GAME_INFO * gameinfo)
{
  return 0;
}
