/* Decode Greyhound Electronics EPROMs into format suitable for trivia database
storage. Should work on any platform with an ANSI-C compiler with 8-bit char size and
libjansson installed (Don't feel like writing my own JSON generator :P) */

/* This program is quick and dirty... it could stand to be improved to reduce the
chance of buffer overlow and more efficient memory allocation. */

/* Greyhound Trivia EPROM format... 0x?? is hex literal
Name: UT[A-z]*0xFF: Varies
Series: 0x20#[0-9]0xFF: Varies
Number of Questions: 1 byte
Jump Table to questions : 2 *s Number of Questions... biased by 0x2000, presumably
  because that's where the EPROMs live in the Z80 address space.
  
Question Format:
Question string literal (ASCII), terminated with 0xFF: Varies
Bit mask indicating answer- log2(0x??) = answer position
3 Consecutive Answers, 0xFF terminated: Varies.
0x00 following a question and 3 answers (meaning 0xFF): End of EPROM.


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #include "trivia.h" */
#ifndef POSIX_MAKE
  #include "cc_config.h"
#endif

#include <jansson.h>

void print_usage();
void parse_args(FILE ** fp_out, FILE ** fp_in, const char * tok_out, const char * tok_in);
int create_header(FILE * fp_in, char * in_buffer, json_t * json_rep);
int create_question(FILE * fp_in, char * in_buffer, char * out_buffer, json_t * json_rep);
char * format_question(char * out_buffer, const char * in_buffer, unsigned int out_size, unsigned int in_size);
int mini_log2(unsigned int n);

int main(int argc, char * argv[])
{
  FILE * in_file, * out_file;
  char * input_buffer, * output_buffer;
  json_t * json_db, * json_qs;
  int file_offset = 0, chars_parsed = 0;
  
  
  /* Argument parsing... */
  if(argc < 3)
  {
    print_usage();
    return EXIT_FAILURE;
  }
  
  parse_args(&out_file, &in_file, argv[1], argv[2]);
  
  /* sanity checks */
  input_buffer = malloc(BUFSIZ);
  output_buffer = malloc(BUFSIZ);
  
  if(out_file == NULL || in_file == NULL || \
  	  input_buffer == NULL || output_buffer == NULL)
  {
    fprintf(stderr, "Error: Could not open either input file, or output file, or\n" \
    	    "malloc() failed to allocate buffer for input/output files!\n");
    return EXIT_FAILURE;
  }
  
  
  /* Initial processing... */
  if((json_db = json_object()) == NULL)
  {
    fprintf(stderr, "Error occurred while creating JSON object!\n");
    return EXIT_FAILURE;
  }
  
  /* We will assume the header doesn't require more than BUFSIZ... */
  if((chars_parsed = create_header(in_file, input_buffer, json_db)) < 0)
  {
    fprintf(stderr, "Error occurred while extracting DB header! (%d)\n", chars_parsed);
    return EXIT_FAILURE;
  }
  
  fseek(in_file, chars_parsed, SEEK_SET);
  json_qs = json_array();
  
  file_offset = chars_parsed;
  /* create_question(in_file, input_buffer, json_qs); */
  while((chars_parsed = create_question(in_file, input_buffer, output_buffer, json_qs)) > 0)
  {
    file_offset += chars_parsed;
    fseek(in_file, file_offset, SEEK_SET);	  
  }
  
  if(chars_parsed < 0)
  {
    fprintf(stderr, "Error occurred while extracting questions! (%d)\n", chars_parsed);
    return EXIT_FAILURE;  
  }
  
  
  json_object_set(json_db, "questions", json_qs);
  json_dumpf(json_db, out_file, JSON_INDENT(2) | JSON_PRESERVE_ORDER);
  
  return EXIT_SUCCESS;
}

void print_usage()
{
  fprintf(stderr, "ghdecode ROM extractor program\n" \
  	  "Usage: ghdecode outfile infile\n" \
  	  "Use '-' to read/write from/to standard streams.\n" \
  	  "Extra arguments ignored.\n");
}

void parse_args(FILE ** fp_out, FILE ** fp_in, const char * tok_out, const char * tok_in)
{
  if(!strcmp("-", tok_out))
  {
    (* fp_out) = stdout;
  }
  else
  {
    (* fp_out) = fopen(tok_out , "wb");
  }
  
  if(!strcmp("-", tok_in))
  {
    (* fp_in) = stdin;
  }
  else
  {
    (* fp_in) = fopen(tok_in, "rb");
  }
}

