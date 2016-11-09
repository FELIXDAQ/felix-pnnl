/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-info application         */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/* (based on code from a.rodriguez@cern.ch                         */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

#define APPLICATION_NAME    "flx-info"
#define APPLICATION_VERSION "1.0.0"


enum info_mode 
{
  INFO_UNKNOWN,
  INFO_GBT,
  INFO_FMC_TEMP_SENSOR,
  INFO_FMC_ADN,
  INFO_CXP,
  INFO_CDCE,
  INFO_DDR3,
  INFO_EGROUP,
  INFO_SFP,
  INFO_IDEEPROM,
  INFO_ALL
};

//Globals
FlxCard flxCard;
u_long baraddr2;


/************************/
static void display_help()
/************************/
{
  printf("Usage: %s [OPTIONS] [COMMAND] [CMD ARGUMENTS]\n", APPLICATION_NAME);
  printf("Displays information about a FLX device.\n\n");
  printf("Options:\n");
  printf("  -d NUMBER                       Use card indicated by NUMBER. Default: 0.\n");
  printf("  -v                              Verbose mode.\n");
  printf("  -D level                        Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h                              Display help.\n");
  printf("  -V                              Display the version number\n");
  printf("Commands:\n");
  printf("  GBT                             Shows GBT channel alignment status.\n");
  printf("  FMC_TEMP_SENSOR                 Display FMC temperature from TC74 sensor.\n");
  printf("  FMC_ADN                         Display FMC ADN register 0x4.\n");
  printf("  CXP                             Display temperature and voltage from CXP1 and CXP2\n");
  printf("  SFP                             Display information from Small Form Factor Pluggable transceivers\n");
  printf("  CDCE                            Display some values from CDCE clock\n");
  printf("  DDR3                            Display values from DDR3 RAM memory\n");
  printf("  ID_EEPROM                       Display the first 32 bytes of the eeprom memory\n");
  printf("  EGROUP       [channel]  [RAW]   Display values from EGROUP registers:  \n");
  printf("                                      If no channel is specified, display all available.\n");
  printf("                                      Using Hexadecimal notation if RAW is specified.\n");
  printf("  ALL                             Display ALL information.\n");
}


/***************************/
static void display_idt(void)
/***************************/
{
  printf("  Address | 0x00 0x04 0x08 0x0C 0x14\n");
  printf("  Value   | 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
    flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x00),
    flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x04),
    flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x08),
    flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x0C),
    flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x14));
}


/********************************/
static void display_card_id(void)
/********************************/
{
  u_long card_id = 0, reg_map_version = 0, major, minor;
  int card_control = 0;
  
  // Register map version stored as hex: 0x0300 -> 3.0
  flxCard.cfg_get_option(BF_REG_MAP_VERSION, &reg_map_version);
  major = (reg_map_version & 0xFF00) >> 8;
  minor = (reg_map_version & 0x00FF) >> 0;
  printf("Reg Map Version  %lx.%lx\n", major, minor);

  flxCard.cfg_get_option(BF_CARD_TYPE, &card_id);
  if(card_id == 0x2C7)
  {
    printf("Card ID:         FLX-711\n");
    card_control = 1;
  }
  if(card_id == 0x2C6)
  {
    printf("Card ID:         FLX-710\n");
    card_control = 1;
  }
  if(card_id==0x2C5)
  {
    printf("Card ID:         FLX-709\n");
    card_control = 1;
  }
  if(card_control==0)
    printf("Card ID:         UNKNOWN\n");
}



/*******************************/
static void display_FW_date(void)
/*******************************/
{
  u_long date = 0;
  u_int version_day = 0, version_month = 0, version_year = 0, version_hour = 0, version_minute = 0;

  flxCard.cfg_get_option(BF_BOARD_ID_TIMESTAMP, &date);
  
  //Not very elegant
  version_year   = (date >> 32);
  version_month  = (0x00FF) & (date >> 24);
  version_day    = (0x0000FF) & (date >> 16);
  version_hour   = (0x000000FF) & (date >> 8);
  version_minute = (0x00000000FF) & date;

  printf("FW version date: %02x/%02x/%02x %02x:%02x\n", version_day, version_month, version_year, version_hour, version_minute);
}


/***********************************/
static void display_SVN_version(void)
/***********************************/
{
  u_long value = 0;
  
  flxCard.cfg_get_option(BF_BOARD_ID_SVN, &value);
  printf("SVN version:     %lu\n", value);
}



/**********************************************/
static void display_interrupts_descriptors(void)
/**********************************************/
{
  u_long descriptors = 0, interrupts = 0;
  
  flxCard.cfg_get_option(BF_GENERIC_CONSTANTS_INTERRUPTS, &interrupts);
  flxCard.cfg_get_option(BF_GENERIC_CONSTANTS_DESCRIPTORS, &descriptors);
  printf("Number of interrupts : %ld\n", interrupts);
  printf("Number of descriptors: %ld\n", descriptors);
}


/********************************/
static void display_channels(void)
/********************************/
{
  u_long channels = 0;
  
  flxCard.cfg_get_option(BF_NUM_OF_CHANNELS, &channels);
  printf("Number of channels   : %lu\n", channels);
}


