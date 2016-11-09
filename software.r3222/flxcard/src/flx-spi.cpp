/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-spi application          */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

#define APPLICATION_NAME    "flx-spi"
#define APPLICATION_VERSION "1.0.0"

enum spi_mode 
{
  SPI_UNKNOWN,
  SPI_READ,
  SPI_WRITE,
  SPI_LIST
};

//Globals
FlxCard flxCard;

/*****************/
void display_help()
/*****************/
{
  spi_device_t* devices;
  
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Transmit or receive data from a SPI device.\n");
  printf("DISCLAIMER: PLEASE NOTE THIS TOOL IS UNDER DEVELOPMENT, \n\t\tBUGS AND SECURITY ISSUES ARE EXPECTED.\n");
  printf("General options:\n");
  printf("  -d NUMBER                    Use card indicated by NUMBER. Default: 0.\n");
  printf("  -D level                     Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h                           Display help.\n");
  printf("  -v                           Display verbose\n");
  printf("  -V                           Display the version number\n");
  printf("Commands:\n");
  printf("  list                                          List available devices.\n");
  printf("  read   DEVICE_NAME     REG_ADDRESS            Read a register from a SPI device.\n");
  printf("  write  DEVICE_NAME     REG_ADDRESS   DATA     Write a register from a SPI device.\n");
  printf("\nList of available devices: \n");
  printf("Device              Model                  Address\n");
  printf("==============================================================================\n");

  for(devices = spi_devices; devices->name != NULL; devices++)
    printf("%-16s    %-25s   0x%x\n", devices->name, devices->description, devices->address);

  printf("\n");
}


/******************************/
static void str_upper(char *str)
/******************************/
{
  do
  {
    *str = toupper((u_char) *str);
  } while (*str++);
}


/*****************/
void spi_list(void)
/*****************/
{
  spi_device_t *devices;
  
  printf("\nList of available devices: \n");
  printf("Device              Model                  Address\n");
  printf("==============================================================================\n");

  for(devices = spi_devices; devices->name != NULL; devices++)
    printf("%-16s    %-25s     0x%x\n", devices->name, devices->description, devices->address);

  printf("\n");
}


/*********************************************************************/
static int spi_read(const char *device, const char *regis, int verbose)
/*********************************************************************/
{
  spi_device_t *devices;
  int found = 0;
  u_int value = 0;
  u_short reg = 0;
  char *p_aux = NULL, *upper = strdup(device);
  
  str_upper(upper);

  reg = (u_short)strtoul(regis, &p_aux, 0);
  if(*p_aux != '\0')
  {
    printf("ERROR: SPI_DEVICE_ERROR_INVALID_REGISTER\n");
    return(-1);
  }

  //Search using name.
  for(devices = spi_devices; devices->name != NULL && found == 0; devices++)
  {;
    if(strcmp(upper, devices->name) == 0)
    {
      if(verbose)
        printf("Device found!\n");

      //Reading value.
      try
      {
        value = flxCard.spi_read(reg);
      }
      catch(FlxException ex)
      {
	std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
	exit(-1);
      }
            
      printf("Register 0x%x = 0x%x\n", reg, value);
      if(verbose)
        printf("Register 0x%02x from device 0x%02x = 0x%02x\n", reg, devices->address, value);
      found = 1;
    }
  }

  if(!found)
  {
    printf("ERROR: SPI_DEVICE_ERROR_NOT_EXIST\n");
    return(-1);
  }
    
  free(upper);
  return(0);
}


/***************************************************************************************/
static int spi_write(const char *device, const char *regis, const char *dat, int verbose)
/***************************************************************************************/
{
  spi_device_t* devices;
  int found = 0;
  u_short reg = 0;
  u_int data = 0;
  char *p_aux = NULL, *upper = strdup(device);

  str_upper(upper);

  reg = (u_short)strtoul(regis, &p_aux, 0);
  if(*p_aux != '\0')
  {
    printf("ERROR: SPI_DEVICE_ERROR_INVALID_REGISTER\n");
    return(-1);
  }
  
  data = (u_int)strtoul(dat, &p_aux, 0);
  if(*p_aux != '\0')
  {
    printf("ERROR: SPI_DEVICE_ERROR_INVALID_DATA\n");
    return(-1);
  }

  //Search using name.
  for(devices = spi_devices; devices->name != NULL && found == 0; devices++)
  {
    if(strcmp(upper, devices->name) == 0)
    {
      if(verbose)
        printf("Device found!\n");

      //Writing value.
      try
      {
        flxCard.spi_write(reg, data);
      }
      catch(FlxException ex)
      {
	std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
	exit(-1);
      }
      
      found = 1;
    }
  }

  if(!found)
  {
    printf("ERROR: SPI_DEVICE_ERROR_NOT_EXIST\n");
    return(-1);
  }
  
  free(upper);
  return(0);
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int debuglevel, opt, device_number = 0, mode = SPI_UNKNOWN, verbose = 0, cont = 0;

  if(argc < 2)
  {
    display_help();
    exit(-1);
  }

  while ((opt = getopt(argc, argv, "hvd:D:V")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
	device_number = atoi(optarg);
	cont = cont + 2;
	break;
	
      case 'h':
	display_help();
	exit(0);

      case 'D':
	debuglevel = atoi(optarg);
        DF::GlobalDebugSettings::setup(debuglevel, DFDB_FELIXCARD);
	break;

      case 'v':
	verbose = 1;
	cont++;
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
    exit(-1);
  }

  if(strcasecmp(argv[optind], "read") == 0)   mode = SPI_READ;
  if(strcasecmp(argv[optind], "write") == 0)  mode = SPI_WRITE;
  if(strcasecmp(argv[optind], "list") == 0)   mode = SPI_LIST;
  if(mode == SPI_UNKNOWN)
  {
    fprintf(stderr, "Unrecognized command '%s'\n", argv[1]);
    fprintf(stderr, "Usage: "APPLICATION_NAME" COMMAND [OPTIONS]\nTry "APPLICATION_NAME" -h for more information.\n");
    exit(-1);
  }

  try
  {
    flxCard.card_open(device_number);

    switch(mode)
    {
      case SPI_READ:
	if(argc == (4 + cont))
          spi_read(argv[2 + cont], argv[3 + cont], verbose);
	else 
          fprintf(stderr, APPLICATION_NAME": error: Number of parameters incorrect\n");
	break;

      case SPI_WRITE:
	if(argc == (5 + cont))
          spi_write(argv[2 + cont], argv[3 + cont], argv[4 + cont], verbose);
	else 
          fprintf(stderr, APPLICATION_NAME": error: Number of parameters incorrect\n");
	break;

      case SPI_LIST:
	spi_list();
	break;
	
      default: 
	fprintf(stderr, "Usage: %s COMMAND [OPTIONS]\nTry %s -h for more information.\n", APPLICATION_NAME, APPLICATION_NAME);
	exit(-1);
    }
    flxCard.card_close();
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
    exit(-1);
  }

  return 0;
}
