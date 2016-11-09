/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-config application       */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

#define APPLICATION_NAME    "flx-config"
#define APPLICATION_VERSION "1.0.0"

//Globals
FlxCard flxCard;


enum cmd_mode
{
  CMD_UNKNOWN,
  CMD_LIST,
  CMD_REGISTERS,
  CMD_SET,
  CMD_LOAD,
  CMD_STORE
};


/*****************/
void display_help()
/*****************/
{
  printf("Usage: %s COMMAND [OPTIONS]\n", APPLICATION_NAME);
  printf("\nCommands:\n");
  printf("  registers <GROUP>  List card configuration.\n");
  printf("                     If GROUP is not defined all registers will be listed\n");
  printf("                     Supported strings for GROUP currently are: GEN, CRC, CRM, GEC, GWC, GWM and HKC\n");
  printf("  list               List card bitfields.\n");
  printf("  set KEY=VALUE      Set option KEY to VALUE. Multiple settings may be given. If no value is given, 0 is assumed.\n");
  printf("  store FILENAME     Store current configuration in the given file.\n");
  printf("  load FILENAME      Load the configuration in the given file. The file format used is the one produced by the 'store' command.\n");
  printf("\nOptions:\n");
  printf("  -d NUMBER          Use card indicated by NUMBER. Default: 0.\n");
  printf("  -D level           Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h                 Display help.\n");
  printf("  -V                 Display the version number\n");
}


/**************************************/
static u_long split_key_value(char *str)
/**************************************/
{
  char *pos = strchr(str, '=');
  if(pos == NULL)
  {
    printf("ERROR: invalid string passed to <set> option\n");
    exit(-1);
  }
  *pos = '\0';
  return strtoll(pos + 1, NULL, 0);
}


/****************************/
static void print_header(void)
/****************************/
{
  printf("Offset   RW   Name                          Value               Description\n");
  printf("================================================================================================================\n");
}


/**************************************************************************************************************/
static void print_register(u_long address, u_int flags, const char *name, u_long value, const char *description)
/**************************************************************************************************************/
{
  if(flags & REGMAP_REG_READ)
  {
    printf("0x%04lx  [%c%c]  %-29s 0x%016lx \t%s\n",
      address,
      flags & REGMAP_REG_READ ? 'R' : ' ',
      flags & REGMAP_REG_WRITE ? 'W' : ' ',
      name,
      value,
      description);
  }
  else
  {
    printf("0x%04lx  [%c%c]  %-24s                  - \t%s\n",
      address,
      flags & REGMAP_REG_READ ? 'R' : ' ',
      flags & REGMAP_REG_WRITE ? 'W' : ' ',
      name,
      description);
  }
}


/************************/
static void cmd_list(void)
/************************/
{
  regmap_bitfield_t *bf;
  u_long value = 0;

  print_header();

  for(bf = regmap_bitfields; bf->name != NULL; bf++)
  {
    if(bf->flags & REGMAP_REG_READ)
    {
      flxCard.cfg_get_option(bf->name, &value);
      print_register(bf->address, bf->flags, bf->name, value, bf->description);
    }
    else
      print_register(bf->address, bf->flags, bf->name, 0, bf->description);
  }
}


