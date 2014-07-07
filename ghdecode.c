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


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <jansson.h>

void print_usage();

int main(int argc, char * argv[])
{
  FILE * in_file, * out_file;
  char * input_buffer;
  
  if(argc < 3)
  {
    print_usage();
    return EXIT_FAILURE;
  }
  
  if(strcmp("-", argv[1]))
  {
    out_file = stdout;
  }
  else
  {
    out_file = fopen(argv[1] , "wb");
  }
  
  if(strcmp("-", argv[2]))
  {
    in_file = stdin;
  }
  else
  {
    in_file = fopen(argv[2], "rb");
  }
  
  input_buffer = malloc(BUFSIZ);
  
  if(out_file == NULL || in_file == NULL || input_buffer == NULL)
  {
    fprintf(stderr, "Error: Could not open either input file, or output file, or\n" \
    	    "malloc() failed to allocate buffer for input file!\n");
    return EXIT_FAILURE;
  }
  
  
  return EXIT_SUCCESS;
}

void print_usage()
{
  fprintf(stderr, "ghdecode ROM extractor program\n" \
  	  "Usage: ghdecode outfile infile\n" \
  	  "Use '-' to read/write from/to standard streams.\n" \
  	  "Extra arguments ignored.\n");
}
