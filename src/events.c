#include <string.h>
#include <stdio.h>
#include <time.h>

#include <unistd.h>


#include "ircwrap.h"
#include "tokparse.h"
#include "events.h"
#include "debug.h"

static IRC_EVENTS parse_event(const IRC_TOKENS * tokens);
static IRC_EVENTS parse_bot_command(const IRC_TOKENS * tokens); /* Commands beginning with '%' are treated separate. */

IRC_EVENTS wait_for_event(sock_id sock, char * line_buffer, unsigned int line_bufsiz, READLINE_STATE * temp_state, IRC_TOKENS * tokens)
{
  int internal_event_processed;
  IRC_EVENTS event_to_handle = NOT_YET_KNOWN;
  
  /* Automatically handle PING/PONG. Timeout is 5 minutes. */
  do
  {
    int read_retval, tok_retval;
    
    internal_event_processed = 0;
    read_retval = read_line_from_socket(sock, line_buffer, BUFSIZ, temp_state);
    #ifndef NPRINT_OUTPUT
    write(0, line_buffer, strlen(line_buffer));
    #endif
    switch(read_retval)
    {
       case -4:
      	event_to_handle = SOCKET_TIMEOUT;
      	break;
      
      case -3:
      	event_to_handle = SOCKET_CLOSED;
      	break;
      
      case -2:
      case -1:
      	/* Warn for -1 and -2, but continue anyway. */
      case 0:
      default:    	
      	tok_retval = tokenize_irc_line(line_buffer, tokens);
      	switch(tok_retval)
      	{          
      	  case 2:
      	    /* No prefix- ignore or handle internally. */
      	    internal_event_processed = 1;
      	    break;
      	    
      	  case 1: /* Send PONG command- literally changes one character in the line buffer! */
      	    send_pong_from_ping(sock, line_buffer);
            internal_event_processed = 1;
      	    break;
      	    
      	  case -1:
      	    /* Warn, but continue anyway. */
      	  case 0:
      	  default:
      	    /* Nothing to do. */
      	    event_to_handle = parse_event(tokens);
      	    break;
      	}
      	break;
    }
  }while(internal_event_processed);
  
  return event_to_handle;
}

IRC_EVENTS parse_event(const IRC_TOKENS * irc_toks)
{
  IRC_EVENTS curr_event;
  if(irc_toks->params[1][0] == BOT_COMMAND_PREFIX)
  {
    /* Any text line beginning with '%' can be a bot command. */
    curr_event = parse_bot_command(irc_toks);
  }
  else if(!strcmp(irc_toks->command, "INVITE"))
  {
    curr_event = INVITE;
  }
  else
  {
    curr_event = INDETERMINATE;
  }
  
  return curr_event;
}

IRC_EVENTS parse_bot_command(const IRC_TOKENS * irc_toks)
{
  IRC_EVENTS curr_event;

  if(!strcmp(&irc_toks->params[1][1], "answer"))
  {
    curr_event = COMMAND_ANSWER;
  }
  else if(!strcmp(&irc_toks->params[1][1], "game"))
  {
    curr_event = COMMAND_GAME;
  }
  else if(!strcmp(&irc_toks->params[1][1], "help"))
  {
    curr_event = COMMAND_HELP;
  }
  else if(!strcmp(&irc_toks->params[1][1], "join"))
  {
    curr_event = COMMAND_JOIN;
  }
  else if(!strcmp(&irc_toks->params[1][1], "quit"))
  {
    curr_event = COMMAND_QUIT;
  }
  else if(!strcmp(&irc_toks->params[1][1], "rules"))
  {
    curr_event = COMMAND_RULES;
  }
  else if(!strcmp(&irc_toks->params[1][1], "stats"))
  {
    curr_event = COMMAND_STATS;
  }
  else
  {
    curr_event = COMMAND_BAD;
  }
  
  return curr_event;
}
