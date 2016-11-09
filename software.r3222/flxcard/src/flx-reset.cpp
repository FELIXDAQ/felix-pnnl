/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-reset application        */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

#define APPLICATION_NAME    "flx-reset"
#define APPLICATION_VERSION "1.0.0"

//Globals
FlxCard flxCard;

enum cmd_mode 
{
  UNKNOWN,
  FLUSH,
  RESET,
  REGISTERS_RESET,
  SOFT_RESET,
  ALL
};


/*****************/
void display_help()
/*****************/
{
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Tool to reset different aspects from the card.\n");
  printf("\nCommands:\n");
  printf("Options:\n");
  printf("  flush            Flushes (resets) the main output FIFO toward Wupper  .\n");
  printf("  reset            Resets the whole Wupper_core .\n");
  printf("  registers-reset  Resets the registers to default values .\n");
  printf("  soft-reset       Global application soft reset .\n");
  printf("  all              Do everything .\n");
  printf("Options:\n");
  printf("  -d NUMBER        Use card indicated by NUMBER. Default: 0.\n");
  printf("  -D leve l        Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h               Display help.\n");
  printf("  -V               Display the version number\n");
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int debuglevel, opt, device_number = 0, mode = UNKNOWN;

  if(argc < 2)
  {
    display_help();
    exit(-1);
  }

  while((opt = getopt(argc, argv, "hd:D:V")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
	device_number = atoi(optarg);
	break;
	
      case 'D':
	debuglevel = atoi(optarg);
        DF::GlobalDebugSettings::setup(debuglevel, DFDB_FELIXCARD);
	break;

      case 'h':
	display_help();
	exit(0);
	
      case 'V':
        printf("This is version %s of %s\n", APPLICATION_VERSION, APPLICATION_NAME);
	exit(0);
		
      default:
	fprintf(stderr, "Usage: %s COMMAND [OPTIONS]\nTry %s -h for more information.\n", APPLICATION_NAME, APPLICATION_NAME);
	exit(-1);
    }
  }

  if(optind == argc)
  {
    fprintf(stderr, "No command given\n");
    fprintf(stderr, "Usage: "APPLICATION_NAME" COMMAND [OPTIONS]\nTry "APPLICATION_NAME" -h for more information.\n");
    exit(-1);
  }
 
  if(strcasecmp(argv[optind], "flush") == 0)            mode = FLUSH;
  if(strcasecmp(argv[optind], "reset") == 0)            mode = RESET;
  if(strcasecmp(argv[optind], "soft-reset") == 0)       mode = SOFT_RESET;
  if(strcasecmp(argv[optind], "registers-reset") == 0)  mode = REGISTERS_RESET;
  if(strcasecmp(argv[optind], "all") == 0)              mode = ALL;

  if(mode == UNKNOWN)
  {
    fprintf(stderr, "Unrecognized command '%s'\n", argv[1]);
    fprintf(stderr, "Usage: "APPLICATION_NAME" COMMAND [OPTIONS]\nTry "APPLICATION_NAME" -h for more information.\n");
    exit(-1);
  }

  try
  {
    flxCard.card_open(device_number);
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
    exit(-1);
  }

  try
  {
    if(mode == RESET || mode == ALL)
      flxCard.dma_reset();

    if(mode == FLUSH || mode == ALL)
      flxCard.dma_fifo_flush();

    if(mode == SOFT_RESET || mode == ALL)
      flxCard.dma_soft_reset();

    if(mode == REGISTERS_RESET || mode == ALL)
      flxCard.registers_reset();

    flxCard.card_close();
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
    exit(-1);
  }
  
  exit(0);
}
