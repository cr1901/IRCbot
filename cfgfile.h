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

int read_settings_file(CFG_PARAMS * cfg, const char * path);
int write_settings_file(const CFG_PARAMS * cfg, const char * path);


#endif        /*  #ifndef CFGFILE_H  */