/*************************************************/
static void display_channels_alignment(int verbose)
/*************************************************/
{
  u_long cont, channels = 0, result = 0, mask = 1, number_channels = 0;

  flxCard.cfg_get_reg(REG_GBT_ALIGNMENT_DONE, &channels); /* CHECK REGISTER */
  flxCard.cfg_get_option(BF_NUM_OF_CHANNELS, &number_channels);

  if(verbose) 
  {
    printf("Number of channels: 0x%lx\n", number_channels);
    printf("Alignment done value: 0x%lx\n", channels);
  }
  
  printf("\n\n  GBT CHANNEL ALIGNMENT STATUS\n\n");
  printf("         ");
  
  for (cont = 0; cont < number_channels; cont++)
    printf("   %lu  ",cont);

  printf("\n        ");
  
  for (cont = 0; cont < number_channels; cont++)
    printf("------");

  printf("\nAligned |");
  for(cont = 0; cont < number_channels; cont++)
  {
    mask = 1;
    mask = (1ul << cont);
    result = channels&mask;
    if(result >= 1)
      printf("  Yes ");
    else 
      printf("  No  ");
  }
  printf("\n\n");
}


/********************************/
static void display_ideeprom(void)
/********************************/
{
  int cont = 0;
  char eeprom_device[15] = "\0";
  u_char character = 0;
  
  try
  {
    printf("First 32 bytes of the eeprom:\n");
    for(cont = 0; cont < 32; cont++)
    {
      flxCard.i2c_devices_read(eeprom_device, cont, &character);
      printf("%c", character);
    }
    printf("\n\n");
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  }  
}


/****************************/
static void display_tc74(void)
/****************************/
{
  i2c_device_t *devices;
  char device[] = "FMC_TEMP_SENSOR";
  u_char value = 0;
  u_long card_model = 0;

  card_model = flxCard.card_model();
  if(card_model == FLX_711)
  {
    devices = i2c_devices_FLX_711;
    std::cout << "ERROR. FLX-711 is not supported" << std::endl; 
    exit(-1);
  }
  if(card_model == FLX_710)
    devices = i2c_devices_FLX_710;
  if(card_model == FLX_709)
    devices = i2c_devices_FLX_709;

  try
  {
    flxCard.i2c_devices_read(device, 0, &value);
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  }  

  printf("\nFMC TEMPERATURE = %d C\n", value);
}


/**********************************/
static void display_TTC_status(void)
/**********************************/
{
  u_char misc_value = 0;
  u_long card_model = 0;
  i2c_device_t *devices;
  char device[] = "FMC_ADN";

  card_model = flxCard.card_model();
  if(card_model == FLX_711)
  {
    devices = i2c_devices_FLX_711;
    std::cout << "ERROR. FLX-711 is not supported" << std::endl; 
    exit(-1);
  }
  if(card_model == FLX_710)
    devices = i2c_devices_FLX_710;
  if(card_model == FLX_709)
    devices = i2c_devices_FLX_709;

  try
  {
    flxCard.i2c_devices_read(device, 4, &misc_value);
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  }  

  printf("\nFMC ADN TTC Status: ");
  if(((misc_value & 0x08) >> 3))
    printf("OFF\n");
  else 
    printf("ON\n");
}


/**************************************/
static void display_adn2814(int verbose)
/**************************************/
{
  i2c_device_t *devices;
  u_char misc_value = 0, ctrla_value = 0, ctrlb_value = 0;
  char device[] = "FMC_ADN";
  u_long card_model = 0;

  //Search using name.

  card_model = flxCard.card_model();
  if(card_model == FLX_711)
  {
    devices = i2c_devices_FLX_711;
    std::cout << "ERROR. FLX-711 is not supported" << std::endl; 
    exit(-1);
  }
  if(card_model == FLX_710)
    devices = i2c_devices_FLX_710;
  if(card_model == FLX_709)
    devices = i2c_devices_FLX_709;
  try
  {
    flxCard.i2c_devices_read(device, 4, &misc_value);
    flxCard.i2c_devices_read(device, 8, &ctrla_value);
    flxCard.i2c_devices_read(device, 9, &ctrlb_value);
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  }  

  if(verbose) 
  {
    printf("Misc value 0x%x\n", misc_value);
    printf("CTRLA value 0x%x\n", ctrla_value);
    printf("CTRLB value 0x%x\n", ctrlb_value);
  }

  printf("TTC Status: ");
  if(((misc_value & 0x08) >> 3))
    printf("OFF\n");
  else
    printf("ON\n");

  printf("Loss of Signal Status: %d \n", ((misc_value & 0x20) >> 5));
  printf("Static Loss of Lock:   %d \n", ((misc_value & 0x10) >> 4));
  printf("Loss of Lock Status:   %d \n", ((misc_value & 0x08) >> 3));
}


