/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-irq-test application     */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

#define APPLICATION_NAME    "flx-irq-test"
#define APPLICATION_VERSION "1.0.0"

//Globals
FlxCard flxCard;


/*****************/
void display_help()
/*****************/
{
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Waits for an interrupt.\n\n");
  printf("Options:\n");
  printf("  -i NUMBER      Wait for the interrupt indicated by NUMBER. Default: 5.\n");
  printf("  -d NUMBER      Use card indicated by NUMBER. Default: 0.\n");
  printf("  -D level       Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h             Display help.\n");
  printf("  -V             Display the version number\n");
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int debuglevel, device_number = 0, irq_id = 5, opt;

  while((opt = getopt(argc, argv, "hi:d:D:V")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
	device_number = atoi(optarg);
	break;
	
      case 'i':
	irq_id = atoi(optarg);
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

  try
  {
    flxCard.card_open(device_number);
    flxCard.irq_init();
    flxCard.irq_enable_all();

    printf("Waiting for interrupt %d...", irq_id);
    fflush(stdout);
    flxCard.irq_wait(irq_id);
    printf(" ok!\n");

    flxCard.irq_disable_all();
    flxCard.card_close();
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
    exit(-1);
  }

  exit(0);
}
