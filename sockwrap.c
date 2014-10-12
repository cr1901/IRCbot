/* POSIX headers */
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* ANSI headers */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "debug.h"
#include "sockwrap.h"


int get_a_socket_and_connect(sock_id * sock, const char * hostname, const char * port)
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
  
  if(fcntl((* sock), F_SETFL, (fcntl((* sock), F_GETFL, 0) | O_NONBLOCK) < 0))
  {
    debug_fprintf(stderr, "Attempt to set socket to non-blocking failed!\n");
    return -3;
  }

  freeaddrinfo(my_addrinfo);
  return 0;
}

/* Violoncello: What exactly did you mean by "I would have expected the code 
to just replace '\n' with '\0' and return buf_offset, rather than mess around 
with copying data into another buffer */
int read_line_from_socket(sock_id sock, char * line_buffer, unsigned int line_bufsiz, READLINE_STATE * temp_state)
{
  int chars_read;
  int newline_found = 0, input_buffer_full = 0, output_buffer_too_small = 0, \
    remote_socket_closed = 0, read_timeout = 0;
  char * newline_loc;
  unsigned int used_elements;
  
  /* What is a good way to guard against the case where buf_offset < buf? */
  used_elements = temp_state->buf_offset - temp_state->buf;
  debug_fprintf(stderr, "used_elements: %u\n", used_elements);
  while(!newline_found && !input_buffer_full && !remote_socket_closed && !read_timeout)
  {
    /* newline_loc will be at most equal to the ptr address temp_state->buf_offset - 1 */
    newline_loc = memchr(temp_state->buf, 0x0A, used_elements);
    
    if(used_elements >= temp_state->bufsiz && newline_loc == NULL)
    {
      fprintf(stderr, "Warning: Input socket buffer is full and no newline is present: %d\n", __LINE__);
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
        fd_set rfds;
        struct timeval tv;
        int select_retval;
        
        
        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);
        tv.tv_sec = 300;
        tv.tv_usec = 0;
        
        select_retval = select(sock + 1, &rfds, NULL, NULL, &tv);
        if(select_retval > 0)
        {        
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
        else if(select_retval == 0)
        {
          debug_fprintf(stderr, "Timeout in select() call.\n");
          read_timeout = 1;
        }
        else /* Greater-than-equal positive one tests all possible success cases. */
        {
          fprintf(stderr, "Socket error in select() call: %s\n", strerror(errno));
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
  else if(read_timeout)
  {
    return -4;
  }
  else
  {
    return 0;
  }
}

int sock_printf(sock_id sock, char * buff, const char * fmt, ...)
{
  int buff_size;
  va_list args;
  
  va_start(args, fmt);
  buff_size = vsprintf(buff, fmt, args);
  write(sock, buff, buff_size);
  va_end(args);
  return buff_size;	
}