/***************************/
static void display_cxp(void)
/***************************/
{
  i2c_device_t *devices;
  int cont = 0, cont1 = 0;
  char valueMSB = 0, *device[4];
  float fractional = (float)1 / (float)256, voltage = 0, temperature = 0;
  u_char aux_value = 0, valueLSB = 0, voltageMSB = 0, voltageLSB = 0, LOS_status = 0;
  u_int um_voltage = 0;
  u_long card_model = 0;

  device[0] = "CXP1_TX";
  device[1] = "CXP1_RX";
  device[2] = "CXP2_TX";
  device[3] = "CXP2_RX";

  card_model = flxCard.card_model();
  if(card_model == FLX_711)
  {
    devices = i2c_devices_FLX_711;
    std::cout << "ERROR. FLX-711 is not supported" << std::endl; 
    exit(-1);
  }
  if(card_model == FLX_710)
    devices = i2c_devices_FLX_710;
  if(card_model == FLX_709)
    devices = i2c_devices_FLX_709;

  for(cont = 0; cont < 4; cont++)
  {
    try
    {
      flxCard.i2c_devices_read(device[cont], 22, &aux_value);

      valueMSB = (char) aux_value;
      flxCard.i2c_devices_read(device[cont], 23, &valueLSB);
      temperature = valueLSB * fractional;
      temperature = temperature + valueMSB;
      printf("\n%s 1st temperature monitor: %.2f C\n", device[cont], temperature);

      flxCard.i2c_devices_read(device[cont], 26, &voltageMSB);
      flxCard.i2c_devices_read(device[cont], 27, &voltageLSB);

      um_voltage = (voltageMSB << 8) + voltageLSB;
      voltage = (float)um_voltage / 10000;
      printf("%s voltage monitor 3.3 V: %.3f V\n", device[cont], voltage);

      printf("%s LOS channels status:\n", device[cont]);
      flxCard.i2c_devices_read(device[cont], 0x08, &LOS_status);
    
      printf("                C0  C1  C2  C3  C4  C5  C6  C7  C8  C9  C10 C11 \n");
      printf("              --------------------------------------------------\n");
      printf("Signal Status |");

      for(cont1 = 0; cont1 < 8; cont1++)
      {
	if(((LOS_status & (1 << cont1)) >> cont1))
          printf(" -- ");
	else 
          printf(" OK ");
      }

      flxCard.i2c_devices_read(device[cont], 0x07, &LOS_status);
      for(cont1 = 0; cont1 < 4; cont1++)
      {
	if(((LOS_status & (1 << cont1)) >> cont1))
          printf(" -- ");
	else 
          printf(" OK ");
      }
    }
    catch(FlxException ex)
    {
      std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
      exit(-1);
    }  
    printf("|\n");
    printf("              --------------------------------------------------\n");
  }
}


/****************************/
static void display_cdce(void)
/****************************/
{
  spi_device_t *devices;
  int found = 0;
  u_int value = 0;
  char device[] = "CLOCK_CDCE";
  u_char version = 0, dll_status = 0;

  try
  {
    printf("\nCDCE Status\n");
    printf("----------\n");
    //Search using name.
    for(devices = spi_devices; devices->name != NULL && found == 0; devices++)
    {
      if(strcasecmp(device, devices->name) == 0)
      {
	//Reading value.
	value = flxCard.spi_read(0x8);
	found = 1;
	value = flxCard.spi_read(0x8);
	version = ((value & 0x1C00) >> 10);
	dll_status = ((value & 0x40) >> 6);
	printf("Version: %u\n", version);
	printf("PLL: %u\n", dll_status);
	printf("\n");
      }
    }
    if(!found)
      fprintf(stderr, APPLICATION_NAME": warning: CDCE not found\n");
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  }    
}


