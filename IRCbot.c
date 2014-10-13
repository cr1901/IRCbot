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
#ifdef HAVE_LIBDB5
	#include <db5/db.h>
#elif defined HAVE_LIBDB4
	#include <db4/db.h>
#elif defined HAVE_LIBDB3
	#include <db3/db.h>
#elif defined HAVE_LIBDB2
	#include <db2/db.h>	
#else
	#include <db.h>
#endif

/* This source defines */
#include "debug.h"
#include "ircwrap.h"
#include "sockwrap.h"
#include "tokparse.h"
#include "users.h"

#define _NL_ "\r\n"

/* char ** commands = { \ */



int create_path(char * out_buf, const char * str_pre, const char * str_app);
int create_and_open_db(DB ** db, char * db_path, DBTYPE db_type);


int main(int argc, char * argv[])
{
  sock_id my_socket, done = 0;
  char * line_buffer, * output_buffer, * db_path;
  READLINE_STATE socket_buf;
  IRC_TOKENS irc_toks;
  DB * user_db, * trivia_db, * game_db;
  ptrdiff_t db_path_size; /* We need the size in bytes of argv[4], not strlen. */
  int retval;

  if(argc < 5)
  {
    fprintf(stderr, "Usage: IRCbot Bot_name server channel db_path (port 6667)\n"
            "Extra arguments ignored.\n");
    return EXIT_FAILURE;
  }

  /* Just use BUFSIZ for now... should be more than enough (must be >= 256,
  but I've never seen it less than 512, which is the max IRC message size. */
  socket_buf.buf_offset = socket_buf.buf = malloc(BUFSIZ);
  irc_toks.buf = malloc(BUFSIZ);
  output_buffer = malloc(BUFSIZ);
  line_buffer = malloc(BUFSIZ);
  socket_buf.bufsiz = irc_toks.bufsiz = BUFSIZ;
  
  /* This should not fail, provided the runtime isn't broken. */
  db_path_size = strrchr(argv[4], '\0') - argv[4] + 1;
  db_path = malloc(db_path_size + 1 + 16); /* Databases shall have filenames no greater than 16 bytes, including NULL. 
  The extra character is to append*/
  
  if(socket_buf.buf == NULL || output_buffer == NULL || line_buffer == NULL \
    || irc_toks.buf == NULL || db_path == NULL)
  {
    fprintf(stderr, "Initial memory allocation failure!\n");
    return EXIT_FAILURE;
  }
  
  /* fprintf(stdout, "Path was: %s.\n", db_path);
  return 0; */
  create_path(db_path, argv[4], "users.db");
  if((retval = create_and_open_db(&user_db, db_path, DB_BTREE)))
  {
    fprintf(stderr, "Could not initialize user db- aborting (%d).\n", retval);
    return EXIT_FAILURE;
  }
  
  create_path(db_path, argv[4], "trivia.db");
  if((retval = create_and_open_db(&trivia_db, db_path, DB_RECNO)))
  {
    fprintf(stderr, "Could not initialize trivia db- aborting (%d).\n", retval);
    return EXIT_FAILURE;
  }
  

  if(get_a_socket_and_connect(&my_socket, argv[2], "6667"))
  {
    fprintf(stderr, "Could not connect- aborting.\n");
    return EXIT_FAILURE;
  }
  
  
  set_credentials(my_socket, output_buffer, argv[1], argv[1]);
  join_room(my_socket, output_buffer, argv[3]);
  /* output_buffer_size = sprintf(output_buffer, "PRIVMSG %s :!help" _NL_, argv[3]);
  write(my_socket, output_buffer, output_buffer_size); */
  
  /* Main loop begins here */
  while(!done)
  {
    int read_retval, tok_retval;
    
    read_retval = read_line_from_socket(my_socket, line_buffer, BUFSIZ, &socket_buf);
    if(read_retval == -3)
    {
      done = 1;
    }
    else if(read_retval == -4)
    {
      done = 1;
      /* Move to state switch statement. */
      /* reconnect_using_current_state() */
      /* display_sorry_message() */
    }
    /* num_chars_in_curr_line = (token_ptr - curr_start_pos + 1); */
    
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
      struct tm * struct_time;
      
      /* Send PONG in response to PING. */
      line_buffer[1] = 'O';
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
      /* debug_fprintf(stderr, "PING received... PONG not sent (debugging).\n"); */
    }
    else if(tok_retval == 0)
    {
      /* Will strncat work instead of zero-ing each time this variable comes 
      into scope? */
      char nickname[17] = {'\0'}; /* If you need more chars than this, change it! */
      ptrdiff_t nickname_len;
      /* int count = 0;
      fprintf(stderr, "Origin: %s, Command: %s, Num Params: %u\nParams:\n", \
      	      irc_toks.prefix, irc_toks.command, irc_toks.num_params);
      for(count = 0; count < irc_toks.num_params; count++)
      {
      	fprintf(stderr, "%d: %s ", count, irc_toks.params[count]);
      }
      fputs("\n", stderr); */
      
      nickname_len = strchr(irc_toks.prefix, '!') - irc_toks.prefix;
      if(nickname_len < 17 && nickname_len >= 0)
      {
        strncpy(nickname, irc_toks.prefix, nickname_len);
      }
      else
      {
      	strcpy(nickname, "NULL");
      }
      
      /* Is this a Private Message for the bot or room? */
      if(!strcmp(irc_toks.command, "PRIVMSG") && \
        (!strcmp(irc_toks.params[0], argv[3]) || !strcmp(irc_toks.params[0], argv[1])))
      {
        /* There is a command being sent... */
        if(irc_toks.params[1][0] == '%')
        {
          /* Check if the command matches... */
          /* fprintf(stderr, "%s", &irc_toks.params[1][1]); */
          if(!strcmp(&irc_toks.params[1][1], "register"))
          {
            int reg_status;
            reg_status = register_user(user_db, nickname, irc_toks.prefix);
            
            if(reg_status == 1)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :The user DB says "\
              	      "I have already registered you, %s." _NL_, argv[3], nickname);
            }
            else if(reg_status == 0)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Okay %s, I "\
            	      "have registered you into the user DB." _NL_, argv[3], nickname);
            }
            else
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry %s, Something "\
            	      "went wrong while registering you. Error code ?." _NL_, argv[3], nickname);
            }
          }
          else if(!strcmp(&irc_toks.params[1][1], "finish"))
          {
            sock_printf(my_socket, output_buffer, "PRIVMSG %s :Goodbye" _NL_, argv[3]);
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
            /* load_status = -1; */
            if(load_status == 1)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :I can't find "\
                "%s in the database." _NL_, argv[3], user_requested);
            }
            else if(load_status == 0)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Stats for %s" _NL_, argv[3], user_requested);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Total pts: %lu" _NL_, argv[3], user_entry.total_pts);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Total q's answered: %u" _NL_, argv[3], user_entry.q_total);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Q's answered correctly: %u" _NL_, argv[3], user_entry.q_success);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Number of games: %u" _NL_, argv[3], user_entry.num_games);
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :BDB User ID: %d" _NL_, argv[3], user_entry.db_id);
            }
            else
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry %s, "\
                "something went wrong while requesting stats for %s (%d)." _NL_, \
                argv[3], nickname, user_requested, load_status);
            }
          }
          /* else if */
          else
          {
            sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry, %s, I "\
            	      "didn't understand the command." _NL_, argv[3], nickname);
          }
        }
      }
    }
  }
  
  close(my_socket);
  user_db->close(user_db, 0);
  trivia_db->close(trivia_db, 0);
  return EXIT_SUCCESS;
}

