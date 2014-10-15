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

static void print_usage();

int main(int argc, char * argv[])
{
  if(argc < 3)
  {
    print_usage();
    return EXIT_FAILURE;
  }
  
  
  
  return EXIT_SUCCESS;
}

static void print_usage()
{
  fprintf(stderr, "Usage: trimport db_file_out json_file_in\n"
    "Extra arguments ignored.\n");
}
