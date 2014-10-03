#include <string.h>

#include "tokparse.h"

/* 
Return codes:
Negative- Error, but tokenization attempted.
-1: IRC command too long, last entry truncated.
0: Tokenization okay
Positive- Special, tokenization not attempted.
1: Ping command.
2: No prefix. */
int tokenize_irc_line(const char * line_buffer, IRC_TOKENS * tokens)
{
  /* int chars_left = 512; */ /* IRC spec has lines of max size 512... first */
  char * prefix_nick_end, /* * ext_prefix_end, */ * command_end /* , * param_end */;
  unsigned int chars_left;
  
  if((line_buffer[0] != ':')) /* Check whether message prefix was specified. */
  {
    if(!strncmp(line_buffer, "PING ", 5))
    {
      return 1;
    }
    else
    {
      return 2;
    }
  }
  
  
  chars_left = (tokens->bufsiz - 1);
  
  /* Don't bother performing the memcpy unless conditions are met. */
  strncpy(tokens->buf, line_buffer, tokens->bufsiz);
  tokens->prefix = (tokens->buf + 1);
  prefix_nick_end = memchr(tokens->buf + 1, 0x20 /* space */, chars_left);
  
  if(prefix_nick_end == NULL)
  {
    return -1;
  }
  
  (* prefix_nick_end) = '\0';
  chars_left = chars_left - (prefix_nick_end - (tokens->buf + 1) + 1);
  tokens->command = prefix_nick_end + 1;
  
  command_end = memchr(tokens->command, ' ', chars_left);
  if(command_end == NULL)
  {
    return -3;
  }
  
  {
    char * param_begin, * param_end, * line_end;
    int done_parsing_params = 0, param_count = 0;
    
    (* command_end) = '\0';
    chars_left = chars_left - (command_end - (prefix_nick_end + 1) + 1);
    param_begin = command_end + 1;
    
    line_end = memchr(param_begin, 0x0A, chars_left);
    
    
    if(line_end == NULL)
    {
      return -5;
    }
    
    while(!done_parsing_params)
    {
      /* A param with a leading colon should be parsed to the end of the
      IRC line. */
      if((* param_begin) == ':')
      {
        param_begin++;
        /* Set CR to NULL... we don't care about the terminator. */
        param_end = line_end - 1;
        done_parsing_params = 1;
      }
      else
      {
        param_end = memchr(param_begin, 0x20, chars_left);
        if(param_end == NULL)
        {
        	/* The last param will be followed by CRLF... we've already 
          verified that CRLF exists. */      
        	param_end = line_end - 1;
          done_parsing_params = 1;
        }
      }
      
      (* param_end)= '\0';
      
      /* After getting next param, point array to start of param,
      and prepare for a new one. Also check to see if other termination
      conditions have been met. */
      tokens->params[param_count] = param_begin;
      chars_left = param_end - param_begin + 1;
      param_begin = param_end + 1; 
      param_count++;
      
    }
    tokens->num_params = param_count;
  }
  
  return 0;
}


/* int get_next_token(char * */
