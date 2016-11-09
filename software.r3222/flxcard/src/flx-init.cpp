/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-init application         */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2016 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

#define APPLICATION_NAME             "flx-init"
#define APPLICATION_VERSION          "1.0.0"
#define FLX_INIT_MAX_CDCE_ITERATIONS 1000
#define REGS_PAGE_0                  183
#define REGS_PAGE_1                  52


//data arrays for setting the TTC clock
u_char si5338_set_page_0[2] = {255, 0};  // set page bit to 0
u_char si5338_set_page_1[2] = {255, 1};  // set page bit to 1

u_char ttc40079_page_0[REGS_PAGE_0][2] =
{
  {  6, 0x00},{ 27, 0x70},{ 28, 0x03},{ 29, 0x41},{ 30, 0x41},{ 31, 0xC0},{ 32, 0xC0},{ 33, 0xC0},{ 34, 0xC0},{ 35, 0xAA},
  { 36, 0x06},{ 37, 0x06},{ 38, 0x06},{ 39, 0x06},{ 40, 0x84},{ 41, 0x10},{ 42, 0x24},{ 45, 0x00},{ 46, 0x00},{ 47, 0x14},
  { 48, 0xBA},{ 49, 0x10},{ 50, 0xC5},{ 51, 0x07},{ 52, 0x10},{ 53, 0x00},{ 54, 0x03},{ 55, 0x00},{ 56, 0x00},{ 57, 0x00},
  { 58, 0x00},{ 59, 0x01},{ 60, 0x00},{ 61, 0x00},{ 62, 0x00},{ 63, 0x10},{ 64, 0x00},{ 65, 0x03},{ 66, 0x00},{ 67, 0x00},
  { 68, 0x00},{ 69, 0x00},{ 70, 0x01},{ 71, 0x00},{ 72, 0x00},{ 73, 0x00},{ 74, 0x10},{ 75, 0x00},{ 76, 0x03},{ 77, 0x00},
  { 78, 0x00},{ 79, 0x00},{ 80, 0x00},{ 81, 0x01},{ 82, 0x00},{ 83, 0x00},{ 84, 0x00},{ 85, 0x10},{ 86, 0x00},{ 87, 0x1C},
  { 88, 0x00},{ 89, 0x00},{ 90, 0x00},{ 91, 0x00},{ 92, 0x01},{ 93, 0x00},{ 94, 0x00},{ 95, 0x00},{ 97, 0x00},{ 98, 0x3A},
  { 99, 0x00},{100, 0x00},{101, 0x00},{102, 0x00},{103, 0x01},{104, 0x00},{105, 0x00},{106, 0x80},{107, 0x00},{108, 0x00},
  {109, 0x00},{110, 0x40},{111, 0x00},{112, 0x00},{113, 0x00},{114, 0x40},{115, 0x00},{116, 0x80},{117, 0x00},{118, 0x40},
  {119, 0x00},{120, 0x00},{121, 0x00},{122, 0xC0},{123, 0x00},{124, 0x00},{125, 0x00},{126, 0x00},{127, 0x00},{128, 0x00},
  {129, 0x00},{130, 0x00},{131, 0x00},{132, 0x00},{133, 0x00},{134, 0x00},{135, 0x00},{136, 0x00},{137, 0x00},{138, 0x00},
  {139, 0x00},{140, 0x00},{141, 0x00},{142, 0x00},{143, 0x00},{144, 0x00},{152, 0x00},{153, 0x00},{154, 0x00},{155, 0x00},
  {156, 0x00},{157, 0x00},{158, 0x00},{159, 0x00},{160, 0x00},{161, 0x00},{162, 0x00},{163, 0x00},{164, 0x00},{165, 0x00},
  {166, 0x00},{167, 0x00},{168, 0x00},{169, 0x00},{170, 0x00},{171, 0x00},{172, 0x00},{173, 0x00},{174, 0x00},{175, 0x00},
  {176, 0x00},{177, 0x00},{178, 0x00},{179, 0x00},{180, 0x00},{181, 0x00},{182, 0x00},{183, 0x00},{184, 0x00},{185, 0x00},
  {186, 0x00},{187, 0x00},{188, 0x00},{189, 0x00},{190, 0x00},{191, 0x00},{192, 0x00},{193, 0x00},{194, 0x00},{195, 0x00},
  {196, 0x00},{197, 0x00},{198, 0x00},{199, 0x00},{200, 0x00},{201, 0x00},{202, 0x00},{203, 0x00},{204, 0x00},{205, 0x00},
  {206, 0x00},{207, 0x00},{208, 0x00},{209, 0x00},{210, 0x00},{211, 0x00},{212, 0x00},{213, 0x00},{214, 0x00},{215, 0x00},
  {216, 0x00},{217, 0x00},{242, 0x00}
};

