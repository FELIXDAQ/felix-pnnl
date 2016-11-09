/*                                                                 */
/* This is the C++ source code of the flx-dma-test application     */
/*                                                                 */
/* Author: Markus Joos, CERN                                       */
/*                                                                 */
/**C 2015 Ecosoft - Made from at least 80% recycled source code*****/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "DFDebug/DFDebug.h"
#include "cmem_rcc/cmem_rcc.h" // CMEM_* declarations
//#include "cmem_common.h" // EC, 20-Apr-2016
#include "FlxCard.h"
#include "FlxException.h"


#define APPLICATION_NAME    "flx-dma-test"
#define APPLICATION_VERSION "1.0.0"
#define BUFSIZE (1024)
#define DMA_ID (0)


//Globals
FlxCard flxCard;


/*****************/
void display_help()
/*****************/
{
  printf("Usage: %s [OPTIONS]\n", APPLICATION_NAME);
  printf("Initializes DMA transfers and the DMA memory on the screen in 1s intervals.\n\n");
  printf("Options:\n");
  printf("  -d NUMBER      Use card indicated by NUMBER. Default: 0.\n");
  printf("  -D level       Configure debug output at API level. 0=disabled, 5, 10, 20 progressively more verbose output. Default: 0.\n");
  printf("  -h             Display help.\n");
  printf("  -V             Display the version number\n");
}


/********************************/
void dump_buffer(u_long virt_addr)
/********************************/
{
  u_char *buf = (u_char *)virt_addr;
  int i;


  for(i = 0; i < BUFSIZE; i++)
  {
    if(i % 32 == 0)
      printf("\n0x  ");
    printf("%02x ", *buf++);
  }
  printf("\n");
}


/*****************************/
int main(int argc, char **argv)
/*****************************/
{
  int i, loop, max_tlp, ret, device_number = 0, opt, handle, debuglevel;
  u_long baraddr0, vaddr, paddr, board_id, bsize, opt_emu_ena_to_host, opt_emu_ena_to_frontend, opt_dnlnk_fo, opt_uplnk_fo;
  flxcard_bar0_regs_t *bar0;

  while((opt = getopt(argc, argv, "hd:D:V")) != -1)
  {
    switch (opt)
    {
      case 'd':
	device_number = atoi(optarg);
	break;

      case 'h':
	display_help();
	exit(0);

      case 'D':
	debuglevel = atoi(optarg);
        DF::GlobalDebugSettings::setup(debuglevel, DFDB_FELIXCARD);
	break;

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

    // save current state
    flxCard.cfg_get_option(BF_GBT_EMU_ENA_TOHOST, &opt_emu_ena_to_host);
    flxCard.cfg_get_option(BF_GBT_EMU_ENA_TOFRONTEND, &opt_emu_ena_to_frontend);
    flxCard.cfg_get_option(BF_GBT_DNLNK_FO_SEL, &opt_dnlnk_fo);
    flxCard.cfg_get_option(BF_GBT_UPLNK_FO_SEL, &opt_uplnk_fo);

    for(loop = 0; loop < 8; loop++)
      flxCard.dma_stop(loop);

    flxCard.dma_reset();
    flxCard.dma_soft_reset();
    flxCard.dma_fifo_flush();
    flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOFRONTEND, 0);
    flxCard.cfg_set_option(BF_GBT_EMU_ENA_TOHOST, 1);

    flxCard.cfg_get_option(REG_BOARD_ID_SVN, &board_id);
    printf("Board ID: %lx\n", board_id);
    printf("About to do CMEM_Open()\n");
    ret = CMEM_Open();
    printf("Back from CMEM_Open(). ret is %i\n",ret);
    bsize = BUFSIZE;
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

    printf("Allocated Memory Segment\n  Phys. Addr: 0x%016lx\n  Virt. Addr: 0x%016lx\n", paddr, vaddr);

    max_tlp = flxCard.dma_max_tlp_bytes();

    printf("\nBuffer before DMA write:\n");
    dump_buffer(vaddr);

    flxCard.dma_program_write(DMA_ID, paddr, BUFSIZE, max_tlp, 0);
    printf("\nBuffer after DMA write:\n");
    dump_buffer(vaddr);

    baraddr0 = flxCard.openBackDoor(0);
    bar0 = (flxcard_bar0_regs_t *)baraddr0;

    printf("Start Ptr:   0x%016lx\n", bar0->DMA_DESC[0].start_address);
    printf("End Ptr:     0x%016lx\n", bar0->DMA_DESC[0].end_address);
    printf("Enable:      0x%0x\n", bar0->DMA_DESC_ENABLE);
    printf("Read Ptr:    0x%016lx\n", bar0->DMA_DESC[0].read_ptr);
    printf("Write Ptr:   0x%016lx\n", bar0->DMA_DESC_STATUS[0].current_address);
    printf("Descriptor done DMA0: 0x%lx\n", bar0->DMA_DESC_STATUS[0].descriptor_done);
    printf("Even Addr. DMA  DMA0: 0x%lx\n", bar0->DMA_DESC_STATUS[0].even_addr_dma);
    printf("Even Addr. PC   DMA0: 0x%lx\n", bar0->DMA_DESC_STATUS[0].even_addr_pc);
    printf("Descriptor done DMA1: 0x%lx\n", bar0->DMA_DESC_STATUS[1].descriptor_done);
    printf("Even Addr. DMA  DMA1: 0x%lx\n", bar0->DMA_DESC_STATUS[1].even_addr_dma);
    printf("Even Addr. PC   DMA1: 0x%lx\n", bar0->DMA_DESC_STATUS[1].even_addr_pc);

    printf("Start Addr: %016lx\nEnd Addr:  %016lx\nRead Ptr: %016lx\n", bar0->DMA_DESC[0].start_address, bar0->DMA_DESC[0].end_address, bar0->DMA_DESC[0].read_ptr);

    //    printf("\nBuffer after DMA write:\n");
    // dump_buffer(vaddr);

    for(i = 0; i<5 ; i++)
    {
      printf("\n--------------------\n  %d:\n", i);
      flxCard.dma_advance_read_ptr(DMA_ID, paddr, BUFSIZE, 512); //512
      flxCard.dma_wait(DMA_ID);

      printf("Read Ptr:    0x%016lx\n", bar0->DMA_DESC[0].read_ptr);
      printf("Write Ptr:   0x%016lx\n", bar0->DMA_DESC_STATUS[0].current_address);
      printf("Descriptor done DMA0: 0x%lx\n", bar0->DMA_DESC_STATUS[0].descriptor_done);
      printf("Even Addr. DMA  DMA0: 0x%lx\n", bar0->DMA_DESC_STATUS[0].even_addr_dma);
      printf("Even Addr. PC   DMA0: 0x%lx\n", bar0->DMA_DESC_STATUS[0].even_addr_pc);

      dump_buffer(vaddr);
      sleep(1);
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
