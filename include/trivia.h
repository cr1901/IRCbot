#ifndef TRIVIA_H
#define TRIVIA_H

#include "cc_config.h"
#include <stddef.h>

#include <db5/db.h>
#include <jansson.h>

typedef struct trivia_q
{
	json_t * json_rep;
	char * question;
	unsigned char num_answers;
	char ** possible_answers;
	int correct_answer;
	unsigned short point_value; /* Ignore... */
}TRIVIA_Q;

typedef struct trivia_db
{
	json_t * json_rep;
	char * name;
	ptrdiff_t header_length;
	ptrdiff_t jump_table_length;
	unsigned short num_questions;
}TRIVIA_DB;

int open_trivia_db(DB ** trivia_db, const char * db_path);
/* trivia_q * get_trivia_question(DB * trivia_db); */

#endif