/* Todo... add windows version? */
int create_path(char * out_buf, const char * str_pre, const char * str_app)
{
  int str_pre_len;
  
  str_pre_len = strlen(str_pre);
  if(str_pre[str_pre_len - 1] == '/')
  {
    sprintf(out_buf, "%s%s", str_pre, str_app);
  }
  else
  {
    sprintf(out_buf, "%s/%s", str_pre, str_app);
  }
  
  return 0;
}

int create_and_open_db(DB ** db, char * db_path, DBTYPE db_type)
{
  int dbopen_retval;
  
  if(db_create(db, NULL, 0))
  {
    fprintf(stderr, "dbcreate (%s) has failed!\n", db_path);
    return -1;
  }
  
  dbopen_retval = (* db)->open((* db), NULL, db_path, NULL, db_type, \
    DB_CREATE | DB_EXCL, 0644);
  
  if(dbopen_retval == EEXIST)
  {
    debug_fprintf(stderr, "Database exists... using existing version (%s).\n", db_path);
    if((* db)->open((* db), NULL, db_path, NULL, db_type, 0, 0644))
    {
      fprintf(stderr, "Database open failed (%s)!\n", db_path);
      return -2;
    }
  }
  else if(dbopen_retval != 0)
  {
    return -3;
  }
  
  return 0;
}