u_char ttc160_page_0[REGS_PAGE_0][2] =
{
  {  6, 0x00},{ 27, 0x70},{ 28, 0x03},{ 29, 0x42},{ 30, 0x42},{ 31, 0xC0},{ 32, 0xC0},{ 33, 0xC0},{ 34, 0xC0},{ 35, 0xAA},
  { 36, 0x06},{ 37, 0x06},{ 38, 0x06},{ 39, 0x06},{ 40, 0x84},{ 41, 0x10},{ 42, 0x24},{ 45, 0x00},{ 46, 0x00},{ 47, 0x14},
  { 48, 0x9D},{ 49, 0x10},{ 50, 0xC5},{ 51, 0x07},{ 52, 0x10},{ 53, 0x00},{ 54, 0x03},{ 55, 0x00},{ 56, 0x00},{ 57, 0x00},
  { 58, 0x00},{ 59, 0x01},{ 60, 0x00},{ 61, 0x00},{ 62, 0x00},{ 63, 0x10},{ 64, 0x00},{ 65, 0x03},{ 66, 0x00},{ 67, 0x00},
  { 68, 0x00},{ 69, 0x00},{ 70, 0x01},{ 71, 0x00},{ 72, 0x00},{ 73, 0x00},{ 74, 0x10},{ 75, 0x00},{ 76, 0x03},{ 77, 0x00},
  { 78, 0x00},{ 79, 0x00},{ 80, 0x00},{ 81, 0x01},{ 82, 0x00},{ 83, 0x00},{ 84, 0x00},{ 85, 0x10},{ 86, 0x80},{ 87, 0x05},
  { 88, 0x00},{ 89, 0x00},{ 90, 0x00},{ 91, 0x00},{ 92, 0x01},{ 93, 0x00},{ 94, 0x00},{ 95, 0x00},{ 97, 0x00},{ 98, 0x1C},
  { 99, 0x00},{100, 0x00},{101, 0x00},{102, 0x00},{103, 0x01},{104, 0x00},{105, 0x00},{106, 0x80},{107, 0x00},{108, 0x00},
  {109, 0x00},{110, 0xC0},{111, 0x00},{112, 0x00},{113, 0x00},{114, 0xC0},{115, 0x00},{116, 0x80},{117, 0x00},{118, 0xC0},
  {119, 0x00},{120, 0x00},{121, 0x00},{122, 0xC0},{123, 0x00},{124, 0x00},{125, 0x00},{126, 0x00},{127, 0x00},{128, 0x00},
  {129, 0x00},{130, 0x00},{131, 0x00},{132, 0x00},{133, 0x00},{134, 0x00},{135, 0x00},{136, 0x00},{137, 0x00},{138, 0x00},
  {139, 0x00},{140, 0x00},{141, 0x00},{142, 0x00},{143, 0x00},{144, 0x00},{152, 0x00},{153, 0x00},{154, 0x00},{155, 0x00},
  {156, 0x00},{157, 0x00},{158, 0x00},{159, 0x00},{160, 0x00},{161, 0x00},{162, 0x00},{163, 0x00},{164, 0x00},{165, 0x00},
  {166, 0x00},{167, 0x00},{168, 0x00},{169, 0x00},{170, 0x00},{171, 0x00},{172, 0x00},{173, 0x00},{174, 0x00},{175, 0x00},
  {176, 0x00},{177, 0x00},{178, 0x00},{179, 0x00},{180, 0x00},{181, 0x00},{182, 0x00},{183, 0x00},{184, 0x00},{185, 0x00},
  {186, 0x00},{187, 0x00},{188, 0x00},{189, 0x00},{190, 0x00},{191, 0x00},{192, 0x00},{193, 0x00},{194, 0x00},{195, 0x00},
  {196, 0x00},{197, 0x00},{198, 0x00},{199, 0x00},{200, 0x00},{201, 0x00},{202, 0x00},{203, 0x00},{204, 0x00},{205, 0x00},
  {206, 0x00},{207, 0x00},{208, 0x00},{209, 0x00},{210, 0x00},{211, 0x00},{212, 0x00},{213, 0x00},{214, 0x00},{215, 0x00},
  {216, 0x00},{217, 0x00},{242, 0x00}
};

