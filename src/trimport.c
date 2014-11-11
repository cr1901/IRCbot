#include "cc_config.h"

/* Import a trivia DB json file into the global trivia database. Takes
advantage of being able to store multiple DBs in a single file. */

#include <stdio.h>
#include <stdlib.h>


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

#include <jansson.h>

#include "trivia.h"

static void print_usage();
static int add_qs_to_db(const char * db_path, const char * qs_path);

int main(int argc, char * argv[])
{
  DB * trivia_db;
  int count;
  
  if(argc < 3)
  {
    print_usage();
    return EXIT_FAILURE;
  }
  
  
  for(count = 2; count < argc; count++)
  {
    if(add_qs_to_db(argv[1], argv[count]))
    {
      return EXIT_FAILURE;
    }
  }  
  
  return EXIT_SUCCESS;
}

static void print_usage()
{
  fprintf(stderr, "Usage: trimport db_file_out json_file_in [json_file_in]\n");
}


int add_qs_to_db(const char * db_path, const char * qs_path)
{
  DB * trivia_db;
  json_t * json_rep;
  
  if(open_trivia_db(&trivia_db, db_path) < 0)
  {
  	  return -1;
  }
  
  /* if(
  */
  
}
