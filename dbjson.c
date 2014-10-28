#include "cc_config.h"

#include <errno.h> /* EEXIST */

#include <stdio.h>
#include <string.h>

/* Library headers */
#include <db5/db.h>
#include <jansson.h>

#include "debug.h"
#include "cfgfile.h"
#include "trivia.h"
#include "users.h"


static int parse_json_null_or_string(char ** ghostnick, json_t * json_rep);
static int parse_json_array_of_strings(char *** str_array, json_t * json_rep);
static int create_and_open_db(DB ** db, const char * db_path, DBTYPE db_type);



/* ===============================================================================
* Config-file code
=============================================================================== */
int read_settings_file(const char * profile, CFG_PARAMS * cfg, const char * path)
{
  json_t * json_entry, * json_ghostnick, * json_defrooms, * json_passwd;
  json_error_t error;
  
  
  cfg->cfgfile_path = path;
  cfg->profile_name = profile;
  
  if((json_entry = json_load_file(path, 0, &error)) == NULL)
  {
    return -1;
  }
  
  if((cfg->json_profile_rep = json_object_get(json_entry, profile)) == NULL)
  {
    json_decref(json_entry);
    return -2;
  }
  
  if(json_unpack_ex(cfg->json_profile_rep, &error, 0, CFG_FORMAT_STR, "server", \
    &cfg->server_name, "nickname", &cfg->nickname, "user_message", &cfg->user_message, \
    "ghostnick", &json_ghostnick, "password", &json_passwd, "userdb", &cfg->userdb_path, \
    "triviadb", &cfg->triviadb_path, "default_rooms", &json_defrooms, \
    "port",  &cfg->port, "next_user_id", &cfg->next_user_id))
  {
    fprintf(stderr, "JSON_ERROR:\ntext: %s\nsource: %s\nline: %d\ncolumn: %d\nposition: %u\n", \
    	    error.text, error.source, error.line, error.column, error.position);
    json_decref(json_entry);
    return -3;
  }
  
  /* Now we need to fill in the remaining fields. */
  if(parse_json_null_or_string(&cfg->ghostnick, json_ghostnick))
  {
    json_decref(json_entry);	  
    return -4;
  }
  
  if(parse_json_null_or_string(&cfg->password, json_passwd))
  {
    json_decref(json_entry);
    return -5;
  }
  
  /* We want to keep a reference to the current profile being used. */
  json_incref(cfg->json_profile_rep);
  /* If we got here, we can safely get rid of the root json_entry. json_ghostnick,
  json_defrooms, and json_passwd are not leaked, as they are reachable from 
  cfg->json_profile_rep. */
  json_decref(json_entry);
  
  return 0;
}
/* int read_settings_file(CFG_PARAMS * cfg, const char * path)
{
	
	
}

int write_settings_file(const CFG_PARAMS * cfg, const char * path)
{
	
	
	
} */

/* ===============================================================================
* Trivia handling code
=============================================================================== */
int open_trivia_db(DB ** trivia_db, const char * db_path)
{
  return create_and_open_db(trivia_db, db_path, DB_RECNO);
}


/* ===============================================================================
* User handling code
=============================================================================== */
int open_user_db(DB ** user_db, const char * db_path)                         
{
  return create_and_open_db(user_db, db_path, DB_BTREE);
}

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
  /* new_user.db_id = 0; */
  new_user.db_id = db_stats.bt_nkeys; /* Doesn't do what I want... */
  
  
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
  
  json_entry = json_pack(USER_FORMAT_STR, "nickname", \
    userinfo->nickname, "fullname", userinfo->fullname, "total_pts", userinfo->total_pts, \
    "q_total", userinfo->q_total, "q_success", userinfo->q_success, "num_games", \
    userinfo->num_games, "db_id", userinfo->db_id);
  if(json_entry == NULL)
  {
    return -1;
  }
  
  if((json_string = json_dumps(json_entry, JSON_COMPACT)) == NULL)
  {
    json_decref(json_entry);
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
    json_decref(json_entry);
    free(json_string);
    return 1;
  }
  else if(dbput_retval)
  {
    json_decref(json_entry);
    free(json_string);  
    return -3;
  }
  
  json_decref(json_entry);
  free(json_string);
  return 0;		
}

int load_user_entry(DB * db, char * nickname, USER_DB_INFO * userinfo)
{
  DBT key, data;	
  /* json_t * json_entry; */
  char * json_string;
  int dbget_retval;
  json_error_t error;
  
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
  if((userinfo->json_rep = json_loads(json_string, 0, &error)) == NULL)
  {
    return -2;
  }
  
  if(json_unpack_ex(userinfo->json_rep, &error, 0, USER_FORMAT_STR, "nickname", \
    &userinfo->nickname, "fullname", &userinfo->fullname, "total_pts", &userinfo->total_pts, \
    "q_total", &userinfo->q_total, "q_success", &userinfo->q_success, "num_games", \
    &userinfo->num_games, "db_id", &userinfo->db_id))
  {
    fprintf(stderr, "JSON_ERROR:\ntext: %s\nsource: %s\nline: %d\ncolumn: %d\nposition: %u\n", \
    	    error.text, error.source, error.line, error.column, error.position);
    json_decref(userinfo->json_rep);
    return -3;
  }
  /* data.data = json_string;
  data.size = strlen(json_string) +1; */
  
  
  return 0;
}

/* ===============================================================================
* Static functions
=============================================================================== */

/* Certain fields can be json strings or null. Either works, so parse accordingly. */
int parse_json_null_or_string(char ** str_rep, json_t * json_rep)
{
    (* str_rep) = NULL; /* Assume NULL. */
    if(json_unpack(json_rep, "n") && json_unpack(json_rep, "s", str_rep))
    {
      /* Both validations failed if we got here. str_rep will be allocated/set 
      appropriately as a side effect. */
      return -1;
    }
    else
    {
      return 0;
    }
}

int parse_json_array_of_strings(char *** str_array, json_t * json_rep)
{
  if(!json_is_array(json_rep))
  {
    return -1;
  }
  else
  {
    /* size_t index;
    json_t * value; */
    /* The retuned value is read-only and must not be modified or freed by the 
    user. It is valid as long as string exists, i.e. as long as its reference 
    count has not dropped to zero. */
    /* Be wary of this code... */
    (* str_array) = NULL;
    
    return 0;
  }
	
}


int create_and_open_db(DB ** db, const char * db_path, DBTYPE db_type)
{
  int dbopen_retval;
  
  if(db_create(db, NULL, 0))
  {
    fprintf(stderr, "dbcreate (%s) has failed!\n", db_path);
    return -1;
  }
  
  dbopen_retval = (* db)->open((* db), NULL, db_path, NULL, db_type, \
    DB_CREATE | DB_EXCL, 0644);
  
  if(dbopen_retval == EEXIST)
  {
    debug_fprintf(stderr, "Database exists... using existing version (%s).\n", db_path);
    if((* db)->open((* db), NULL, db_path, NULL, db_type, 0, 0644))
    {
      fprintf(stderr, "Database open failed (%s)!\n", db_path);
      return -2;
    }
  }
  else if(dbopen_retval != 0)
  {
    return -3;
  }
  
  return 0;
}