u_char ttc40_page_0[REGS_PAGE_0][2] =
{
{  6, 0x00},{ 27, 0x70},{ 28, 0x03},{ 29, 0x00},{ 30, 0x40},{ 31, 0xC0},{ 32, 0xC0},{ 33, 0xC0},{ 34, 0xC0},{ 35, 0xAA},
{ 36, 0x06},{ 37, 0x06},{ 38, 0x06},{ 39, 0x06},{ 40, 0x84},{ 41, 0x10},{ 42, 0x24},{ 45, 0x00},{ 46, 0x00},{ 47, 0x14},
{ 48, 0x9D},{ 49, 0x10},{ 50, 0xC5},{ 51, 0x07},{ 52, 0x10},{ 53, 0x00},{ 54, 0x03},{ 55, 0x00},{ 56, 0x00},{ 57, 0x00},
{ 58, 0x00},{ 59, 0x01},{ 60, 0x00},{ 61, 0x00},{ 62, 0x00},{ 63, 0x10},{ 64, 0x00},{ 65, 0x03},{ 66, 0x00},{ 67, 0x00},
{ 68, 0x00},{ 69, 0x00},{ 70, 0x01},{ 71, 0x00},{ 72, 0x00},{ 73, 0x00},{ 74, 0x10},{ 75, 0x00},{ 76, 0x03},{ 77, 0x00},
{ 78, 0x00},{ 79, 0x00},{ 80, 0x00},{ 81, 0x01},{ 82, 0x00},{ 83, 0x00},{ 84, 0x00},{ 85, 0x10},{ 86, 0x00},{ 87, 0x1C},
{ 88, 0x00},{ 89, 0x00},{ 90, 0x00},{ 91, 0x00},{ 92, 0x01},{ 93, 0x00},{ 94, 0x00},{ 95, 0x00},{ 97, 0x00},{ 98, 0x1C},
{ 99, 0x00},{100, 0x00},{101, 0x00},{102, 0x00},{103, 0x01},{104, 0x00},{105, 0x00},{106, 0x80},{107, 0x00},{108, 0x00},
{109, 0x00},{110, 0xC0},{111, 0x00},{112, 0x00},{113, 0x00},{114, 0xC0},{115, 0x00},{116, 0x80},{117, 0x00},{118, 0xC0},
{119, 0x00},{120, 0x00},{121, 0x00},{122, 0xC0},{123, 0x00},{124, 0x00},{125, 0x00},{126, 0x00},{127, 0x00},{128, 0x00},
{129, 0x00},{130, 0x00},{131, 0x00},{132, 0x00},{133, 0x00},{134, 0x00},{135, 0x00},{136, 0x00},{137, 0x00},{138, 0x00},
{139, 0x00},{140, 0x00},{141, 0x00},{142, 0x00},{143, 0x00},{144, 0x00},{152, 0x00},{153, 0x00},{154, 0x00},{155, 0x00},
{156, 0x00},{157, 0x00},{158, 0x00},{159, 0x00},{160, 0x00},{161, 0x00},{162, 0x00},{163, 0x00},{164, 0x00},{165, 0x00},
{166, 0x00},{167, 0x00},{168, 0x00},{169, 0x00},{170, 0x00},{171, 0x00},{172, 0x00},{173, 0x00},{174, 0x00},{175, 0x00},
{176, 0x00},{177, 0x00},{178, 0x00},{179, 0x00},{180, 0x00},{181, 0x00},{182, 0x00},{183, 0x00},{184, 0x00},{185, 0x00},
{186, 0x00},{187, 0x00},{188, 0x00},{189, 0x00},{190, 0x00},{191, 0x00},{192, 0x00},{193, 0x00},{194, 0x00},{195, 0x00},
{196, 0x00},{197, 0x00},{198, 0x00},{199, 0x00},{200, 0x00},{201, 0x00},{202, 0x00},{203, 0x00},{204, 0x00},{205, 0x00},
{206, 0x00},{207, 0x00},{208, 0x00},{209, 0x00},{210, 0x00},{211, 0x00},{212, 0x00},{213, 0x00},{214, 0x00},{215, 0x00},
{216, 0x00},{217, 0x00},{242, 0x00}
};