/****************************/
static void display_ddr3(void)
/****************************/
{
  char *device[2];
  int cont = 0, sel_device = 0;
  float timebase = 0;
  u_int sd_cap = 0, total_cap = 0;
  u_char part_number[40] = "\0", spd_revision = 0, dram_type = 0, module_type = 0, density = 0, ba_bits = 0;
  u_char mod_org = 0, n_ranks = 0, dev_width = 0, mod_width = 0, pb_width = 0, mtb_dividend = 0, mtb_divisor = 0;
  u_char tckmin = 0, taamin = 0, twrmin = 0, trcdmin = 0, trrdmin = 0, trpmin = 0, trasrcupper = 0, trasmin_aux = 0, trcmin_aux = 0;
  u_char trfcmin_aux1 = 0, trfcmin_aux2 = 0, twtrmin = 0, trtpmin = 0, tfawmin_aux1 = 0, tfawmin_aux2 = 0, tropts = 0;
  u_char cassupport_aux1 = 0, cassupport_aux2 = 0;
  u_short trasmin = 0, trcmin = 0, trfcmin = 0, tfawmin = 0, cassupport = 0;
  u_long card_model = 0;

  try
  {
    card_model = flxCard.card_model();

    device[0] = "DDR3-1";
    device[1] = "DDR3-2";

    for(sel_device = 0; sel_device < 2; sel_device++)
    {
      for(cont = 128; cont < 145; cont++)
	flxCard.i2c_devices_read(device[sel_device], cont, &part_number[cont - 128]);

      printf("Device %s\n", device[sel_device]);
      printf("----------------------------------------------\n");
      printf("Part number      : %s\n", part_number);

      flxCard.i2c_devices_read(device[sel_device], 1, &spd_revision);
      printf("SPD Revision     : %d.%d\n", (spd_revision >> 4), (spd_revision & 0x0f));

      flxCard.i2c_devices_read(device[sel_device], 2, &dram_type);
      printf("DRAM Device Type : 0x%x\n", dram_type);

      flxCard.i2c_devices_read(device[sel_device], 3, &module_type);
      printf("Module Type      : 0x%x\n", module_type);

      flxCard.i2c_devices_read(device[sel_device], 4, &density);
      ba_bits = (density & 0x70);
      ba_bits = (ba_bits >> 4);
      ba_bits = ba_bits + 3;
      sd_cap  = 256 * (1ul << (density & 0xf));
      
      printf("Bank Address     : %d bit\n", (int)ba_bits);
      printf("SDRAM Capacity   : %d Mbit\n", (int)sd_cap);

      flxCard.i2c_devices_read(device[sel_device], 7, &mod_org);
      n_ranks = ((mod_org >> 3) & 0x7) + 1;
      dev_width = 4 * (1 << (mod_org & 0x07));
      printf("Number of Ranks  : %d \n", (int)n_ranks);
      printf("Device Width     : %d bit\n", (int)dev_width);

      flxCard.i2c_devices_read(device[sel_device], 8, &mod_width);
      pb_width = (mod_width & 0x7);
      pb_width = (1 << pb_width);
      pb_width = pb_width * 8;
      printf("Bus Width        : %d bit\n", (int)pb_width);

      total_cap = sd_cap / 8 * pb_width / dev_width * n_ranks;
      printf("Total Capacity   : %d MB\n", total_cap);

      flxCard.i2c_devices_read(device[sel_device], 10, &mtb_dividend);
      flxCard.i2c_devices_read(device[sel_device], 11, &mtb_divisor);
      timebase = (float)mtb_dividend / (float)mtb_divisor;
      printf("Medium Timebase  : %.2f ns\n", timebase);

      flxCard.i2c_devices_read(device[sel_device], 12, &tckmin);
      printf("tCKmin           : %.2f ns\n", tckmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 16, &taamin);
      printf("tAAmin           : %.2f ns\n", taamin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 17, &twrmin);
      printf("tWRmin           : %.2f ns\n", twrmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 18, &trcdmin);
      printf("tRCDmin          : %.2f ns\n", trcdmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 19, &trrdmin);
      printf("tRRDmin          : %.2f ns\n", trrdmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 20, &trpmin);
      printf("tRPmin           : %.2f ns\n", trpmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 21, &trasrcupper);
      flxCard.i2c_devices_read(device[sel_device], 22, &trasmin_aux);
      trasmin = ((trasrcupper & 0x0f) << 8) |trasmin_aux;
      printf("tRASmin          : %.2f ns\n", trasmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 23, &trcmin_aux);
      trcmin = ((trasrcupper & 0xf0) << 4) | trcmin_aux;
      printf("tRCmin           : %.2f ns\n", trcmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 25, &trfcmin_aux1);
      flxCard.i2c_devices_read(device[sel_device], 24, &trfcmin_aux2);
      trfcmin = (trfcmin_aux1 << 8) | trfcmin_aux2;
      printf("tRFCmin          : %.2f ns\n", trfcmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 26, &twtrmin);
      printf("tWTRmin          : %.2f ns\n", twtrmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 27, &trtpmin);
      printf("tRTPmin          : %.2f ns\n", trtpmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 28, &tfawmin_aux1);
      flxCard.i2c_devices_read(device[sel_device], 29, &tfawmin_aux2);
      tfawmin = ((tfawmin_aux1 << 8) & 0x0f) | tfawmin_aux2;
      printf("tFAWmin          : %.2f ns\n", tfawmin * timebase);

      flxCard.i2c_devices_read(device[sel_device], 32, &tropts);
      printf("Thermal Sensor   : %d\n", (tropts >> 7) & 1);

      flxCard.i2c_devices_read(device[sel_device], 15, &cassupport_aux1);
      flxCard.i2c_devices_read(device[sel_device], 14, &cassupport_aux2);
      cassupport = (cassupport_aux1 << 8) | cassupport_aux2;
      printf("CAS Latencies    : ");
      for(cont = 0; cont < 14; cont++)
      {
        if((cassupport >> cont) & 1)
          printf("CL %d  ", (cont + 4));
      }
      printf("\n");
    }
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  } 
}


