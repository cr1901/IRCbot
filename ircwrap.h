#ifndef IRCWRAP_H
#define IRCWRAP_H

#include "sockwrap.h"

void set_credentials(sock_id sock, char * output_buffer, const char * user_name, const char * nick);
void join_room(sock_id sock, char * output_buffer, const char * room_name);



#endif