u_char ttc_page_1[REGS_PAGE_1][2] =
{
  {31, 0x00},{32, 0x00},{33, 0x01},{34, 0x00},{35, 0x00},{36, 0x90},{37, 0x31},{38, 0x00},{39, 0x00},{40, 0x01},
  {41, 0x00},{42, 0x00},{43, 0x00},{47, 0x00},{48, 0x00},{49, 0x01},{50, 0x00},{51, 0x00},{52, 0x90},{53, 0x31},
  {54, 0x00},{55, 0x00},{56, 0x01},{57, 0x00},{58, 0x00},{59, 0x00},{63, 0x00},{64, 0x00},{65, 0x01},{66, 0x00},
  {67, 0x00},{68, 0x90},{69, 0x31},{70, 0x00},{71, 0x00},{72, 0x01},{73, 0x00},{74, 0x00},{75, 0x00},{79, 0x00},
  {80, 0x00},{81, 0x00},{82, 0x00},{83, 0x00},{84, 0x90},{85, 0x31},{86, 0x00},{87, 0x00},{88, 0x01},{89, 0x00},
  {90, 0x00},{91, 0x00}
};


//Globals
FlxCard flxCard;
u_long baraddr2;
int verbose = 0;

/*****************/
void display_help()
/*****************/
{
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Initializes a FLX device.\n\n");
  printf("General options:\n");
  printf("  -d NUMBER                    Use card indicated by NUMBER. Default: 0.\n");
  printf("  -h                           Display help.\n");
  printf("  -D level                     Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -v                           Display verbose output.\n");
  printf("  -vv                          Super verbose mode.\n");
  printf("  -V                           Display the version number\n");
  printf("GBT calibration options:\n");
  printf("  -s SOFT|FIRM                 Use SOFTWARE (SOFT) or FIRMWARE (FSM) alignment. Default: SOFT.\n");
  printf("  -a ONE|CONTIUNOUS            Select alignment type. Default: ONE.\n");
  printf("  -t FEC|WideBus               Select transmission mode. Default: FEC.\n");
  printf("  -e YES|NO                    Use the descrambler output value. Default: NO.\n");
  printf("  -f FILENAME                  Specify which file will be used to calibrate the delay. Default: ../flx_propagation_delay.conf)\n");
  printf("TTC calibration options:\n");
  printf("  -C                           Initialize the CDCE chip (incompatible with -T)\n");
  printf("  -G                           Get and display the status of the SI5338\n");
  printf("  -T  Frequency                Set the TTC clock to a given frequency\n");
  printf("                               Legal values for frequency are:\n");
  printf("                               1 = 40 MHz\n");
  printf("                               2 = 40.079 MHz (default)\n");
  printf("                               3 = 160 MHz\n");

}

/***********************/
void str_upper(char *str) 
/***********************/
{
  do 
  {
    *str = toupper((u_char) *str);
  } while (*str++);
}

/*********************/
void ttc_get_regs(void)
/*********************/
{
  u_int loop;
  u_char regdata;

  if(verbose)
    printf("Switching to page 0\n");

  flxCard.i2c_devices_write("FMC_SI5338", si5338_set_page_0[0], si5338_set_page_0[1]); //709

  for (loop = 0; loop < REGS_PAGE_0; loop++)
  {
    if(verbose)
      printf("Reading from register %d on page 0\n", ttc40_page_0[loop][0]);
    flxCard.i2c_devices_read("FMC_SI5338", ttc40_page_0[loop][0], &regdata);
    printf("P: 0, A: %3d = 0x%02x\n", ttc40_page_0[loop][0], regdata);
  }

  if(verbose)
    printf("Now switching to page 1\n");
  flxCard.i2c_devices_write("FMC_SI5338", si5338_set_page_1[0], si5338_set_page_1[1]);

  for (loop = 0; loop < REGS_PAGE_1; loop++)
  {
    if(verbose)
      printf("Reading from register %d on page 1\n", ttc_page_1[loop][0]);
    flxCard.i2c_devices_read("FMC_SI5338", ttc_page_1[loop][0], &regdata);
    printf("P: 1, A: %3d = 0x%02x\n", ttc_page_1[loop][0], regdata);
  }

  if(verbose)
    printf("Now switching back to page 0\n");
  flxCard.i2c_devices_write("FMC_SI5338", si5338_set_page_0[0], si5338_set_page_0[1]);

  printf("Register dump complete.\n");
}

