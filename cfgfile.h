#ifndef CFGFILE_H
#define CFGFILE_H

#include <jansson.h>

typedef struct cfg_params
{
	/* char * cfgfile_name; */
	char * server_name;
	char * nickname;
	char * user_message;
	char * ghostnick;
	char * password;
	char * userdb_path;
	char * triviadb_path;
	char ** default_rooms;
	unsigned short port;
	unsigned short next_user_id;
}CFG_PARAMS;

#define CFG_FORMAT_STR "{s: s, s: s, s: s, s: o, s: o, s: s, s: s, s: o, s: i, s: i}"

int read_settings_file(const char * profile, CFG_PARAMS * cfg, const char * path);
int write_settings_file(const char * profile, const CFG_PARAMS * cfg, const char * path);
int add_default_room(const char * profile, const char * room_name, const char * path);
int update_next_user_id(const char * profile, unsigned short user_id, const char * path);


#endif        /*  #ifndef CFGFILE_H  */

