#include "cc_config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

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
#include "users.h"

#define _NL_ "\r\n"

/* char ** commands = { \ */
char * commands[] = { "NICK cr1901_IRCbot" _NL_, \
                      "USER cr1901 . . :This is a bot" _NL_, \
                      "JOIN #higan" _NL_
                    };

char * quotes[] = { "Hello all you beautiful people :D! It's time for some trivia!\n" _NL_, \
                  };
                  
typedef struct readline_state
{
  char * buf;
  char * buf_offset;
  unsigned int bufsiz;
}READLINE_STATE;

typedef struct irc_tokens
{
  char * buf;
  char * prefix;
  char * command;
  char * params[15];
  unsigned int bufsiz;
  unsigned char num_params;
}IRC_TOKENS;


int get_a_socket_and_connect(int * sock, const char * hostname, const char * port);
int read_line_from_socket(int sock, char * line_buffer, unsigned int line_bufsiz, READLINE_STATE * temp_state);
int tokenize_irc_line(const char * line_buffer, IRC_TOKENS * tokens);
int create_path(char * out_buf, const char * str_pre, const char * str_app);
int create_and_open_db(DB ** db, char * db_path, DBTYPE db_type);

/* printf wrappers */
int sock_printf(int sock, char * buff, /* int buflen, */ const char * fmt, ...);
int debug_fprintf(FILE * fp, const char * fmt, ...);