/**************************/
void ttc_clock_set(int freq)
/**************************/
{
  u_int loop;
  u_char dummy;

  if(verbose)
    printf("Switching to page 0\n");
  flxCard.i2c_devices_write("1:0x70", si5338_set_page_0[0], si5338_set_page_0[1]);

  for (loop = 0; loop < REGS_PAGE_0; loop++)
  {
    if (freq == 1)
    {
      if(verbose)
        printf("Writing 0x%x to register %d on page 0\n", ttc40_page_0[loop][1], ttc40_page_0[loop][0]);
      flxCard.i2c_devices_write("1:0x70", ttc40_page_0[loop][0], ttc40_page_0[loop][1]);
    }
    else if (freq == 2)
    {
      if(verbose)
        printf("Writing 0x%x to register %d on page 0\n", ttc40079_page_0[loop][1], ttc40079_page_0[loop][0]);
      flxCard.i2c_devices_write("1:0x70", ttc40079_page_0[loop][0], ttc40079_page_0[loop][1]);
    }
    else
    {
      if(verbose)
        printf("Writing 0x%x to register %d on page 0\n", ttc160_page_0[loop][1], ttc160_page_0[loop][0]);
      flxCard.i2c_devices_write("1:0x70", ttc160_page_0[loop][0], ttc160_page_0[loop][1]);
    }
  }

  if(verbose)
    printf("Now switching to page 1\n");
  flxCard.i2c_devices_write("1:0x70", si5338_set_page_1[0], si5338_set_page_1[1]);

  for (loop = 0; loop < REGS_PAGE_1; loop++)
  {
    if(verbose)
      printf("Writing 0x%x to register %d on page 1\n", ttc_page_1[loop][1], ttc_page_1[loop][0]);
    flxCard.i2c_devices_write("1:0x70", ttc_page_1[loop][0], ttc_page_1[loop][1]);
  }
  
  if(verbose)
    printf("Now switching back to page 0\n");
  flxCard.i2c_devices_write("1:0x70", si5338_set_page_0[0], si5338_set_page_0[1]);

  flxCard.i2c_devices_write("1:0x70", 246, 2);
  sleep(2);
  flxCard.i2c_devices_read("1:0x70", 246, &dummy);
  sleep(2);
  flxCard.i2c_devices_write("1:0x70", 246, 0);
  sleep(2);
  flxCard.i2c_devices_read("1:0x70", 246, &dummy);

  printf("TTC clock configuration done.\n");
}


/*********************************************************************/
static int check_digic_value(char *str, u_long *version, u_long *delay)
/*********************************************************************/
{
  char *p_aux = NULL;
  char *pos = strchr(str, ':');

  if(pos == NULL)
    return -1;
    
  *pos = '\0';

  *version = strtoul(str, &p_aux, 0);
  if(*p_aux != '\0')
    return -2;

  *delay = strtoul(pos+1, &p_aux, 0);
  if(*delay == 0)
    return -3;

  return 0;
}


/************************************************************/
long int gbt_version_delay(u_long svn_version, char *filename)
/************************************************************/
{
  FILE *file_delay;
  int control = 0;
  u_long delay = 0, f_version = 0, f_delay = 0;
  char data[1024] = "\0", *fileused, file_default[] = "../flx_propagation_delay.conf";

  if(filename == NULL)
    fileused = file_default;
  else 
    fileused = filename;
  
  if((file_delay = fopen(fileused, "r")) != NULL)
  {
    while(control == 0 && fgets(data, 1024, file_delay) != NULL)
    {
      if(check_digic_value(data, &f_version, &f_delay) == 0)
      {
        if(f_version == svn_version)
        {
          delay = f_delay;
          control = 1;
        }
      }
    }
    fclose(file_delay);
    
    if(control == 0)
      delay = FLX_GBT_VERSION_NOT_FOUND;
  }
  else 
    delay = FLX_GBT_FILE_NOT_FOUND;

  return delay;
}


