#ifndef EVENTS_H
#define EVENTS_H

#include "ircwrap.h"
#include "tokparse.h"

typedef enum irc_events
{
  INDETERMINATE=-1,
  NOT_YET_KNOWN,
  INVITE,
  PRIVMSG_ROOM, /* PRIVMSG to room */
  PRIVMSG_BOT, /* PRIVMSG to bot. */
  COMMAND_BAD,
  COMMAND_REGISTER,
  COMMAND_QUIT,
  SOCKET_CLOSED,
  SOCKET_TIMEOUT
}IRC_EVENTS;

IRC_EVENTS wait_for_event(sock_id sock, char * line_buffer, unsigned int line_bufsiz, READLINE_STATE * temp_state, IRC_TOKENS * tokens);

#endif        /*  #ifndef EVENTS_H  */