/***************************/
static void display_sfp(void)
/***************************/
{
  i2c_device_t *devices;
  int cont = 0, cont1 = 0, cont2 = 0;
  u_char valueMSB = 0, aux_value = 0, valueLSB = 0, voltageMSB = 0, voltageLSB = 0;
  u_char part_number[40] = "\0", tx_fault = 0, rx_lost = 0, loss_os[4], data_ready = 0;
  u_short u_temperature = 0;
  float fractional = (float)1 / (float)256, voltage = 0, temperature = 0;
  u_int um_voltage = 0;
  u_long card_model = 0;
  char *device[8];
  
  device[0] = "SFP1-1";
  device[1] = "SFP1-2";
  device[2] = "SFP2-1";
  device[3] = "SFP2-2";
  device[4] = "SFP3-1";
  device[5] = "SFP3-2";
  device[6] = "SFP4-1";
  device[7] = "SFP4-2";
  loss_os[0] = 0;
  loss_os[1] = 0;
  loss_os[2] = 0;
  loss_os[3] = 0;

  try
  {
    card_model = flxCard.card_model();
    if(card_model == FLX_711)
    {
      devices = i2c_devices_FLX_711;
      std::cout << "ERROR. FLX-711 is not supported" << std::endl; 
      exit(-1);
    }
    if(card_model == FLX_710)
      devices = i2c_devices_FLX_710;
    if(card_model == FLX_709)
      devices = i2c_devices_FLX_709;

    for(cont = 0; cont < 8; cont++)
    {
      flxCard.i2c_devices_read(device[cont], 96, &valueMSB);
      for(cont1 = 20; cont1 < 26; cont1++)
        flxCard.i2c_devices_read(device[cont], cont1, &part_number[cont1 - 20]);

      for(cont1 = 40; cont1 < 51; cont1++)
        flxCard.i2c_devices_read(device[cont], cont1, &part_number[cont1 - 40 + 6]);

      printf("Device %s\n", device[cont]);
      printf("----------------------------------------------\n");
      printf("Part number      : %s\n", part_number);

      cont++; //Switch to enhanced memory.

      flxCard.i2c_devices_read(device[cont], 96, &valueMSB);
      flxCard.i2c_devices_read(device[cont], 97, &valueLSB);

      u_temperature = valueMSB;
      u_temperature = (u_temperature << 8);
      u_temperature = u_temperature + valueLSB;
      temperature = ((short)u_temperature * fractional);
      printf("\n%s temperature monitor: %.2f C\n", device[cont], temperature);

      flxCard.i2c_devices_read(device[cont], 98, &voltageMSB);
      flxCard.i2c_devices_read(device[cont], 99, &voltageLSB);

      um_voltage = voltageMSB;
      um_voltage = (um_voltage << 8) + voltageLSB;
      voltage = (float)um_voltage / 10000;
      printf("%s voltage monitor: %.3f V\n", device[cont], voltage);

      flxCard.i2c_devices_read(device[cont], 110, &aux_value);

      tx_fault   = ((aux_value&(1 << 2)) >> 2);
      rx_lost    = ((aux_value&(1 << 1)) >> 1);
      data_ready = (aux_value & 1);

      printf("Transmission fault estate: %u", tx_fault);
      
      if(tx_fault == 1)
        printf("    Fault on data transmission\n");
      else 
        printf("    Transmitting data\n");

      loss_os[cont2] = rx_lost;
      cont2++;
      printf("Loss of Signal estate: %u", rx_lost);
      
      if(rx_lost == 1)
        printf("        Signal not received or under threshold\n");
      else 
        printf("        Receiving signal\n");
	
      printf("Data ready: %u", tx_fault);
      
      if(tx_fault == 1)
        printf("                   Transceiver not ready\n");
      else 
        printf("                   Transceiver ready\n");
      printf("\n");
    }
    
    printf("               1    2    3    4\n");
    printf("---------------------------------\n");
    printf("Link Status  |");
    for(cont=0;cont<4;cont++)
    {
      if(loss_os[cont]==1)
        printf(" --  ");
      else 
        printf(" Ok  ");
    }
    printf("\n\n");
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  } 
}


/*************************************/
static void display_egroup(int channel)
/*************************************/
{
  u_long offset = 0x1100, step = 0x0010, address = 0, value = 0;
  int cont1 = 0, cont2 = 0;
  u_int chunk_lenth = 0, chunk_lenth_aux = 0, encoding = 0;
  u_char c_encoding = 0, eproc_pos = 0, eproc_val = 0;

  address = offset + (step * 12 * channel);
  printf("\n\n------------------------------------------DISPLAYING CHANNEL %d-------------------------------------\n\n", channel);
  printf("               MAX CHUNK                                 PATH                                  \n");
  printf("          IN2  IN4  IN8  IN16      0       1       2       3       4       5       6       7   \n");
  printf("-----------------------------------------------------------------------------------------------\n");

  for(cont1 = 0; cont1 < 7; cont1++)
  {
    u_long *p_value = (u_long*)(baraddr2 + address);  
    value = *p_value;
    address = address + step;

    chunk_lenth = ((value & 0x7FF80000000) >> 31);
    encoding = ((value & 0x7FFF8000) >> 15);

    printf("TH_E_%d | ", cont1);

    for(cont2 = 0;cont2 < (4 * 3); cont2 = cont2 + 3)
    {
      chunk_lenth_aux = ((chunk_lenth & (7ul << cont2)) >> cont2);
      printf("%4d ", chunk_lenth_aux * 512);
    }
    printf(" |");
    
    for(cont2 = 0;cont2 < 16;cont2 = cont2 + 2)
    {
      eproc_pos = cont2 / 2;

      if(eproc_pos == 0)
        eproc_val = (16 * ((value & (1ul << eproc_pos)) >> eproc_pos));
      if(eproc_pos == 1 || eproc_pos == 2)
        eproc_val = (8 * ((value & (1ul << eproc_pos)) >> eproc_pos));
      if(eproc_pos >= 3 && eproc_pos <= 6)
        eproc_val = (4 * ((value & (1ul << eproc_pos)) >> eproc_pos));
      if(eproc_pos >= 7 && eproc_pos <= 14)
        eproc_val = ( 2 *((value & (1ul << eproc_pos)) >> eproc_pos));

      c_encoding = ((encoding & (3ul << cont2)) >> cont2);
      switch(c_encoding)
      {
        case 0:
          printf("   ---  ");
          break;
        case 1:
          printf("  %2d(8) ", eproc_val);
          break;
        case 2:
          printf("  %2d(H) ", eproc_val);
          break;
        case 3:
          printf("  %2d(R) ", eproc_val);
          break;
        default:
          printf(" UNKNOWN");
      }
    }
    printf("\n");
  }
  printf("-----------------------------------------------------------------------------------------------\n");

  for(cont1 = 0; cont1 < 5; cont1++)
  {
    u_long *p_value= (u_long *)(baraddr2 + address);  
    value = *p_value;
    address = address + step;

    encoding = ((value & 0x7FFFFFFF8000) >> 15);
    printf("FH_E_%d                        |", cont1);
    
    for(cont2 = 0; cont2 < (8 * 4); cont2 = cont2 + 4)
    {
      eproc_pos = cont2 / 4;

      if(eproc_pos == 0)
        eproc_val = (16 * ((value & (1ul << eproc_pos)) >> eproc_pos));
      if(eproc_pos == 1 || eproc_pos == 2)
        eproc_val = (8 * ((value & (1ul << eproc_pos)) >> eproc_pos));
      if(eproc_pos >= 3 && eproc_pos <= 6)
        eproc_val = (4 * ((value & (1ul << eproc_pos)) >> eproc_pos));
      if(eproc_pos >= 7 && eproc_pos <= 14)
        eproc_val = (2 * ((value & (1ul << eproc_pos)) >> eproc_pos));

      c_encoding = ((encoding & (0xFul << cont2)) >> cont2);

      switch(encoding)
      {
        case 0:
          printf("   ---  ");
          break;
        case 1:
          printf("  %2d(8) ", eproc_val);
          break;
        case 2:
          printf("  %2d(H) ", eproc_val);
          break;
        case 3:
          printf("  %2d(T0)", eproc_val);
          break;
        case 4:
          printf("  %2d(T2)", eproc_val);
          break;
        default:
          printf("  %2d(E) ", eproc_val);
      }
    }
    printf("\n");
  }
}


