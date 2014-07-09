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

#include <stdio.h>
#include <string.h>

#include <db.h>
#include <jansson.h>

#include "users.h"

int register_user(DB * db, char * nickname, char * fullname)
{
  DBT key, data;
  DB_BTREE_STAT db_stats;
  USER_DB_INFO new_user;
  int stor_retval;
  
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  key.data = nickname;
  key.size = strlen(nickname) + 1;
  
  if(db->stat(db, NULL, &db_stats, 0))
  {
    return -2;
  }
  
  new_user.nickname = nickname;
  new_user.fullname = fullname;
  new_user.total_pts = new_user.q_total= new_user.q_success= new_user.num_games= 0;
  new_user.db_id = 0;
  /* new_user.db_id = db_stats.bt_nkeys; */ /* Doesn't do what I want... */
  
  
  if((stor_retval = store_user_entry(db, &new_user)) == 1)
  {
    /* User exists. */
    return 1;
  }
  else if(stor_retval)
  {
    /* Something went wrong. */
    return -3;
  }
  
  fprintf(stderr, "A new user has been added, user id %d.\n", new_user.db_id);

  return 0;
}

int join_game(DB * db, char * nickname, USER_GAME_INFO * gameinfo)
{
  return 0;
}



int store_user_entry(DB * db, const USER_DB_INFO * userinfo)
{
  DBT key, data;	
  json_t * json_entry;
  char * json_string;
  int dbput_retval;
  
  json_entry = json_pack("{s, s, s, s, s, i, s, i, s, i, s, i, s, i}", "nickname", \
    userinfo->nickname, "fullname", userinfo->fullname, "total_pts", userinfo->total_pts, \
    "q_total", userinfo->q_total, "q_success", userinfo->q_success, "num_games", \
    userinfo->num_games, "db_id", userinfo->db_id);
  if(json_entry == NULL)
  {
    return -1;
  }
  
  if((json_string = json_dumps(json_entry, JSON_COMPACT)) == NULL)
  {
    return -2;
  }
  
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  key.data = userinfo->nickname;
  key.size = strlen(userinfo->nickname) + 1;
  data.data = json_string;
  data.size = strlen(json_string) +1;
  
  if((dbput_retval = db->put(db, NULL, &key, &data, DB_NOOVERWRITE)) == DB_KEYEXIST)
  {
    free(json_string);
    return 1;
  }
  else if(dbput_retval)
  {
    free(json_string);  
    return -3;
  }
  
  free(json_string);
  return 0;		
}

int load_user_entry(DB * db, char * nickname, USER_DB_INFO * userinfo)
{
  DBT key, data;	
  json_t * json_entry;
  char * json_string;
  int dbget_retval;
  
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  key.data = nickname;
  key.size = strlen(nickname) + 1;
  
  if((dbget_retval = db->get(db, NULL, &key, &data, 0)) == DB_NOTFOUND)
  {
    return 1;
  }
  else if(dbget_retval)
  {
    return -1;
  }
  
  json_string = data.data;
  if((json_entry = json_loads(json_string, 0, NULL)) == NULL)
  {
    return -2;
  }
  
  if(json_unpack(json_entry, "{s, s, s, s, s, i, s, i, s, i, s, i, s, i}", "nickname", \
    userinfo->nickname, "fullname", userinfo->fullname, "total_pts", userinfo->total_pts, \
    "q_total", userinfo->q_total, "q_success", userinfo->q_success, "num_games", \
    userinfo->num_games, "db_id", userinfo->db_id))
  {
    return -3;
  }
  /* data.data = json_string;
  data.size = strlen(json_string) +1; */
  
  
  return 0;
}