/*****************************************************/
static void cmd_register(char **groups, int num_groups)
/*****************************************************/
{
  regmap_register_t *reg;
  regmap_group_t *grp;
  u_long value, amin, amax;
  int loop, index, groupIndex;

  printf("You have selected %d group(s)\n", num_groups);

  print_header();

  for(grp = regmap_groups; grp->name != NULL; grp++)
  {
    for (loop = 0; loop < num_groups; loop++)
    {
      if(strcasecmp(grp->name, groups[loop]) == 0)
      {
	printf("\nListing registers for group %s (%s)\n", grp->name, grp->description);

        for (index = 0; grp->index[index] != -1; index++)
	{
         groupIndex = grp->index[index];
	 if(regmap_registers[groupIndex].flags & REGMAP_REG_READ)
	 {
	   flxCard.cfg_get_reg(regmap_registers[groupIndex].name, &value);
           print_register(regmap_registers[groupIndex].address, regmap_registers[groupIndex].flags, regmap_registers[groupIndex].name, value, regmap_registers[groupIndex].description);
	 }
	 else
           print_register(regmap_registers[groupIndex].address, regmap_registers[groupIndex].flags, regmap_registers[groupIndex].name, 0, regmap_registers[groupIndex].description);
	}
      }
    }
  }

  if (num_groups == 0)
  {
    for(grp = regmap_groups; grp->name != NULL; grp++)
    {
      printf("\nListing registers for group %s (%s)\n", grp->name, grp->description);

      for (index = 0; grp->index[index] != -1; index++)
      {
       groupIndex = grp->index[index];
       if(regmap_registers[groupIndex].flags & REGMAP_REG_READ)
       {
	 flxCard.cfg_get_reg(regmap_registers[groupIndex].name, &value);
	 print_register(regmap_registers[groupIndex].address, regmap_registers[groupIndex].flags, regmap_registers[groupIndex].name, value, regmap_registers[groupIndex].description);
       }
       else
	 print_register(regmap_registers[groupIndex].address, regmap_registers[groupIndex].flags, regmap_registers[groupIndex].name, 0, regmap_registers[groupIndex].description);
      }
    }
  }
}


/***************************************/
static void cmd_set(char **config, int n)
/***************************************/
{
  int i;

  for(i = 0; i < n; i++)
  {
    u_long value = split_key_value(config[i]);
    flxCard.cfg_set_option(config[i], value);
  }
}


/**********************************/
static void cmd_load(char *filename)
/**********************************/
{
  FILE *fp;
  char *line = NULL;
  size_t len = 0;

  fp = fopen(filename, "r");
  if (fp == NULL)
  {
    fprintf(stderr, "Could not open '%s'\n", filename);
    exit(-1);
  }

  while (getline(&line, &len, fp) != -1)
  {
    u_long value = split_key_value(line);
    flxCard.cfg_set_option(line, value);
  }

  if (line)
    free(line);
  fclose(fp);
}


/***********************************/
static void cmd_store(char *filename)
/***********************************/
{
  FILE *fp;
  regmap_bitfield_t *bf;
  u_long value;

  fp = fopen(filename, "w");
  if (fp == NULL)
  {
    fprintf(stderr, "Could not open '%s'\n", filename);
    exit(-1);
  }

  for(bf = regmap_bitfields; bf->name != NULL; bf++)
  {
    if(bf->flags & REGMAP_REG_WRITE && bf->flags & REGMAP_REG_READ)
    {
      flxCard.cfg_get_option(bf->name, &value);
      fprintf(fp, "%s=0x%llx\n", bf->name, value);
    }
  }

  fclose(fp);
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int opt, debuglevel, device_number = 0, mode = CMD_UNKNOWN;

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

  if(strcasecmp(argv[optind], "registers") == 0) mode = CMD_REGISTERS;
  if(strcasecmp(argv[optind], "list") == 0)      mode = CMD_LIST;
  if(strcasecmp(argv[optind], "set") == 0)       mode = CMD_SET;
  if(strcasecmp(argv[optind], "load") == 0)      mode = CMD_LOAD;
  if(strcasecmp(argv[optind], "store") == 0)     mode = CMD_STORE;

  if(mode == CMD_UNKNOWN)
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

  switch(mode)
  {
    case CMD_LIST:
      cmd_list();
      break;

    case CMD_REGISTERS:
      cmd_register(argv + optind + 1, argc - 1 - optind);
      break;

    case CMD_SET:
      cmd_set(argv + optind + 1, argc - 1 - optind);
      break;

    case CMD_LOAD:
      if(argc < 3 || argv[2][0] == '-')
      {
	fprintf(stderr, "No filename given\n");
	exit(-1);
      }
      cmd_load(*(argv + optind + 1));
      break;

    case CMD_STORE:
      if(argc < 3 || argv[2][0] == '-')
      {
	fprintf(stderr, "No filename given\n");
	exit(-1);
      }
      cmd_store(*(argv + optind + 1));
      break;
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