/*****************************************/
static void display_egroup_raw(int channel)
/*****************************************/
{
  u_long offset = 0x1100, step = 0x0010, address = 0, value = 0;
  int cont1 = 0, cont2 = 0;
  u_int chunk_lenth = 0, encoding = 0;
  u_char c_encoding = 0;
  u_short eprocs = 0;

  address = offset + (step * 12 * channel);
  printf("\n\n---------------------------------------DISPLAYING CHANNEL %d--------------------------------\n\n", channel);
  printf("      MAX CHUNK                               PATH                             \n");
  printf("     LENGTH ARRAY   0       1       2       3       4       5       6       7      EPROC\n");
  printf("------------------------------------------------------------------------------------------\n");

  for(cont1 = 0; cont1 < 7; cont1++)
  {
    u_long *p_value = (u_long*)(baraddr2 + address);  
    value = *p_value;
    address = address + step;

    chunk_lenth = ((value & 0x7FF80000000) >> 31);
    encoding = ((value & 0x7FFF8000) >> 15);
    eprocs = (value & 0x00007FFF);

    printf("TH_E_%d | 0x%03x | ", cont1, chunk_lenth);
    for(cont2 = 0; cont2 < 16; cont2 = cont2 + 2)
    {
      c_encoding = ((encoding & (3ul << cont2)) >> cont2);
      printf("  0x%02x  ", c_encoding);
    }
    printf("| 0x%04x\n", eprocs);
  }

  printf("------------------------------------------------------------------------------------------\n");
  for(cont1 = 0; cont1 < 5; cont1++)
  {
    u_long *p_value= (u_long*)(baraddr2 + address); 
    value = *p_value;
    address = address + step;

    encoding = ((value & 0x7FFFFFFF8000) >> 15);
    eprocs = (value & 0x00007FFF);
    printf("FH_E_%d         | ", cont1);
    for(cont2 = 0; cont2 < (8 * 4) ; cont2 = cont2 + 4)
    {
      c_encoding = ((encoding&(0xFul << cont2)) >> cont2);
      printf("  0x%02x  ", c_encoding);
    }
    printf("| 0x%04x\n", eprocs);
  }
}


/********************************************************************************/
static void display_egroups(int argc, char **argv, int common_info, int arguments)
/********************************************************************************/
{
  int n_channel = 0;
  u_long value = 0, cont = 0;

  try
  {
    if(argc == (4 + arguments))
    {
      if(strcasecmp(argv[3 + arguments], "RAW") == 0)
      {
	n_channel = atoi(argv[2 + arguments]);
	if((n_channel < 24) && (n_channel >= 0))
          display_egroup_raw(n_channel);
	else 
          fprintf(stderr, APPLICATION_NAME": error: Invalid number of channel\n");
      } 
      else
	fprintf(stderr, APPLICATION_NAME": error: Invalid argument\n");
    } 
    else
    {
      if(argc == (3 + arguments))
      {
	if(strcasecmp(argv[2 + arguments], "RAW") == 0)
	{
          flxCard.cfg_get_option(BF_NUM_OF_CHANNELS, &value);
          printf("Displaying %lu channels\n", value);

          for(cont = 0; cont < value; cont++)
            display_egroup_raw(cont);
	}
	else 
	{
          n_channel = atoi(argv[2 + arguments]);
          if((n_channel < 24) && (n_channel >= 0))
            display_egroup(n_channel);
	  else 
            fprintf(stderr, APPLICATION_NAME": error: Invalid number of channel\n");
	}
      }
      else 
      {
	if(argc == (2 + arguments))
	{
          flxCard.cfg_get_option(BF_NUM_OF_CHANNELS, &value);
          for(cont = 0; cont < value; cont++)
            display_egroup(cont);
	}
	else
          fprintf(stderr, APPLICATION_NAME": error: Invalid number of arguments\n");
      }
    }
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  } 
}