int main(int argc, char * argv[])
{
  int my_socket, done = 0;
  char * line_buffer, * output_buffer, * db_path;
  READLINE_STATE socket_buf;
  IRC_TOKENS irc_toks;
  DB * user_db, * trivia_db;
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
  
  
  
  sock_printf(my_socket, output_buffer, "USER %s . . :This is a bot programmed by William D. Jones" _NL_, argv[1]);
  sock_printf(my_socket, output_buffer, "NICK %s" _NL_, argv[1]);
  sock_printf(my_socket, output_buffer, "JOIN %s" _NL_, argv[3]);
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
      /* debug_fprintf(stderr, "PING received... PONG not sent (debugging).\n");*/
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
            
            /* load_status = load_user_entry(user_db, user_requested, &user_entry); */
            load_status = -1;
            if(load_status == 1)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :I can't find "\
                "%s in the database." _NL_, argv[3], user_requested);
            }
            else if(load_status == 0)
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Stats for %s" _NL_, argv[3], user_requested);
              
            }
            else
            {
              sock_printf(my_socket, output_buffer, "PRIVMSG %s :Sorry %s, "\
                "something went wrong while requesting stats for %s." _NL_, \
                argv[3], nickname, user_requested);
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


int get_a_socket_and_connect(int * sock, const char * hostname, const char * port)
{
  /* struct addrinfo * my_addrinfo, * curr_addrinfo, hints; */
  struct addrinfo hints, * my_addrinfo, * curr_addrinfo;
  int retval;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  retval = getaddrinfo(hostname, port, &hints, &my_addrinfo);

  if(retval != 0)
  {
    debug_fprintf(stderr, "getaddrinfo failed with error code %d.\n", retval);
    return -1;
  }

  for(curr_addrinfo = my_addrinfo; curr_addrinfo != NULL; curr_addrinfo = curr_addrinfo->ai_next)
  {
    debug_fprintf(stderr, "Attempting to connect...\n");
    if(((* sock) = socket(PF_INET, SOCK_STREAM, 0)) == -1)
    {
      continue;
    }

    if(connect((* sock), curr_addrinfo->ai_addr, curr_addrinfo->ai_addrlen) == 0)
    {
      break;
    }

    close((* sock));
  }

  if(curr_addrinfo == NULL)
  {
    debug_fprintf(stderr, "Socket creation failed!\n");
    return -2;
  }

  freeaddrinfo(my_addrinfo);
  return 0;
}


/* Violoncello: What exactly did you mean by "I would have expected the code 
to just replace '\n' with '\0' and return buf_offset, rather than mess around 
with copying data into another buffer */
int read_line_from_socket(int sock, char * line_buffer, unsigned int line_bufsiz, READLINE_STATE * temp_state)
{
  int chars_read;
  int newline_found = 0, input_buffer_full = 0, output_buffer_too_small = 0, remote_socket_closed = 0;
  char * newline_loc;
  unsigned int used_elements;
  
  /* What is a good way to guard against the case where buf_offset < buf? */
  used_elements = temp_state->buf_offset - temp_state->buf;
  debug_fprintf(stderr, "used_elements: %u\n", used_elements);
  while(!newline_found && !input_buffer_full && !remote_socket_closed)
  {
    /* newline_loc will be at most equal to the ptr address temp_state->buf_offset - 1 */
    newline_loc = memchr(temp_state->buf, 0x0A, used_elements);
    
    if(used_elements >= temp_state->bufsiz)
    {
      fprintf(stderr, "Warning: Input socket buffer is full: %d\n", __LINE__);
      input_buffer_full = 1;
    }
    else
    {
      if(newline_loc != NULL)
      {
      	/* This guard probably isn't necessary, but IRC wants CRLF line endings...
      	check for the LF first, so that we don't accidentally read out of bounds. */
        /* if((* (newline_loc - 1)) == 0x0D) */
        {
          /* The difference between newline_loc and temp_state->buf is inclusive
          to include the newline character itself. */
          unsigned int line_length, num_chars_to_xfer;
          
          line_length = newline_loc - temp_state->buf + 1;
          debug_fprintf(stderr, "We found a newline! used_elements: %u, line_length: %u\n", used_elements, line_length);
          num_chars_to_xfer = line_length < line_bufsiz  ? line_length : (line_bufsiz - 1);
          
          /* chars_transferred = snprintf(line_buffer, num_chars_to_xfer, "%s", temp_state->buf); */
          memcpy(line_buffer, temp_state->buf, num_chars_to_xfer);
          line_buffer[num_chars_to_xfer] = '\0'; /* Add null terminator. */
          newline_found = 1;
          /* If used_elements == line_bufsiz, the null terminator will truncate
          the last character. */
          if(line_length >= line_bufsiz)
          {
            fprintf(stderr, "Warning: Output buffer could not hold entire line..." \
            	    "it has been truncated: %d\n", __LINE__);
            output_buffer_too_small = 1;
          }
          else
          {
            used_elements -= line_length;
            temp_state->buf_offset -= line_length;  
            memmove(temp_state->buf, (newline_loc + 1), used_elements);
            debug_fprintf(stderr, "Readline state has been memmoved: used_elements: %u, temp_state->buf_offset: %p\n", used_elements, temp_state->buf_offset);
          }
        }
      }
      else
      {
        /* Fill a socket buffer with input data, up to temp_state->buf array index (bufsiz - 1).
        temp_state->buf_offset points to "next free". temp_state->buf will always
        contain the first element after the previously successfully-parsed line. */
        chars_read = read(sock, temp_state->buf_offset, (temp_state->bufsiz - used_elements));
        if(chars_read > 0)
        {
          debug_fprintf(stderr, "We read something- chars_read: %u, used_elements: %u, temp_state->buf_offset: %p\n", chars_read, used_elements, temp_state->buf_offset);
          temp_state->buf_offset += chars_read;
          used_elements = temp_state->buf_offset - temp_state->buf;
        }
        else if(chars_read == 0)
        {
          debug_fprintf(stderr, "Remote socket has closed.\n");
          remote_socket_closed = 1;
        }
        else
        {
          fprintf(stderr, "Socket error in read() call: %s\n", strerror(errno));
        }
      }
    }
  }
  
  if(input_buffer_full)
  {
    return -1;
  }
  else if(output_buffer_too_small)
  {
    return -2;
  }
  else if(remote_socket_closed)
  {
    return -3;
  }
  else
  {
    return 0;
  }
}

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


int sock_printf(int sock, char * buff, const char * fmt, ...)
{
  int buff_size;
  va_list args;
  
  va_start(args, fmt);
  buff_size = vsprintf(buff, fmt, args);
  write(sock, buff, buff_size);
  va_end(args);
  return buff_size;	
}

int debug_fprintf(FILE * fp, const char * fmt, ...)
{
  #ifndef NDEBUG_MESSAGES
  int chars_written = 0;
  va_list args;
  
  va_start(args, fmt);
  chars_written = vfprintf(fp, fmt, args);
  va_end(args);
  return chars_written;
  #else
  return 0;
  #endif
}
