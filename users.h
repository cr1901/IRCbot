#ifndef USERS_H
#define USERS_H

#include "cc_config.h"

#include <sys/types.h>

#ifdef HAVE_LIBDB5
	#include <db5/db.h>
#elif defined HAVE_LIBDB4
	#include <db4/db.h>
#elif defined HAVE_LIBDB3
	#include <db3/db.h>
#elif defined HAVE_LIBDB2
	#include <db2/db.h>	
#else
	#include <db.h>
#endif


typedef struct user_db_info
{
  char * nickname;
  char * fullname;
  unsigned long int total_pts;
  unsigned int q_total;
  unsigned int q_success;
  unsigned int num_games;
  unsigned short int db_id;
}USER_DB_INFO;

typedef struct user_game_info
{
  USER_DB_INFO db_info;
  unsigned char curr_q_total;
  unsigned char curr_q_success; 
  unsigned short int curr_pts;
}USER_GAME_INFO;

#define USER_FORMAT_STR "{s: s, s: s, s: i, s: i, s: i, s: i, s: i}"

int open_user_db(DB ** user_db, const char * db_path);
int register_user(DB * db, char * nickname, char * fullname);
int join_game(DB * db, char * nickname, USER_GAME_INFO * gameinfo);
int store_user_entry(DB * db, const USER_DB_INFO * userinfo);
int load_user_entry(DB * db, char * nickname, USER_DB_INFO * userinfo);
#endif

