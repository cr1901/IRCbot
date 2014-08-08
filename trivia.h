#ifndef TRIVIA_H
#define TRIVIA_H

#include "cc_config.h"

#include <stddef.h>


typedef struct trivia_q
{
	char * question;
	unsigned char num_answers;
	char ** possible_answers;
	int correct_answer;
	unsigned short point_value; /* Ignore... */
}TRIVIA_Q;

typedef struct trivia_db
{
	char * name;
	ptrdiff_t header_length;
	ptrdiff_t jump_table_length;
	unsigned short num_questions;
}TRIVIA_DB;	

#endif

