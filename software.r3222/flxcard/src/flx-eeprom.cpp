/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-eeprom application       */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stddef.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

#define APPLICATION_NAME    "flx-eeprom"
#define APPLICATION_VERSION "1.0.0"

//Globals
FlxCard flxCard;

enum eeprom_mode 
{
  EEPROM_UNKNOWN,
  EEPROM_READ,
  EEPROM_WRITE
};


/************************/
static void display_help()
/************************/
{
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Read or write data from/to a eeprom device.\n\n");
  printf("General options:\n");
  printf("  -d NUMBER                       Use card indicated by NUMBER. Default: 0.\n");
  printf("  -D level                        Configure debug outout at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h                              Display help.\n");
  printf("  -V                              Display the version number\n");
  printf("\nCommands:\n");
  printf("  r[ead]   [REG_ADDRESS]          Read a eeprom starting from a specific register until NULL. (Default address: 0)\n");
  printf("  w[rite]  [REG_ADDRESS]   DATA   Write a eeprom starting from a specific register. (Default address: 0)\n");
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int debuglevel, opt, device_number = 0, mode = EEPROM_UNKNOWN, cont = 0;
  u_long reg = 0;
  char *p_aux = NULL;

  if(argc < 2)
  {
    display_help();
    exit(-1);
  }

  while ((opt = getopt(argc, argv, "hd:D:V")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
	device_number = atoi(optarg);
	cont = cont + 2;
	break;

      case 'D':
	debuglevel = atoi(optarg);
        DF::GlobalDebugSettings::setup(debuglevel, DFDB_FELIXCARD);
	break;

      case 'h':
	display_help();
	exit(0);
      break;

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
    exit(EXIT_FAILURE);
  }

  if(strcasecmp(argv[optind], "read") == 0)  mode = EEPROM_READ;
  if(strcasecmp(argv[optind], "r") == 0)     mode = EEPROM_READ;
  if(strcasecmp(argv[optind], "write") == 0) mode = EEPROM_WRITE;
  if(strcasecmp(argv[optind], "w") == 0)     mode = EEPROM_WRITE;

  if(mode == EEPROM_UNKNOWN)
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
  
  //card_model = flxCard.card_model();
  //if(card_model != FLX_710 && card_model != FLX_709)
  //{
  //  fprintf(stderr, APPLICATION_NAME" error: Card model not recognized\n");
  //  exit(-1);
  //}

  switch(mode)
  {
    case EEPROM_READ:

    try
    {
      if(argc == (2 + cont))
	flxCard.eeprom_read();
      else 
      {
	if(argc == (3 + cont))
	{
          reg = strtoul(argv[2 + cont], &p_aux, 0);
          if(*p_aux != '\0')
	  {
            printf("ERROR: Invalid register\n");
	    exit(-1);
	  }
	  else 
            flxCard.eeprom_read();
	} 
	else 
          fprintf(stderr, APPLICATION_NAME": error: Number of parameters incorrect\n");
      }
    }
    catch(FlxException ex)
    {
      std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
      exit(-1);
    }
    break;


    case EEPROM_WRITE:
    try
    {
      if(argc == (3 + cont))
	flxCard.eeprom_write(0, argv[2 + cont]);
      else 
      {
	if(argc == (4 + cont))
	{
          reg = strtoul(argv[2 + cont], &p_aux, 0);
          if(*p_aux != '\0')
	  {
            printf("ERROR: Invalid register\n");
	    exit(-1);
	  }
	  else 
            flxCard.eeprom_write(0, argv[3 + cont]);
	} 
	else
          fprintf(stderr, APPLICATION_NAME": error: Number of parameters incorrect\n");
      }
    }
    catch(FlxException ex)
    {
      std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
      exit(-1);
    }
    break;

    default: 
      fprintf(stderr, "Usage: %s COMMAND [OPTIONS]\nTry %s -h for more information.\n", APPLICATION_NAME, APPLICATION_NAME);
      exit(-1);
  }

  try
  {
    flxCard.card_close();
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
    exit(-1);
  }

  exit(0);
}
