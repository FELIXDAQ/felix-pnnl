/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-dump-blocks application  */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <cassert>
#include <unistd.h>
#include <stdlib.h>

#include "DFDebug/DFDebug.h"
#include "cmem_rcc/cmem_rcc.h"
//#include "cmem_common.h" // EC, 20-Apr-2016
#include "FlxCard.h"
#include "FlxException.h"

#define DMA_ID              0
#define BUFSIZE             1024
#define APPLICATION_NAME    "flx-dump-blocks"
#define APPLICATION_VERSION "1.0.0"


//Globals
FlxCard flxCard;


void display_help()
{
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Reads raw data from a FLX and writes them into a file.\n\n");
  printf("Options:\n");
  printf("  -d NUMBER      Use card indicated by NUMBER. Default: 0.\n");
  printf("  -D level       Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -n NUMBER      Dump NUMBER blocks. Default: 100.\n");
  printf("  -f FILENAME    Dump blocks into the given file. Default: 'out.blocks'.\n");
  printf("  -F             Enable To-Frontend GBT emulator\n");
  printf("  -h             Display help.\n");
  printf("  -V             Display the version number\n");
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int loop, ret, debuglevel, max_tlp, bsize, handle, opt, device_number = 0, emu_to_frontend = 0;
  u_int nblocks = 100;
  u_long vaddr, paddr, opt_emu_ena_to_host, opt_emu_ena_to_frontend, opt_dnlnk_fo, opt_uplnk_fo;
  const char *filename = "out.blocks";

  while((opt = getopt(argc, argv, "hd:f:n:D:FV")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
	device_number = atoi(optarg);
	break;
	
      case 'f':
	filename = optarg;
	break;
	
      case 'D':
	debuglevel = atoi(optarg);
        DF::GlobalDebugSettings::setup(debuglevel, DFDB_FELIXCARD);
	break;

      case 'n':
	nblocks = atoi(optarg);
	break;

      case 'h':
	display_help();
	exit(0);

      case 'F':
        emu_to_frontend = 1;
        break;
      
      case 'V':
        printf("This is version %s of %s\n", APPLICATION_VERSION, APPLICATION_NAME);
	exit(0);
		
      default: 
	fprintf(stderr, "Usage: %s [OPTIONS]\nTry %s -h for more information.\n", APPLICATION_NAME, APPLICATION_NAME);
	exit(-1);
    }
  }

  try
  {
    flxCard.card_open(device_number);

    // save current state 
    flxCard.cfg_get_option(BF_GBT_EMU_ENA_TOHOST, &opt_emu_ena_to_host);
    flxCard.cfg_get_option(BF_GBT_EMU_ENA_TOFRONTEND, &opt_emu_ena_to_frontend);
    flxCard.cfg_get_option(BF_GBT_DNLNK_FO_SEL, &opt_dnlnk_fo);
    flxCard.cfg_get_option(BF_GBT_UPLNK_FO_SEL, &opt_uplnk_fo);

    flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOHOST, 1);
    flxCard.cfg_set_option(BF_GBT_DNLNK_FO_SEL, 0xffffff);
    flxCard.cfg_set_option(BF_GBT_UPLNK_FO_SEL, 0xffffff);

    if(emu_to_frontend)
    {
      flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, 1);
      flxCard.cfg_set_option(BF_GBT_DNLNK_FO_SEL, 0x0);
      flxCard.cfg_set_option(BF_GBT_UPLNK_FO_SEL, 0xffffff);
    }

    for(loop = 0; loop < 8; loop++)
      flxCard.dma_wait(loop);

    flxCard.dma_reset();
    flxCard.dma_soft_reset();
    flxCard.dma_fifo_flush();
    //flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, 0);
    //flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOHOST, 1);

    ret = CMEM_Open();    
    bsize = BUFSIZE * nblocks;
    if (!ret)
      ret = CMEM_SegmentAllocate(bsize, "FlxThroughput", &handle);
    
    if (!ret)
      ret = CMEM_SegmentPhysicalAddress(handle, &paddr);
   
    if (!ret)
      ret = CMEM_SegmentVirtualAddress(handle, &vaddr);

    if (ret)
    {
      rcc_error_print(stdout, ret);
      exit(-1);
    }  

    FILE *f = fopen(filename, "w");
    if(!f)
    {
      printf("Could not open file for writing\n");
      exit(-1);
    }
  
    max_tlp = flxCard.dma_max_tlp_bytes();

    flxCard.dma_program_write(DMA_ID, paddr, BUFSIZE * nblocks, max_tlp, 0);
    flxCard.dma_wait(DMA_ID);

    if(nblocks != fwrite((void*)(vaddr), 1024, nblocks, f))
    {
      fprintf(stderr, APPLICATION_NAME": I/O Error\n");
      exit(-1);
    }

    fclose(f);
    
    // reset to initial state
    flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOHOST, opt_emu_ena_to_host);
    flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, opt_emu_ena_to_frontend);
    flxCard.cfg_set_option(BF_GBT_DNLNK_FO_SEL, opt_dnlnk_fo);
    flxCard.cfg_set_option(BF_GBT_UPLNK_FO_SEL, opt_uplnk_fo);
    
    ret = CMEM_SegmentFree(handle);
    if (!ret)
      ret = CMEM_Close();
    if (ret)
      rcc_error_print(stdout, ret);

    flxCard.card_close();
  }
  catch(FlxException ex)
  {
    std::cout << "ERROR. Exception thrown: " << ex.what() << std:: endl; 
    exit(-1);
  }
  exit(0);
}