int create_header(FILE * fp_in, char * in_buffer, json_t * json_rep)
{
  char * name_ptr_end, * series_ptr_end;
  int next_offset, num_qs, header_len, jump_len, in_buffer_left;
  
  next_offset = fread(in_buffer, 1, BUFSIZ, fp_in);
  
  if(next_offset < BUFSIZ && !feof(fp_in))
  {
    return -1;
  }

  if((name_ptr_end = memchr(in_buffer, 0xFF, BUFSIZ)) == NULL)
  {
    return -2;
  }
  
  in_buffer_left = BUFSIZ - (name_ptr_end - in_buffer + 1);
  if((series_ptr_end = memchr(name_ptr_end + 1, 0xFF, in_buffer_left)) == NULL)
  {
    return -3;
  }
  
  /* Construct a name string. */
  (* series_ptr_end) = '\0'; /* Create a null terminator. */
  memmove(name_ptr_end, name_ptr_end + 1, (series_ptr_end) - (name_ptr_end + 1) + 1);
  num_qs = (unsigned int) (* (series_ptr_end + 1)); /* Skip checking if series_ptr_end == BUFSIZ. */
  jump_len = num_qs * 2;
  header_len = ((series_ptr_end + 1) - in_buffer) + 1;
  
  json_object_set(json_rep, "name", json_string(in_buffer));
  json_object_set(json_rep, "series_id", json_string("gtsers1"));
  json_object_set(json_rep, "num_qs", json_integer(num_qs));
  json_object_set(json_rep, "jump_len", json_integer(jump_len));
  json_object_set(json_rep, "header_len", json_integer(header_len));
  
  return jump_len + header_len;
}

int create_question(FILE * fp_in, char * in_buffer, char * out_buffer, json_t * json_rep)
{
  int next_offset, count, in_buffer_left, answer;
  const int num_ans = 3;
  json_t * json_curr_q, * json_answers;
  char * q_end, * ans_begin, * ans_end;
  
  
  next_offset = fread(in_buffer, 1, BUFSIZ, fp_in);
  if(next_offset < BUFSIZ && !feof(fp_in))
  {
    return -1;
  }
  
  
  json_curr_q = json_object();
  if(json_curr_q == NULL)
  {
    return -2;
  }
  
  if((q_end = memchr(in_buffer, 0xFF, BUFSIZ)) == NULL)
  {
    return -3;
  }
  
  
  
  (* q_end) = '\0';
  answer = mini_log2((unsigned int) (* (q_end + 1))) + 1;
  
  if(answer <= 0)
  {
    return -4;
  }
  
  ans_begin = (q_end + 2);
  in_buffer_left = BUFSIZ - (ans_begin - in_buffer + 1);
  
  json_answers = json_array();
  for(count = 0; count < num_ans; count++)
  {
    if((ans_end = memchr(ans_begin, 0xFF, in_buffer_left)) == NULL)
    {
      return -5;
    }
    
    (* ans_end) = '\0';
    
    json_array_append(json_answers, json_string(format_question(out_buffer, ans_begin, BUFSIZ, BUFSIZ)));
    ans_begin = ans_end + 1;
    in_buffer_left = BUFSIZ - (ans_begin - in_buffer + 1);
  }
  
  /* fprintf(stderr, "Parsing %s\n", in_buffer); */
  json_object_set(json_curr_q, "question", json_string(format_question(out_buffer, in_buffer, BUFSIZ, BUFSIZ)));
  json_object_set(json_curr_q, "answer_index", json_integer(answer));
  json_object_set(json_curr_q, "num_answers", json_integer(3)); /* Always 3 for Greyhound Trivia? */
  json_object_set(json_curr_q, "possible_answers", json_answers); 
  json_array_append(json_rep, json_curr_q);
  
  /* 0x00 means END of EPROM. */
  return ((* ans_begin) == 0) ? 0 : (ans_begin - in_buffer);
  /* return ((* ans_begin) == 0) ? 0 : ans_begin; */ /* Causes all sorts of fun
  problems... try with SEEK_CUR in loop! */
}


/* Returns output buffer for convenience. */
char * format_question(char * out_buffer, const char * in_buffer, unsigned int out_size, unsigned int in_size)
{
  /* out_size and in_size actually aren't required for now, but are kept
  in case that changes in the future. */
  char * newline_loc, * out_pos;
  const char * in_pos;
  
  out_pos = out_buffer;
  in_pos = in_buffer;
  newline_loc = strchr(in_buffer, '\n');
  
  while(newline_loc != NULL)
  {
    int chars_parsed = newline_loc - in_pos;
    
    memcpy(out_pos, in_pos, chars_parsed);
    out_pos = out_pos + chars_parsed;
    (* out_pos) = ' '; /* Replace a newline with a space. */
    out_pos++;
    in_pos += (chars_parsed + 1); /* Go past the newline found. */
    newline_loc = strchr(in_pos, '\n');
  }
  
  /* Copy the rest of the buffer, including null. 
  Guaranteed to be null-terminated, so skip check. */
  strcpy(out_pos, in_pos);

  return out_buffer;	
}

int mini_log2(unsigned int n)
{
  int retval;
  switch(n)
  {
    case 1:
      retval = 0;
      break;
    case 2:
      retval = 1;
      break;
    case 4:
      retval = 2;
      break;
    default:
      retval = -1;
  }
  return retval;
}