/********************/
static int check_idt()
/********************/
{
  try
  {
    if(flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x00) != 0x2a) return 1;
    if(flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x04) != 0x00) return 1;
    if(flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x08) != 0x11) return 1;
    if(flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x0C) != 0x0a) return 1;
    if(flxCard.i2c_read_byte(I2C_ADDR_CXP, 0x14) != 0x1f) return 1;
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  } 
  
  return 0;
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int device_number = 0, opt, verbose = 0, gbt_info = 0, tem_sensor_info = 0, common_info = 0, adn_info = 0, mode = INFO_UNKNOWN;
  int debuglevel, cxp_info = 0, cdce_info = 0, ddr3_info = 0, egroup_info = 0, sfp_info = 0, id_eeprom_info = 0, arguments = 0;
  u_long card_model = 0;

  while((opt = getopt(argc, argv, "hvd:D:V")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
	device_number = atoi(optarg);
	arguments = arguments + 2;
	break;
	
      case 'v':
	verbose++;
	arguments++;
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

  if(optind != argc)
  {
    if(strcasecmp(argv[optind], "GBT") == 0)              mode = INFO_GBT;
    if(strcasecmp(argv[optind], "FMC_TEMP_SENSOR") == 0)  mode = INFO_FMC_TEMP_SENSOR;
    if(strcasecmp(argv[optind], "FMC_ADN") == 0)          mode = INFO_FMC_ADN;
    if(strcasecmp(argv[optind], "CXP") == 0)              mode = INFO_CXP;
    if(strcasecmp(argv[optind], "CDCE") == 0)             mode = INFO_CDCE;
    if(strcasecmp(argv[optind], "DDR3") == 0)             mode = INFO_DDR3;
    if(strcasecmp(argv[optind], "EGROUP") == 0)           mode = INFO_EGROUP;
    if(strcasecmp(argv[optind], "SFP") == 0)              mode = INFO_SFP;
    if(strcasecmp(argv[optind], "ID_EEPROM") == 0)        mode = INFO_IDEEPROM;
    if(strcasecmp(argv[optind], "ALL") == 0)              mode = INFO_ALL;

    if(mode == INFO_UNKNOWN)
    {
      fprintf(stderr, "Unrecognized command '%s'\n", argv[1]);
      fprintf(stderr, "Usage: "APPLICATION_NAME" COMMAND [OPTIONS]\nTry "APPLICATION_NAME" -h for more information.\n");
      exit(-1);
    }
  }

  switch(mode)
  {
    case INFO_GBT:
      gbt_info = 1;
      break;
      
    case INFO_FMC_TEMP_SENSOR:
      tem_sensor_info = 1;
      break;
      
    case INFO_FMC_ADN:
      adn_info = 1;
      break;
      
    case INFO_CXP:
      cxp_info = 1;
      break;
      
    case INFO_CDCE:
      cdce_info = 1;
      break;
      
    case INFO_DDR3:
      ddr3_info = 1;
      break;
      
    case INFO_EGROUP:
      egroup_info = 1;
      break;
      
    case INFO_SFP:
      sfp_info = 1;
      break;
      
    case INFO_IDEEPROM:
      id_eeprom_info = 1;
      break;
      
    case INFO_ALL:
      id_eeprom_info = 1;
      tem_sensor_info = 1;
      gbt_info = 1;
      common_info = 1;
      adn_info = 1;
      cxp_info = 1;
      ddr3_info = 1;
      cdce_info = 1;
      sfp_info = 1;
      egroup_info = 1;
      break;
      
    default: 
      common_info = 1;
  }

  try
  {
    flxCard.card_open(device_number);
    baraddr2 = flxCard.openBackDoor(2);
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  }    

  card_model = flxCard.card_model();
  if(card_model != FLX_711 && card_model != FLX_710 && card_model != FLX_709)
  {
    fprintf(stderr, APPLICATION_NAME" error: Card model not recognized\n");
    exit(-1);
  }

  if(common_info)
  {
    u_long value, card_id;
    float fvalue;
    
    //Information display.
    printf("\nGeneral information\n");
    printf("-------------------\n");
    display_card_id();
    display_FW_date();
    display_SVN_version();
    printf("\nInterrupts, descriptors & channels\n");
    printf("----------------------------------\n");
    display_interrupts_descriptors();
    display_channels();
    printf("\n");
    
    
    printf("\nFPGA status\n");
    printf("----------------------------------\n");
    
    flxCard.cfg_get_option(BF_CARD_TYPE, &card_id);
    flxCard.cfg_get_option(BF_FPGA_CORE_TEMP, &value);
    if(card_id == 0x2C7)
      fvalue = (((float)value * 503.975)/4096.0 - 273.15); 
    else
      fvalue = (((float)value * 502.9098)/4096.0 - 273.8195);     
    printf("Temperature:                  %f C\n", fvalue);
    
    flxCard.cfg_get_option(BF_FPGA_CORE_VCCINT, &value);
    fvalue = value * 3.0 / 4096.0;
    printf("Internal voltage  [VccInt]:   %f V\n", fvalue);
    
    flxCard.cfg_get_option(BF_FPGA_CORE_VCCAUX, &value);
    fvalue = value * 3.0 / 4096.0;
    printf("Auxiliary voltage [VccAux]:   %f V\n", fvalue);
    
    flxCard.cfg_get_option(BF_FPGA_CORE_VCCBRAM, &value);
    fvalue = value * 3.0 / 4096.0;
    printf("BRAM voltage      [VccBRAM]:  %f V\n", fvalue);

    printf("\nCentral Router settings\n");
    printf("----------------------------------\n");
    
    flxCard.cfg_get_option(BF_INCLUDE_EPROC16, &value);
    printf("E-proc 16         : %s\n", value?"Yes":"No");
    
    flxCard.cfg_get_option(BF_WIDE_MODE, &value);
    printf("Wide mode         : %s\n", value?"Yes":"No");

    flxCard.cfg_get_option(BF_CR_INTERNAL_LOOPBACK_MODE, &value);
    printf("Internal loopback : %s\n", value?"Yes":"No");

    flxCard.cfg_get_reg(REG_TTC_EMU_CONST, &value);
    printf("TTC test mode     : %s\n", (value & 0x1)?"Yes":"No");
    printf("TTC Emulator      : %s\n", (value & 0x2)?"Yes":"No");

    printf("\nLinks and GBT settings\n");
    printf("----------------------------------\n");

    flxCard.cfg_get_option(BF_NUM_OF_CHANNELS, &value);
    printf("Number of channels    : %d\n", value);
    
    flxCard.cfg_get_option(BF_GENERATE_GBT, &value);
    printf("GBT Wrapper generated : %s\n", value?"Yes":"No");
    
    flxCard.cfg_get_option(BF_OPTO_TRX_NUM, &value);
    printf("Optical transceivers  : %d\n", value & 0xff);

    printf("\nClock resources\n");
    printf("----------------------------------\n");

    flxCard.cfg_get_option(BF_MMCM_MAIN_OSC_SEL, &value);
    printf("Local clock in use  : %s\n", value?"Yes":"No");

    flxCard.cfg_get_option(BF_MMCM_MAIN_PLL_LOCK, &value);
    if(value & 1)
      printf("Internal PLL Lock   : Yes\n");
    else
      printf("Internal PLL Lock   : NO !!\n");

/*
MJ: Do not just reactivate this code. It has bugs.
For more info: Andrea Borga (who else)
    flxCard.i2c_write_byte(0x70, I2C_SWITCH_CXP1);
    if(!check_idt())
      printf("CXP1 @ 240 MHz    : Yes\n");
    else
      printf("CXP1 @ 240 MHz    : NO !!\n");

    flxCard.i2c_write_byte(0x70, I2C_SWITCH_CXP2);
    if(!check_idt())
      printf("CXP2 @ 240 MHz    : Yes\n");
    else
      printf("CXP2 @ 240 MHz    : NO !!\n");
*/
    display_TTC_status();

    if(verbose)
    {
      printf("\n\nADN2814\n");
      flxCard.i2c_write_byte(0x70, I2C_SWITCH_ADN); // Switch to FMC
      u_char freq0, freq1, freq2, rate, misc;
      freq0 = flxCard.i2c_read_byte(I2C_ADDR_ADN, 0x0);
      freq1 = flxCard.i2c_read_byte(I2C_ADDR_ADN, 0x1);
      freq2 = flxCard.i2c_read_byte(I2C_ADDR_ADN, 0x2);
      rate = flxCard.i2c_read_byte(I2C_ADDR_ADN, 0x3);
      misc = flxCard.i2c_read_byte(I2C_ADDR_ADN, 0x4);
      printf(" FREQ0=0x%x\n FREQ1=0x%x\n FREQ2=0x%x\n RATE=0x%x\n MISC=0x%x\n", freq0, freq1, freq2, rate, misc);

      printf("\n\nIDT 8N3Q001 (CXP1)\n");
      flxCard.i2c_write_byte(0x70, I2C_SWITCH_CXP1);
      display_idt();

      printf("\n\nIDT 8N3Q001 (CXP2)\n");
      flxCard.i2c_write_byte(0x70, I2C_SWITCH_CXP2);
      display_idt();
    }
  }

  if(gbt_info)        display_channels_alignment(verbose);
  if(tem_sensor_info) display_tc74();
  if(adn_info)        display_adn2814(verbose);
  if(cxp_info)        display_cxp();
  if(cdce_info)       display_cdce();
  if(ddr3_info)       display_ddr3();
  if(egroup_info)     display_egroups(argc, argv, common_info, arguments);
  if(sfp_info)        display_sfp();
  if(id_eeprom_info)  display_ideeprom();

  try
  {
    flxCard.card_close();
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std::endl; 
    exit(-1);
  }
  
  exit(0);
}
