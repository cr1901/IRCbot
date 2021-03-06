#ifndef TOKPARSE_H
#define TOKPARSE_H

#define BOT_COMMAND_PREFIX '%'

typedef struct irc_tokens
{
  char * buf;
  char * prefix;
  char * command;
  char * params[15];
  unsigned int bufsiz;
  unsigned char num_params;
}IRC_TOKENS;

int tokenize_irc_line(const char * line_buffer, IRC_TOKENS * tokens);
char * determine_msg_recipient(char * nick_buffer, const char * bot_nick, const IRC_TOKENS * tokens);
/* parse_trivia_answer */

#endif
