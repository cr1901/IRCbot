#ifndef CFGFILE_H
#define CFGFILE_H

typedef struct static_cfgfile
{
	/* char * cfgfile_name; */
	char * server_name;
	char * nickname;
	char * user_message;
	char * ghostnick;
	char * password;
	char * userdb_path;
	char * triviadb_path;
	unsigned int port;
}STATIC_CFGFILE;

typedef struct dynamic_cfgfile
{
	char ** default_rooms;
	unsigned short next_user_id;
}DYNAMIC_CFGFILE;

typedef struct cfg_params
{
	STATIC_CFGFILE stat;
	DYNAMIC_CFGFILE dyna;
}CFG_PARAMS;

int read_settings_file(const char * profile, CFG_PARAMS * cfg, const char * path);
int write_settings_file(const char * profile, const CFG_PARAMS * cfg, const char * path);
int add_default_room(const char * profile, const char * room_name, const char * path);
int update_next_user_id(const char * profile, unsigned short user_id, const char * path);


#endif        /*  #ifndef CFGFILE_H  */

