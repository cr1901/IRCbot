#define _POSIX_SOURCE
#define _BSD_SOURCE /* At least glibc on Linux requires this... ./configure for 
Berkeley DB, and the default glibc macros contradict:
1. using the -ansi option on GCC for compiling, AND/OR
2. Defining _POSIX_SOURCE for POSIX compliance (non-GCC/GLIBC compilers).
This application does the latter :P.

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

#include <string.h>

#include <db.h>
#include <jansson.h>

#include "users.h"

int register_user(DB * db, char * nickname, char * fullname)
{
  DBT key, data;
  DB_BTREE_STAT db_stats;
  USER_DB_INFO new_user;
  int get_retval, uid;
  
  /* Check that user does not already exist. */
  
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  key.data = nickname;
  key.size = strlen(nickname) + 1;
  
  get_retval = db->get(db, NULL, &key, &data, 0);
  
  if(get_retval == 0)
  {
    return 1;
  }
  else if(get_retval != DB_NOTFOUND)
  {
    return -1;
  }
  
  if(db->stat(db, NULL, &db_stats, DB_FAST_STAT))
  {
    return -2;
  }
  
  
  
  /* user_obj = json_object(void);
  if(user_obj == NULL)
  {
    return -3;
  } */
  
  
  fprintf(stderr, "A new user has been added.\n");

  return 0;
}

int join_game(DB * db, char * nickname, USER_GAME_INFO * gameinfo)
{
  return 0;
}



int store_user_entry(DB * db, USER_DB_INFO * userinfo)
{
  return 0;		
}

int load_user_entry(DB * db, USER_DB_INFO * userinfo)
{
  return 0;
}
