#ifndef EVENTS_H
#define EVENTS_H

#include <time.h>

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
  COMMAND_STATS,
  COMMAND_RULES,
  COMMAND_HELP,
  COMMAND_JOIN,
  COMMAND_GAME,
  COMMAND_ANSWER,
  COMMAND_QUIT,
  TIMER_EXPIRED,
  SOCKET_CLOSED,
  SOCKET_TIMEOUT
}IRC_EVENTS;

typedef struct event_timer
{
  time_t start_time;
  int target_time;
  int timeout_ack;
  int timer_active;
}EVENT_TIMER;

#define SOCKET_TIMEOUT_TIME 300
#define JOIN_TIMEOUT 30
#define QUIZ_QUESTION_TIMEOUT 10

#define TIME_LEFT(_x) (_x).target_time - (int) difftime(time(NULL), (_x).start_time)
#define SET_TIMER(_x, _y) (_x).start_time = time(NULL); (_x).target_time = _y; (_x).timer_active = 1;
#define ACK_TIMEOUT(_x) (_x).timeout = 1;
#define UNSET_TIMER(_x) (_x).timer_active = 0;

IRC_EVENTS wait_for_event(sock_id sock, char * line_buffer, \
  unsigned int line_bufsiz, READLINE_STATE * temp_state, IRC_TOKENS * tokens, \
  EVENT_TIMER * timer);

#endif        /*  #ifndef EVENTS_H  */