/************************/
static int init_cdce(void)     ///Initializes CDCE jitter cleaner
/************************/
{
  int i;
  u_long hk_mon = 0;

  for(i = 0; i < FLX_INIT_MAX_CDCE_ITERATIONS; i++)
  {
    flxCard.cfg_set_reg(REG_HK_CTRL_CDCE, 0);                   // default all signals to 0
    flxCard.cfg_set_reg(REG_HK_CTRL_CDCE, 0x06);                // select the REF_SEL and power the chip asserting PD
    flxCard.cfg_set_option(BF_HK_CTRL_CDCE_SYNC, 1);    // Synch the clock input ON
    flxCard.cfg_set_option(BF_HK_CTRL_CDCE_SYNC, 0);    // Synch the clock input OFF

    usleep(1000);

    flxCard.cfg_get_option(BF_HK_MON_CDCE_PLL_LOCK, &hk_mon);

    if(hk_mon)
      return 0;
  }

  return 1;
}


/********************/
void init_idt240(void)        //Initializes IDT chips to 240 MHz
/********************/
{
  int i, switch_ports[] = {I2C_SWITCH_CXP1, I2C_SWITCH_CXP2};

  for(i = 0; i < 2; i++)
  {
    flxCard.i2c_write_byte(0x70, switch_ports[i]);   // configure switch

    // configure 240 MHz
    flxCard.i2c_write_byte_to_addr(I2C_ADDR_CXP, 0x12, 0xA8);
    flxCard.i2c_write_byte_to_addr(I2C_ADDR_CXP, 0x00, 0x2A);
    flxCard.i2c_write_byte_to_addr(I2C_ADDR_CXP, 0x04, 0x00);
    flxCard.i2c_write_byte_to_addr(I2C_ADDR_CXP, 0x08, 0x11);
    flxCard.i2c_write_byte_to_addr(I2C_ADDR_CXP, 0x0C, 0x0A);
    flxCard.i2c_write_byte_to_addr(I2C_ADDR_CXP, 0x14, 0x1F);
    flxCard.i2c_write_byte_to_addr(I2C_ADDR_CXP, 0x12, 0xA0);
  }
}


