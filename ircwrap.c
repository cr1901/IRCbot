

#include "sockwrap.h"
#include "ircwrap.h"

#define _NL_ "\r\n"

void set_credentials(sock_id sock, char * output_buffer, const char * user_name, const char * nick)
{
  sock_printf(sock, output_buffer, "USER %s . . :This is a bot programmed by William D. Jones" _NL_, user_name);
  sock_printf(sock, output_buffer, "NICK %s" _NL_, nick);
}

void join_room(sock_id sock, char * output_buffer, const char * room_name)
{
  sock_printf(sock, output_buffer, "JOIN %s" _NL_, room_name);
}

/* void send_message(sock_id sock, char * output_buffer, const char * recipient, const char * message)
{
	
	
} */
