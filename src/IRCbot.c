#include "cc_config.h"

/* ANSI headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <time.h>
#include <errno.h>

/* Library headers */
#include <jansson.h>

/* This source defines */
#include "debug.h"
#include "events.h"
#include "cfgfile.h"
#include "ircwrap.h"
#include "tokparse.h"
#include "trivia.h"
#include "users.h"

#define _NL_ "\r\n"

/* char ** commands = { \ */



int create_path(char * out_buf, const char * str_pre, const char * str_app);
static int allocate_buffers(char ** buf_ptrs[], int num_bufs, size_t bufsiz);

int main(int argc, char * argv[])
{
  sock_id my_socket;
  int retval, done = 0; /* Generic return value used throughout. */
  char ** buffer_array[4], * line_buffer, * output_buffer, * db_path;
  READLINE_STATE socket_buf;
  IRC_TOKENS irc_toks;
  CFG_PARAMS cfg_file;
  DB * user_db, * trivia_db, * game_db;
  ptrdiff_t db_path_size; /* We need the size in bytes of argv[4], not strlen. */
  
  if(argc < 2)
  {
    fprintf(stderr, "Usage: IRCbot settings_file\n"
            "Extra arguments ignored.\n");
    return EXIT_FAILURE;
  }

  buffer_array[0] = &socket_buf.buf;
  buffer_array[1] = &irc_toks.buf;
  buffer_array[2] = &output_buffer;
  buffer_array[3] = &line_buffer;
  /* Just use BUFSIZ for now... should be more than enough (BUFSIZ must be >= 256,
  but I've never seen it less than 512, which is the max IRC message size. */
  if((retval = allocate_buffers(buffer_array, (sizeof(buffer_array)/sizeof(char *)), BUFSIZ)))
  {
    fprintf(stderr, "Initial memory allocation failure!\n");
    return EXIT_FAILURE;
  }
  
  socket_buf.buf_offset = socket_buf.buf;
  socket_buf.bufsiz = irc_toks.bufsiz = BUFSIZ;
  
  if(read_settings_file("default", &cfg_file, argv[1]))
  {
    fprintf(stderr, "Failed to read settings file!\n");
    return EXIT_FAILURE;
  }
  
  if((retval = open_user_db(&user_db, cfg_file.userdb_path)))
  {
    fprintf(stderr, "Could not initialize user db- aborting (%d).\n", retval);
    return EXIT_FAILURE;
  }
  
  /* create_path(db_path, argv[4], "trivia.db"); */
  if((retval = open_trivia_db(&trivia_db, cfg_file.triviadb_path)))
  {
    fprintf(stderr, "Could not initialize trivia db- aborting (%d).\n", retval);
    return EXIT_FAILURE;
  }
  

  if(get_a_socket_and_connect(&my_socket, cfg_file.server_name, "6667"))
  {
    fprintf(stderr, "Could not connect- aborting.\n");
    return EXIT_FAILURE;
  }
  
  
  set_credentials(my_socket, output_buffer, cfg_file.nickname, cfg_file.nickname);
  /* join_room(my_socket, output_buffer, argv[3]); */
  /* output_buffer_size = sprintf(output_buffer, "PRIVMSG %s :!help" _NL_, argv[3]);
  write(my_socket, output_buffer, output_buffer_size); */
  
  /* Main loop begins here */
  while(!done)
  {
    int read_retval, tok_retval;
    IRC_EVENTS curr_event;
   char * msg_recipient;
    char nickname[17] = {'\0'};
    ptrdiff_t nickname_len;
    int count = 0;
    
    
    curr_event = wait_for_event(my_socket, line_buffer, BUFSIZ, &socket_buf, &irc_toks);
    
    /* fprintf(stderr, "Origin: %s, Command: %s, Num Params: %u\nParams:\n", \
    	      irc_toks.prefix, irc_toks.command, irc_toks.num_params);
    
    for(count = 0; count < irc_toks.num_params; count++)
    {
    	fprintf(stderr, "%d: %s ", count, irc_toks.params[count]);
    }
    fputs("\n", stderr); */
    
    /* Refactor from here... */
    nickname_len = strchr(irc_toks.prefix, '!') - irc_toks.prefix;
    if(nickname_len < 17 && nickname_len >= 0)
    {
      strncpy(nickname, irc_toks.prefix, nickname_len);
    }
    else
    {
      strcpy(nickname, "NULL");
    }
    
    if(!strcmp(irc_toks.params[0], cfg_file.nickname))
    {
      msg_recipient = nickname;
    }
    else
    {
      msg_recipient = irc_toks.params[0];
    }
    /* To here... */
    
    
    if(curr_event == INVITE)
    {
      fprintf(stderr, "Invite received from: %s\n", irc_toks.params[1]);
      join_room(my_socket, output_buffer, irc_toks.params[1]);
    }
    else if(curr_event == COMMAND_QUIT)
    {
      sock_printf(my_socket, output_buffer, "PRIVMSG %s :Goodbye" _NL_, msg_recipient);
      sock_printf(my_socket, output_buffer, "QUIT :Chances are, I'm being worked on." _NL_);
      done = 1;    
    }
    else if(curr_event == COMMAND_BAD)
    {
    	sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry, %s, I "\
    	  "didn't understand the command." _NL_, msg_recipient, nickname);    
    }
    
    /* read_retval = read_line_from_socket(my_socket, line_buffer, BUFSIZ, &socket_buf);
    if(read_retval == -3)
    {
      done = 1;
    }
    else if(read_retval == -4)
    {
      done = 1;
      // Move to state switch statement.
      // reconnect_using_current_state()
      // display_sorry_message()
    } 
    
    #ifndef NPRINT_OUTPUT
    write(0, line_buffer, strlen(line_buffer));
    #endif
    
    tok_retval = tokenize_irc_line(line_buffer, &irc_toks);
    
    if(tok_retval < 0)
    {
      fprintf(stderr, "Warning: tokenization failed with error code: %d... will" \
      	      " attempt to continue.\n", tok_retval);
    }
    else if(tok_retval == 1)
    {
      int chars_written;
      time_t curr_time;
      struct tm * struct_time; */
      
      /* Send PONG in response to PING. */
      /* line_buffer[1] = 'O';
      chars_written = write(my_socket, line_buffer, strlen(line_buffer));
      if(chars_written >= 0)
      {
      	time(&curr_time);
        struct_time = localtime(&curr_time);
        debug_fprintf(stderr, "PONG sent in response to PING. (%d bytes, time: %s)", chars_written, asctime(struct_time));
      }
      else
      {
      	 fprintf(stderr, "Socket error in write() call: %s\n", strerror(errno));
      }
    }
    else if(tok_retval == 0)
    {
      char nickname[17] = {'\0'};
      ptrdiff_t nickname_len;
      int count = 0;
      fprintf(stderr, "Origin: %s, Command: %s, Num Params: %u\nParams:\n", \
      	      irc_toks.prefix, irc_toks.command, irc_toks.num_params);
      for(count = 0; count < irc_toks.num_params; count++)
      {
      	fprintf(stderr, "%d: %s ", count, irc_toks.params[count]);
      }
      fputs("\n", stderr);
      
      nickname_len = strchr(irc_toks.prefix, '!') - irc_toks.prefix;
      if(nickname_len < 17 && nickname_len >= 0)
      {
        strncpy(nickname, irc_toks.prefix, nickname_len);
      }
      else
      {
      	strcpy(nickname, "NULL");
      }
      
      if(!strcmp(irc_toks.command, "PRIVMSG"))
      {
      	char * msg_recipient;
      	if(!strcmp(irc_toks.params[0], cfg_file.nickname))
      	{
      	  msg_recipient = nickname;
      	}
      	else
      	{
      	  msg_recipient = irc_toks.params[0];
      	}
      	
        if(irc_toks.params[1][0] == '%')
        {
          if(!strcmp(&irc_toks.params[1][1], "register"))
          {
            int reg_status = register_user(user_db, nickname, irc_toks.prefix);
            
            if(reg_status == 1)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :The user DB says "\
              	      "I have already registered you, %s." _NL_, msg_recipient, nickname);
            }
            else if(reg_status == 0)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Okay %s, I "\
            	      "have registered you into the user DB." _NL_, msg_recipient, nickname);
            }
            else
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry %s, Something "\
            	      "went wrong while registering you. Error code ?." _NL_, msg_recipient, nickname);
            }
          }
          else if(!strcmp(&irc_toks.params[1][1], "finish"))
          {
            sock_printf(my_socket, output_buffer, "PRIVMSG %s :Goodbye" _NL_, msg_recipient);
            sock_printf(my_socket, output_buffer, "QUIT :Chances are, I'm being worked on." _NL_);
            done = 1;
          }
          else if(!strcmp(&irc_toks.params[1][1], "stats"))
          {
            char * user_requested;
            int load_status;
            USER_DB_INFO user_entry;
            
            if(irc_toks.num_params <= 2)
            {
              user_requested = nickname;
            }
            else
            {
              user_requested = irc_toks.params[2];
            }
            
            load_status = load_user_entry(user_db, user_requested, &user_entry);
            if(load_status == 1)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :I can't find "\
                "%s in the database." _NL_, msg_recipient, user_requested);
            }
            else if(load_status == 0)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Stats for %s" _NL_, msg_recipient, user_requested);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Total pts: %lu" _NL_, msg_recipient, user_entry.total_pts);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Total q's answered: %u" _NL_, msg_recipient, user_entry.q_total);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Q's answered correctly: %u" _NL_, msg_recipient, user_entry.q_success);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Number of games: %u" _NL_, msg_recipient, user_entry.num_games);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :BDB User ID: %d" _NL_, msg_recipient, user_entry.db_id);
            }
            else
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry %s, "\
                "something went wrong while requesting stats for %s (%d)." _NL_, \
                msg_recipient, nickname, user_requested, load_status);
            }
          }
          else
          {
            sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry, %s, I "\
            	      "didn't understand the command." _NL_, msg_recipient, nickname);
          }
        }
      }
      else if(!strcmp(irc_toks.command, "INVITE"))
      {
        join_room(my_socket, output_buffer, irc_toks.params[1]);
      }
    } */
  }
  
  close(my_socket);
  user_db->close(user_db, 0);
  trivia_db->close(trivia_db, 0);
  return EXIT_SUCCESS;
}

int allocate_buffers(char ** buf_ptrs[], int num_bufs, size_t bufsiz)
{
  int count = 0;
  for(count = 0; count < num_bufs; count++)
  {     
    if(((* buf_ptrs[count]) = malloc(bufsiz)) == NULL)
    {
    	    return -1;
    }
  }
  return 0;
}