/*************************************************************/
static long int delay_check(u_long svn_version, char *filename)
/*************************************************************/
{
  long delay = 0;

  flxCard.cfg_get_option(BF_BOARD_ID_SVN,&svn_version);
  delay = gbt_version_delay(svn_version, filename);
  return delay;
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  u_long svn_version=0;
  long delay = 0;
  int device_number = 0, opt, alignment_mode = FLX_GBT_ALIGNMENT_ONE, transmision_mode = FLX_GBT_TMODE_FEC;
  int descrambler_enable = 0;            //Use descrambler recommended values?  1=YES, 0=NO
  int soft_alignmet = 1;                 //Software Alignment==1; Firmware alignment==0
  int ttc_freq = 2, get_ttc_regs = 0, set_ttc_clock = 0, set_cdce = 0;
  int channel_count = FLX_GBT_CHANNEL_AUTO, channel_number = 24, debuglevel;
  char *filename = NULL;

  while((opt = getopt(argc, argv, "phvd:a:t:e:s:f:D:T:CGV")) != -1)
  {
    switch (opt) 
    {
    case 'v':
      verbose++;
      break;
      
    case 'd':
      device_number = atoi(optarg);
      break;
	
    case 'D':
      debuglevel = atoi(optarg);
      DF::GlobalDebugSettings::setup(debuglevel, DFDB_FELIXCARD);
      break;

    case 'V':
      printf("This is version %s of %s\n", APPLICATION_VERSION, APPLICATION_NAME);
      exit(0);
		
    case 's':
      if(0 == strcasecmp(optarg, "SOFT"))
        soft_alignmet = 1;
      else
      {
        if(strcasecmp(optarg, "FIRM") == 0)
          soft_alignmet=0;
        else
        {
          fprintf(stderr, APPLICATION_NAME": error: Wrong platform alignment\n");
          exit(-1);
        }
      }
      break;
      
    case 'a':
      if(strcasecmp(optarg, "ONE") == 0)
        alignment_mode = FLX_GBT_ALIGNMENT_ONE;
      else
      {
        if(strcasecmp(optarg, "CONTINUOUS") == 0)
          alignment_mode = FLX_GBT_ALIGNMENT_CONTINUOUS;
        else
        {
          fprintf(stderr, APPLICATION_NAME": error: Wrong alignment mode\n");
          exit(-1);
        }
      }
      break;

    case 't':
      if(strcasecmp(optarg, "FEC") == 0)
        transmision_mode = FLX_GBT_TMODE_FEC;
      else
      {
        if(strcasecmp(optarg, "WideBus") == 0)
          transmision_mode = FLX_GBT_TMODE_WideBus;
        else
        {
          fprintf(stderr, APPLICATION_NAME": error: Wrong transmision mode\n");
          exit(-1);
        }
      }
      break;
	
    case 'e':
      if(strcasecmp(optarg, "YES") == 0)
        descrambler_enable = 1;
      else
      {
        if(strcasecmp(optarg, "NO") == 0)
          descrambler_enable = 0;
        else
        {
          fprintf(stderr, APPLICATION_NAME": error: Wrong option for descrambler\n");
          exit(-1);
        }
      }
      break;
	
    case 'f':
      filename = optarg;
      break;

    case 'h':
      display_help();
      exit(0);
      
    case 'T':
      ttc_freq = atoi(optarg);
      set_ttc_clock = 1;
      break;
 
    case 'C':
      set_cdce = 1;
      break;

    case 'G':
      get_ttc_regs = 1;
      break;
     
    default:
      fprintf(stderr, "Usage: %s COMMAND [OPTIONS]\nTry %s -h for more information.\n", APPLICATION_NAME, APPLICATION_NAME);
      exit(-1);
    }
  }

  if(verbose)
  {
    printf("\nWelcome to flx-init tool!\n");
    printf("\n---PARAMETERS USED---\n");
    printf("Device number=        %d\n", device_number);
    
    if(soft_alignmet == 1) 
      printf("Alignment control=    SOFTWARE\n");
    else 
      printf("Alignment control=    FIRMWARE\n");

    if(alignment_mode == FLX_GBT_ALIGNMENT_ONE) 
      printf("Alignment mode=       ONE ALIGNMENT\n");
    else 
      printf("Alignment mode=       CONTINUOUS ALIGNMENT\n");

    if(transmision_mode == FLX_GBT_TMODE_FEC) 
      printf("Transmision mode=     FEC\n");
    else 
      printf("Transmision mode=     WIDE BUS\n");

    if(descrambler_enable == 1) 
      printf("Descrambler enable=   YES\n");
    else 
      printf("Descrambler enable=   NO\n");

    if(channel_count == FLX_GBT_CHANNEL_AUTO) 
      printf("Channels detection=   AUTO\n");
    else 
    {
      printf("Channels detection=   MANUAL\n");
      printf("Number of channels=   %d\n", channel_number);
    }
    
    if(filename == 0) 
      printf("Delay value=          Default file\n\n");
    else 
      printf("Delay value=         %s\n\n", filename);
  }

  if (verbose) 
    printf("Opening device.....\n");

  try
  {
    flxCard.card_open(device_number);

    if(get_ttc_regs)
      ttc_get_regs();

    if (set_ttc_clock)
      ttc_clock_set(ttc_freq);

    baraddr2 = flxCard.openBackDoor(2);

    if (set_cdce)
    {
      if (verbose) 
        printf("Initializing CDCE.....");
      if(init_cdce() != 0)
        fprintf(stderr, APPLICATION_NAME": warning: could not initialize CDCE\n");
      else if (verbose) 
        printf("CDCE OK\n");
    }

    if (verbose) 
      printf("Initializing IDT clocks.....");
    init_idt240();

    if (verbose) 
      printf("\nLoading initial configuration.....\n");
    
    flxCard.cfg_get_option(BF_BOARD_ID_SVN, &svn_version);

    if(filename != NULL)
    {
      delay = delay_check(svn_version, filename);
      if(delay == FLX_GBT_FILE_NOT_FOUND)
        fprintf(stderr, APPLICATION_NAME": warning: could not find the specified file\n");
      if(delay == FLX_GBT_VERSION_NOT_FOUND)
        fprintf(stderr, APPLICATION_NAME": warning: could not find the actual version on the propagation delay file\n");
    }

    if (verbose) 
      printf("Firmware version= %ld\n\n", svn_version);
      
    int bad_channels = flxCard.gbt_setup(soft_alignmet, alignment_mode, transmision_mode, descrambler_enable, svn_version, delay);

    if (verbose) 
      printf("\nConfiguration loaded!!!\n");

    if (bad_channels)
    {
      fprintf(stderr, APPLICATION_NAME": warning: Not all channels align!\n");
      fprintf(stderr, APPLICATION_NAME": warning: %d channels not aligned\n", bad_channels); 
    }
    else if (verbose)
      fprintf(stderr, APPLICATION_NAME": All channels align!\n");


    if (verbose) 
      printf("\nClosing device.....");
    
    flxCard.card_close();
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
    exit(-1);
  }
      
  exit(0);
}




