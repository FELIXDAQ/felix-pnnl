/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the flx-throughput application   */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <time.h>

#include "DFDebug/DFDebug.h"
#include "cmem_rcc/cmem_rcc.h" 
//#include "cmem_common.h"  // EC, 20-Apr-2016
#include "rcc_error/rcc_error.h"
#include "FlxCard.h"
#include "FlxException.h"

#define DMA_ID              0
#define BUFSIZE             1024
//#define BUFSIZE             1024*8

#define APPLICATION_NAME    "flx-throughput"
#define APPLICATION_VERSION "1.0.0"

//Globals
FlxCard flxCard;


/*****************/
void display_help()
/*****************/
{
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Reads raw data from a FLX and writes them into a file.\n\n");
  printf("Options:\n");
  printf("  -d NUMBER      Use card indicated by NUMBER. Default: 0.\n");
  printf("  -b NUM         Use a buffer of size NUM blocks. Default: 100 blocks of 1 KB each.\n");
  printf("  -w             Use circular buffer wraparound mechanism.\n");
  printf("  -D level       Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h             Display help.\n");
  printf("  -V             Display the version number\n");
}


/**********/
double now()
/**********/
{
  struct timespec tp;
  
  clock_gettime(CLOCK_MONOTONIC, &tp);
  return tp.tv_sec + 1e-9 * tp.tv_nsec;
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int debuglevel, handle, ret, loop, device_number = 0, nblocks = 100, wraparound = 0, opt, max_tlp;
  double timedelta = 2, t0, t1;
  u_long bsize, paddr, blocks_read = 0, opt_emu_ena_to_host, opt_emu_ena_to_frontend, opt_dnlnk_fo, opt_uplnk_fo;

  while((opt = getopt(argc, argv, "hd:b:wD:V")) != -1) 
  {
    switch (opt) 
    {
      case 'd':
	device_number = atoi(optarg);
	break;
	
      case 'b':
	nblocks = atoi(optarg);
	std::cout << "DMA read/write chunks will be  " << BUFSIZE * nblocks << std::endl;
	break;
	
      case 'D':
	debuglevel = atoi(optarg);
        DF::GlobalDebugSettings::setup(debuglevel, DFDB_FELIXCARD);
	break;

      case 'w':
	wraparound = 1;
	break;
	
      case 'h':
	display_help();
	exit(0);

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

    for(loop = 0; loop < 8; loop++)
      flxCard.dma_stop(loop);

    flxCard.dma_reset();
    flxCard.dma_soft_reset();
    flxCard.dma_fifo_flush();

    
    // save current state
    flxCard.cfg_get_option(BF_GBT_EMU_ENA_TOHOST, &opt_emu_ena_to_host);
    flxCard.cfg_get_option(BF_GBT_EMU_ENA_TOFRONTEND, &opt_emu_ena_to_frontend);
    flxCard.cfg_get_option(BF_GBT_DNLNK_FO_SEL, &opt_dnlnk_fo);
    flxCard.cfg_get_option(BF_GBT_UPLNK_FO_SEL, &opt_uplnk_fo);

    std::cout << "GBT-EMU-TOFRONTEND, as config'd: " << opt_emu_ena_to_host << std::endl;
    std::cout << "GBT-EMU-TOHOST, as config'd: " << opt_emu_ena_to_frontend << std::endl;

    flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, 0);
    flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOHOST, 1);
        
    ret = CMEM_Open();    
    bsize = BUFSIZE * nblocks;
    if (!ret)
      ret = CMEM_GFPBPASegmentAllocate(bsize, "FlxThroughput", &handle);
    
    if (!ret)
      ret = CMEM_SegmentPhysicalAddress(handle, &paddr);
  
    if (ret)
    {
      rcc_error_print(stdout, ret);
      exit(-1);
    }

    max_tlp = flxCard.dma_max_tlp_bytes();
    t0 = now();

    if(wraparound)
    {
      fprintf(stderr, "TODO: wraparound support not fully implemented yet.\n");
      for (int jj=0; jj<8; jj++)
	flxCard.dma_program_write(DMA_ID+jj, paddr, BUFSIZE * nblocks, max_tlp, FLX_DMA_WRAPAROUND);
    }

    bool once (true);
    while(1)
    {
      if(wraparound)
      {
	// TODO: Wait for data interrupt

	flxCard.dma_advance_read_ptr(DMA_ID, paddr, bsize, BUFSIZE * nblocks);
	blocks_read += nblocks;
      }
      else
      {
	for (int jj=0; jj<8; jj++)
	  flxCard.dma_program_write(DMA_ID+jj, paddr, BUFSIZE * nblocks, max_tlp, 0); //EC: only moves read/write pointers around and enables DMA

	for (int jj=0; jj<8; jj++)
	  {
	    if (once) std::cout << " DMA_ID is " << DMA_ID+jj << std::endl;
	    flxCard.dma_wait(DMA_ID+jj); // Must be fpga code which sets to not enable dma'ing, and thus breaks us out of the while loop in dma_wait()?!
	    blocks_read += nblocks;
	  }
	once = false;
      }

      t1 = now();

      if((t1 - t0) > timedelta)
      {

	printf("Blocks/time read:  %lu, %f\n", blocks_read, t1-t0);
	printf("Blocks rate:  %f blocks/s\n", blocks_read / (t1 - t0));

        double dmaperformance =  (double)blocks_read * (double)BUFSIZE / ((t1 - t0) * 1024. * 1024. * 1024.);
	printf("DMA Read:     %f GiB/s\n", dmaperformance);

	printf("\n");
	blocks_read = 0;
	t0 = t1;
      }
    }

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

