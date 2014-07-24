#include "cc_config.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_LIBDB5
	#include <db5/db.h>
#else
	#include <db.h>
#endif

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
