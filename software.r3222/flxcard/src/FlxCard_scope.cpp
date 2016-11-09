/*******************************************************************/
/*                                                                 */
/* This is the test program for the FlxCard object                 */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>

#include "FlxCard.h"
#include "DFDebug/DFDebug.h"
#include "ROSGetInput/get_input.h"


//Globals
FlxCard flxCards[MAXCARDS];

// prototypes
int mainhelp(void);
int setdebug(void);
void checkoffset(void);
void dumpreg(void);


/*****************************/
int main(int argc, char **argv)
/*****************************/
{  
  int fun = 1;
  
  printf("This is FlxCard_Scope\n");
  
  while(fun != 0)
  {
    printf("\n");
    printf("Select an option:\n");
    printf("   1 Help                        2 Set debug parameters\n");
    printf("   2 Check register offsets      4 Dump registers\n");
    printf("   0 Quit\n");
    printf("Your choice: ");
    fun = getdecd(fun);
    if (fun == 1) mainhelp();
    if (fun == 2) setdebug();
    if (fun == 3) checkoffset();
    if (fun == 4) dumpreg();
  }
}


/****************/
void dumpreg(void)
/****************/
{
  static int cardnum;
  u_long baraddr;
  int loop;
  flxcard_bar0_regs_t *bar0;
  flxcard_bar1_regs_t *bar1;

  printf("Enter the number (0...N-1) of the card to access\n");
  cardnum = getdecd(0);
  try
  {
    flxCards[0].card_open(cardnum);

    baraddr = flxCards[0].openBackDoor(0);
    bar0 = (flxcard_bar0_regs_t *)baraddr;
    baraddr = flxCards[0].openBackDoor(1);
    bar1 = (flxcard_bar1_regs_t *)baraddr;

    printf("Dumping (some) registers of BAR0\n");
    printf("BAR0_VALUE      = 0x%08x\n", bar0->BAR0_VALUE);
    printf("BAR1_VALUE      = 0x%08x\n", bar0->BAR1_VALUE);
    printf("BAR2_VALUE      = 0x%08x\n", bar0->BAR2_VALUE);
    printf("DMA_DESC_ENABLE = 0x%02x\n", bar0->DMA_DESC_ENABLE);
    
    printf("\nDumping all registers of BAR1\n");
    printf("| Nr. |    Control |       Data |            Address |\n");
    printf("|-----|------------|------------|--------------------|\n");
    for (loop = 0; loop < 8; loop++)
      printf("|   %d | 0x%08x | 0x%08x | 0x%016x |\n", loop, bar1->INT_VEC[loop].control, bar1->INT_VEC[loop].data, bar1->INT_VEC[loop].address);
    
    printf("\nINT_TAB_ENABLE  = 0x%02x\n", bar1->INT_TAB_ENABLE);
       
    flxCards[0].card_close();
  }

  catch(int n)  //MJ: replace "int n" by the proper type once the exception class is defined
  {
    printf("Error\n");
  }
}


/********************/
void checkoffset(void)
/********************/
{
  flxcard_bar0_regs_t *ptr;
  ptr = (flxcard_bar0_regs_t *)0;
  printf("ptr->DMA_DESC[0].start_address          is at 0x%016lx\n", &ptr->DMA_DESC[0].start_address);
  printf("ptr->DMA_DESC[0].end_address            is at 0x%016lx\n", &ptr->DMA_DESC[0].end_address);
  printf("ptr->DMA_DESC_STATUS[0].current_address is at 0x%016lx\n", &ptr->DMA_DESC_STATUS[0].current_address);
  printf("ptr->BAR1_VALUE                         is at 0x%016lx\n", &ptr->BAR1_VALUE);
  printf("ptr->SOFT_RESET                         is at 0x%016lx\n\n", &ptr->SOFT_RESET);

  flxcard_bar1_regs_t *ptr1;
  ptr1 = (flxcard_bar1_regs_t *)0;
  printf("ptr->INT_VEC[0].control                 is at 0x%016lx\n", &ptr1->INT_VEC[0].control);
  printf("ptr->INT_TAB_ENABLE                     is at 0x%016lx\n\n", &ptr1->INT_TAB_ENABLE);
}


/****************/
int setdebug(void)
/****************/
{
  static u_int dblevel = 0, dbpackage = DFDB_FELIXCARD;
  
  printf("Enter the debug level: ");
  dblevel = getdecd(dblevel);
  printf("Enter the debug package: ");
  dbpackage = getdecd(dbpackage);
  DF::GlobalDebugSettings::setup(dblevel, dbpackage);
  return(0);
}


/****************/
int mainhelp(void)
/****************/
{
  printf("\n=========================================================================\n");
  printf("Call Markus Joos, 72364, 160663 if you need more help\n");
  printf("=========================================================================\n\n");
  return(0);
}
