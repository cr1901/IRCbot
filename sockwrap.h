#ifndef SOCKWRAP_H
#define SOCKWRAP_H

typedef struct readline_state
{
  char * buf;
  char * buf_offset;
  char * newline_loc;
  unsigned int bufsiz;
}READLINE_STATE;


typedef int sock_id;

int get_a_socket_and_connect(sock_id * sock, const char * hostname, const char * port);
int read_line_from_socket(sock_id sock, char * line_buffer, unsigned int line_bufsiz, READLINE_STATE * temp_state);

/* printf wrappers */
int sock_printf(sock_id sock, char * buff, /* int buflen, */ const char * fmt, ...);

#endif
