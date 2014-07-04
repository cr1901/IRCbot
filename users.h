#include <sys/types.h>
#include <db.h>


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
  
int register_user(char * nickname, char * fullname);
int join_game(char * nickname, USER_GAME_INFO * gameinfo);
