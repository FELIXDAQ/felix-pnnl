/*******************************************************************/
/*                                                                 */
/* This is the C++ source code of the FlxCard object               */
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
#include <signal.h>

#include <stdexcept>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <linux/types.h>

#include "DFDebug/DFDebug.h"
#include "FlxCard.h"
#include "FlxException.h"

int FlxCard::m_cards_open = 0;

/****************/
FlxCard::FlxCard()
/****************/
{
  m_fd                   = 0;
  m_is_open              = false;
  m_timeout              = 0;
  m_slotNumber           = -1;
  m_Bar_0_Base           = 0;
  m_Bar_1_Base           = 0;
  m_Bar_2_Base           = 0;
  m_card_model           = 0;
  verboseFlag            = false;
}


i2c_device_t i2c_devices_FLX_709[] =
  {
    {"USR_CLK",         "SI570",                    0x5d, "1:0"},
    {"FMC_ADN",         "ADN2814",                  0x40, "2:0"},
    {"FMC_SI5338",      "SI5338",                   0x70, "2:0"},
    {"FMC_TEMP_SENSOR", "TC74 (on CRORC TEST FMC)", 0x4A, "2:0"},
    {"ID_EEPROM",       "M24C08-WDW6TP",            0x54, "8:0"},
    {"SFP1-1",          "AFBR-709SMZ (Conven Mem)", 0x50, "16:1"},
    {"SFP1-2",          "AFBR-709SMZ (Enhan Mem)",  0x51, "16:1"},
    {"SFP2-1",          "AFBR-709SMZ (Conven Mem)", 0x50, "16:2"},
    {"SFP2-2",          "AFBR-709SMZ (Enhan Mem)",  0x51, "16:2"},
    {"SFP3-1",          "AFBR-709SMZ (Conven Mem)", 0x50, "16:4"},
    {"SFP3-2",          "AFBR-709SMZ (Enhan Mem)",  0x51, "16:4"},
    {"SFP4-1",          "AFBR-709SMZ (Conven Mem)", 0x50, "16:8"},
    {"SFP4-2",          "AFBR-709SMZ (Enhan Mem)",  0x51, "16:8"},
    {"DDR3-1",          "SRAM-MT8KTF51264HZ",       0x51, "64:0"},
    {"DDR3-2",          "SRAM-MT8KTF51264HZ",       0x52, "64:0"},
    {"REC_CLOCK",       "SI5324",                   0x68, "128:0"},
    {NULL,              NULL,                       0,    0}
  };


i2c_device_t i2c_devices_FLX_710[] =
  {
    {"CLOCK_RAM",       "ICS8N4Q001L IDT",          0x6e, "1:0"},
    {"CLOCK_SYS",       "ICS8N4Q001L IDT",          0x6e, "2:0"},
    {"CLOCK_CXP1",      "IDT 8N3Q001",              0x6e, "4:0"},
    {"CLOCK_CXP2",      "IDT 8N3Q001",              0x6e, "8:0"},
    {"FMC_ADN",         "ADN2814 (on TTCfx FMC)",   0x40, "16:0"},    
    {"FMC_SI5338",      "SI5338",                   0x70, "16:0"},
    {"FMC_TEMP_SENSOR", "TC74 (on CRORC TEST FMC)", 0x4A, "16:0"},
    {"CXP1_TX",         "AFBR-83PDZ",               0x50, "32:0"},
    {"CXP1_RX",         "AFBR-83PDZ",               0x54, "32:0"},
    {"CXP2_TX",         "AFBR-83PDZ",               0x50, "64:0"},
    {"CXP2_RX",         "AFBR-83PDZ",               0x54, "64:0"},
    {"DDR3-1",          "SRAM-MT16JTF25664HZ",      0x50, "128:0"},
    {"DDR3-2",          "SRAM-MT16JTF25664HZ",      0x51, "128:0"},
    {NULL,              NULL,                       0,    0}
  };
 
 
//MJ: Note: The I2C tree of the FLX711 is not yet defined  
i2c_device_t i2c_devices_FLX_711[] =
  {
    {NULL,              NULL,                       0,    0}
  };


/****************************/
void FlxCard::card_open(int n)
/****************************/
{
  card_params_t card_data;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::card_open: Method called.");

  //Install a signal handler for the implementation of watchdog timers.
  //Note: The signal handler must only be installed once.

  if (m_cards_open == 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::card_open: Installing signal handler");
    struct sigaction sa;

    // Install timer_handler as the signal handler for SIGVTALRM
    memset(&sa, 0, sizeof (sa));
    sa.sa_handler = (void (*)(int))&FlxCard::watchdogtimer_handler;
    sigaction (SIGVTALRM, &sa, NULL);
  }
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::card_open: Signal handler already installed");
    m_cards_open++;
  }

  int fd = open("/dev/flx", O_RDWR);
  if (fd < 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::card_open: Failed to open /dev/flx.");
    THROW_FLX_EXCEPTION(NOTOPENED, "Failed to open /dev/flx.")
  }
  card_data.slot = n;

  int iores = ioctl(fd, SETCARD, &card_data);
  if(iores < 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::card_open: Error from ioctl(SETCARD).");
    THROW_FLX_EXCEPTION(IOCTL, "Error from ioctl(SETCARD)")
  }

  m_fd = fd;
  m_Bar_0_Base = map_memory_bar(card_data.baseAddressBAR0, 4096);
  m_Bar_1_Base = map_memory_bar(card_data.baseAddressBAR1, 4096);
  m_Bar_2_Base = map_memory_bar(card_data.baseAddressBAR2, 65536);

  m_bar0 = (flxcard_bar0_regs_t *)m_Bar_0_Base;
  m_bar1 = (flxcard_bar1_regs_t *)m_Bar_1_Base;
//  m_bar2 = (flxcard_bar2_regs_t *)m_Bar_2_Base;

  m_card_model = card_model();
}


/****************************/
void FlxCard::card_close(void)
/****************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::card_close: Method called.");

  //Get rid of the signal handler when we close the last instance
  if (m_cards_open == 1)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::card_close: Removing signal handler");
    sigaction(SIGVTALRM, NULL, NULL);
  }
  else
  {
    m_cards_open--;
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::card_close: m_cards_open = " << m_cards_open);
  }

  if (m_fd == 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::card_close: m_fd is zero");
    THROW_FLX_EXCEPTION(NOTOPENED, "The link to the driver is already closed (or has never been opened)")
  }

  unmap_memory_bar(m_Bar_0_Base, 4096);
  unmap_memory_bar(m_Bar_1_Base, 4096);
  unmap_memory_bar(m_Bar_2_Base, 65536);

  close(m_fd);
}


/**********************************/
int FlxCard::dma_max_tlp_bytes(void)
/**********************************/
{
  int tlp_bits;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_max_tlp_bytes: Method called.");

  int iores = ioctl(m_fd, GET_TLP, &tlp_bits);
  if(iores < 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::dma_max_tlp_bytes: Error from ioctl(GET_TLP).");
    THROW_FLX_EXCEPTION(IOCTL, "Error from ioctl(GET_TLP)")
  }

  return (128 << tlp_bits);  //MJ: replace 128 by a constant??
}


/**********************************/
void FlxCard::dma_stop(u_int dma_id)
/**********************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_stop: Method called.");
  m_bar0->DMA_DESC_ENABLE &= ~(1 << dma_id);
}


/********************************************************************************************/
void FlxCard::dma_program_write(u_int dma_id, u_long dst, size_t size, u_int tlp, u_int flags)
/********************************************************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_program_write: Method called.");
  dma_stop(dma_id);

  if (dst == 0 || size == 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::dma_program_write: dst or size is zero.");
    THROW_FLX_EXCEPTION(PARAM, "dst or size is zero")
  }

  m_bar0->DMA_DESC[dma_id].start_address = dst;
  m_bar0->DMA_DESC[dma_id].end_address   = dst + size;
  m_bar0->DMA_DESC[dma_id].tlp           = tlp / 4;
  m_bar0->DMA_DESC[dma_id].read          = 0;
  m_bar0->DMA_DESC[dma_id].wrap_around   = (flags & FLX_DMA_WRAPAROUND) ? 1 : 0;
  m_bar0->DMA_DESC[dma_id].read_ptr      = dst + size;

  m_bar0->DMA_DESC_ENABLE |= 1 << dma_id;

}


/*******************************************************************************************/
void FlxCard::dma_program_read(u_int dma_id, u_long dst, size_t size, u_int tlp, u_int flags)
/*******************************************************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_program_read: Method called.");
  dma_stop(dma_id);

  if (dst == 0 || size == 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::dma_program_read: dst or size is zero.");
    THROW_FLX_EXCEPTION(PARAM, "dst or size is zero")
  }

  m_bar0->DMA_DESC[dma_id].start_address = dst;
  m_bar0->DMA_DESC[dma_id].end_address   = dst + size;
  m_bar0->DMA_DESC[dma_id].tlp           = tlp / 4;
  m_bar0->DMA_DESC[dma_id].read          = 1;
  m_bar0->DMA_DESC[dma_id].wrap_around   = (flags & FLX_DMA_WRAPAROUND) ? 1 : 0;
  m_bar0->DMA_DESC[dma_id].read_ptr      = dst;

  m_bar0->DMA_DESC_ENABLE |= 1 << dma_id;
}


/**********************************/
void FlxCard::dma_wait(u_int dma_id)
/**********************************/
{
  //Set up watchdog
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 1000000;    // One second
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;       // Only one shot
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
  m_timeout = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_wait: Method called for dma_id = " << dma_id);

  while(m_bar0->DMA_DESC_ENABLE & (1 << dma_id))
    //    while(0)
  {

    if (m_timeout)
    {
      THROW_FLX_EXCEPTION(I2C, "Timeout")
    }

  } 

  //Stop watchdog
  timer.it_value.tv_usec = 0;    // Stop timer
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
}


/*************************************************************************************/
void FlxCard::dma_advance_read_ptr(u_int dma_id, u_long dst, size_t size, size_t bytes)
/*************************************************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_advance_read_ptr: Method called.");

  m_bar0->DMA_DESC[dma_id].read_ptr += bytes;

  if(m_bar0->DMA_DESC[dma_id].read_ptr >= dst + size)
    m_bar0->DMA_DESC[dma_id].read_ptr -= size;
}


/********************************/
void FlxCard::dma_fifo_flush(void)
/********************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_fifo_flush: Method called.");
  m_bar0->DMA_FIFO_FLUSH = 1;
}


/***************************/
void FlxCard::dma_reset(void)
/***************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_reset: Method called.");
  m_bar0->DMA_RESET = 1;
}


/********************************/
void FlxCard::dma_soft_reset(void)
/********************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::dma_soft_reset: Method called.");
  m_bar0->SOFT_RESET = 1;
}


/*********************************/
void FlxCard::registers_reset(void)
/*********************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::registers_reset: Method called.");
  m_bar0->REGISTERS_RESET = 1;
}


/**********************************************************/
void FlxCard::i2c_write_byte(u_char slave_addr, u_char byte)
/**********************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_write_byte: Method called.");

  i2c_wait_not_full();

  u_long value = 0;
  value |= ((u_long)slave_addr << 1);
  value |= ((u_long)byte << 8);
  i2c_wr_set(value);
}


/*******************************************************************************/
void FlxCard::i2c_write_byte_to_addr(u_char slave_addr, u_char addr, u_char byte)
/*******************************************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_write_byte_to_addr: Method called.");

  i2c_wait_not_full();

  u_long value = 0;
  value |= ((u_long)slave_addr << 1);
  value |= ((u_long)addr << 8);
  value |= ((u_long)byte << 16);
  value |= (1 << 24);
  i2c_wr_set(value);
}


/***********************************************************/
u_char FlxCard::i2c_read_byte(u_char slave_addr, u_char addr)
/***********************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_read_byte: Method called.");

  i2c_wait_not_full();

  u_long value = 1;
  value |= ((u_long)slave_addr << 1);
  value |= ((u_long)addr << 8);
  value |= (1 << 24);
  i2c_wr_set(value);

  i2c_wait_not_empty();
  i2c_rd_set(1);      // pop
  usleep(I2C_DELAY);
  u_long res = i2c_rd_get(); // read
  usleep(I2C_SLEEP);
  if(res & I2C_EMPTY_FLAG)
    return (res & 0xff);
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_read_byte: i2c_read_byte has failed because I2C_EMPTY_FLAG was not set");
    THROW_FLX_EXCEPTION(I2C, "i2c_read_byte has failed because I2C_EMPTY_FLAG was not set")
  }
  return 0;
}


/*****************************/
void FlxCard::eeprom_read(void)    //MJ: what does that function actually do?
/*****************************/
{
  int cont = 0;   //MJ: int or char?
  char device_name[20] = "\0";
  u_char rel_reg = 0u, character = 1;           //Not initialized to zero

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::eeprom_read: Method called.");

  eeprom_selection(device_name);

  printf("Read: ");     //MJ: is this only for debugging??
  for (cont = rel_reg; (cont < 1024) && (character != 0); cont++)
  {
    i2c_devices_read(device_name, cont, &character);
    if(character == 0)
    {
      printf("\n");
    }
    else
    {
      printf("%c", character);
    }
  }
}


/******************************************************/
void FlxCard::eeprom_write(u_long reg, const char *data)
/******************************************************/
{
  u_long cont1 = 0, cont2 = 0;  //MJ: Is cont1 a char?
  char device_name[20] = "\0", character = 0;
  int data_length = 0;
  u_char eeprom_data;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::eeprom_write: Method called.");

  data_length = strlen(data);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::eeprom_write: data_length = " << data_length);

  eeprom_selection(device_name);

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::eeprom_write: Writing...");

  cont2 = 0;
  for (cont1 = reg; ((cont1 < (u_long)data_length) && (cont1 < 1023)); cont1++)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::eeprom_write: cont1 = " << cont1);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::eeprom_write: device = " << device_name);

    /*
    mj: should this method be able to write one data word or a complete array?
    eeprom_data = (u_char)strtoul(dat, &p_aux, 0);
    if(*p_aux != '\0')
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_write: Invalid data.");
      THROW_FLX_EXCEPTION(I2C, "Invalid data")
    }
    i2c_devices_write(device_name, cont1, &data[cont2]);
*/

    DEBUG_TEXT(DFDB_FELIXCARD, 20,  "FlxCard::eeprom_write: data[cont2] = " << data[cont2]);

    cont2++;
  }

  DEBUG_TEXT(DFDB_FELIXCARD, 20,  "FlxCard::eeprom_write: Closing block....");
  character = 0;


  //MJ: to be ported to the new API: i2c_devices_write(device_name, cont1, &character);
}


/*******************************************************************************/
void FlxCard::i2c_devices_write(const char *device, u_char reg_addr, u_char data)
/*******************************************************************************/
{
  int found = 0;
  u_char dec_port1 = 0, dec_port2 = 0, bin_port1 = 0, bin_port2 = 0, address = 0;
  char str[6], *p_aux = NULL;
  i2c_device_t *devices;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_devices_write: method called with device = " << device << " and reg_addr = " << (u_int)reg_addr);

  int digics = check_digic_value(device, &dec_port1, &dec_port2, &address);
  char *upper = strdup(device);
  str_upper(upper);

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_write: dec_port1 = " << (u_int)dec_port1);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_write: dec_port2 = " << (u_int)dec_port2);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_write: address   = " << (u_int)address);

  if(m_card_model == FLX_711)
  {
    devices = i2c_devices_FLX_711;
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_write: I2C support for the FLX711 is not yet available");
    THROW_FLX_EXCEPTION(I2C, "I2C support for the FLX711 is not yet available")
  }
  if(m_card_model == FLX_710)
    devices = i2c_devices_FLX_710;
  if(m_card_model == FLX_709)
    devices = i2c_devices_FLX_709;

  for(; devices->name != NULL && found == 0; devices++)
  {
    if(strcmp(upper, devices->name) == 0)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_write: Device found!");

      sscanf(devices->port, "%s", str);
      check_port_value(str, &bin_port1, &bin_port2);
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_write: Read " << HEX(bin_port1) << " and " << HEX(bin_port2));

      //Writing value.
      i2c_write(bin_port1, bin_port2, devices->address, reg_addr, data);
      found = 1;
    }
  }

  if(digics == 0)
  {
    bin_port1 = (1 << dec_port1);
    if(dec_port2 != 0)
      bin_port2 = (1 << dec_port2);
    else
      bin_port2 = 0;

    //Writing value.
    i2c_write(bin_port1, bin_port2, address, reg_addr, data);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_write: Writing " << data << " to register " << (u_int)reg_addr << " on port 1 " << (u_int)dec_port1 << " and port 2 " << (u_int)dec_port2 << " from device " << (u_int)address);
    found = 1;
  }

  free(upper);

  if(!found)
  {
    if(digics == I2C_DEVICE_ERROR_INVALID_PORT)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_write: Invalid port.");
      THROW_FLX_EXCEPTION(I2C, "Invalid port")
    }
    if (digics == I2C_DEVICE_ERROR_INVALID_ADDRESS)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_write: Invalid address.");
      THROW_FLX_EXCEPTION(I2C, "Invalid address")
    }

    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_write: Device does not exist.");
    THROW_FLX_EXCEPTION(I2C, "Device does not exist")
  }
}


/********************************************************************************/
void FlxCard::i2c_devices_read(const char *device, u_char reg_addr, u_char *value)
/********************************************************************************/
{
  i2c_device_t *devices;
  int result = 0, found = 0;
  char str[6];
  u_char dec_port1 = 0, dec_port2 = 0, bin_port1 = 0, bin_port2 = 0, address = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_devices_read: method called with device = " << device << " and reg_addr = " << (u_int)reg_addr);

  char digics = check_digic_value(device, &dec_port1, &dec_port2, &address);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: dec_port1 = " << (u_int)dec_port1);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: dec_port2 = " << (u_int)dec_port2);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: address = " << (u_int)address);
  char *upper = strdup(device);
  str_upper(upper);

  if(m_card_model == FLX_711)
  {
    devices = i2c_devices_FLX_711;
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_read: I2C support for the FLX711 is not yet available");
    THROW_FLX_EXCEPTION(I2C, "I2C support for the FLX711 is not yet available")
  }

  if(m_card_model == FLX_710)
    devices = i2c_devices_FLX_710;

  if(m_card_model == FLX_709)
    devices = i2c_devices_FLX_709;

  for(; devices->name != NULL && found == 0; devices++)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: devices->name        = " << devices->name);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: devices->description = " << devices->description);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: devices->address     = " << (u_int)devices->address);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: devices->port        = " << devices->port);
    if(strcmp(upper, devices->name) == 0)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: Device found!");

      sscanf(devices->port, "%s", str);
      check_port_value(str, &bin_port1, &bin_port2);

      //Reading value.
      *value = i2c_read(bin_port1, bin_port2, devices->address, reg_addr);
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_device_read: Resgister " << (u_int)reg_addr << " from device " << (u_int)devices->address << " = " << (u_int)*value);
      found = 1;
    }
  }

  if(digics == 0)
  {
      bin_port1 = (1 << dec_port1);
      if(dec_port2 != 0)
        bin_port2 = ( 1 << dec_port2);
      else
        bin_port2 = 0;

      //Reading value.
      *value = i2c_read(bin_port1, bin_port2, address, reg_addr);
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_devices_read: Register " << (u_int)reg_addr << " on port " << (u_int)dec_port1 << " and " << (u_int)dec_port2 << " from device " << (u_int)address << " = " << (u_int)*value);
      found = 1;
  }

  free(upper);

  if(!found)
  {
    result = I2C_DEVICE_ERROR_NOT_EXIST;

    if(digics == I2C_DEVICE_ERROR_INVALID_PORT)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_read: Invalid port.");
      THROW_FLX_EXCEPTION(I2C, "Invalid address")
      result = I2C_DEVICE_ERROR_INVALID_PORT;
    }
    if (digics == I2C_DEVICE_ERROR_INVALID_ADDRESS)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_read: Invalid address.");
      THROW_FLX_EXCEPTION(I2C, "Invalid address")
      result = I2C_DEVICE_ERROR_INVALID_ADDRESS;
    }

    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::i2c_devices_read: Device does not exist.");
    THROW_FLX_EXCEPTION(I2C, "Device does not exist")
  }
}


/***********************************************************************************************************************************/
u_int FlxCard::gbt_setup(int soft_alignment, int alignment, int channel_mode, int descrambler_enable, u_long svn_version, long delay)
/***********************************************************************************************************************************/
{
  int topbot_mode = 0;
  u_int wrong_channels = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_setup: method called.");
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_setup: ---General initialization");

  gbt_initialization(svn_version, delay);

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_setup: ---Configuring GBT TX....");
  gbt_tx_configuration(channel_mode, alignment);

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_setup: ---Configuring GBT RX....");
  wrong_channels = gbt_rx_configuration(channel_mode, soft_alignment, descrambler_enable, topbot_mode);

  return wrong_channels;
}


/*********************************************************************/
long int FlxCard::gbt_version_delay(u_long svn_version, char *filename)
/*********************************************************************/
{
  FILE *file_delay;
  int control = 0;
  u_long delay = 0, f_version = 0, f_delay = 0;
  char data[1024]="\0", *fileused, file_default[] = "../flx_propagation_delay.conf";

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_version_delay: method called.");

  if(filename == NULL)
    fileused = file_default;
  else
    fileused = filename;

  if((file_delay = fopen(fileused , "r")) != NULL)
  {
    while(control == 0 && fgets(data, 1024, file_delay) != NULL)
    {
      if(check_digic_value2(data, &f_version, &f_delay) == 0)
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


/*******************************************************/
void FlxCard::cfg_get_reg(const char *key, u_long *value)
/*******************************************************/
{
  int found = 0;
  char *upper = strdup(key);
  regmap_register_t *reg;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::cfg_get_reg: method called for register " << key);
  str_upper(upper);

  for(reg = regmap_registers; reg->name != NULL; reg++)
  {
    if(strcmp(upper, reg->name) == 0)
    {
      found = 1;

      if(!(reg->flags & REGMAP_REG_READ))
      {
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_reg: Register " << key << " not readable!");
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_reg: reg->flags   = 0x" << HEX(reg->flags));
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_reg: reg->address = 0x" << HEX(reg->address));
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_reg: reg->name    = " << reg->name);
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_reg: REGMAP_REG_READ = 0x" << HEX(REGMAP_REG_READ));
        THROW_FLX_EXCEPTION(REG_ACCESS, "Register not readable!")
      }

      u_long *v = (u_long *)(m_Bar_2_Base + reg->address);
      *value = *v;
      break;
    }
  }

  free(upper);

  if(!found)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_reg: Register " << key << " does not exist!");
    THROW_FLX_EXCEPTION(REG_ACCESS, "Register does not exist!")
  }
}


/**********************************************************/
void FlxCard::cfg_get_option(const char *key, u_long *value)
/**********************************************************/
{
  int found = 0;
  char *upper = strdup(key);
  regmap_bitfield_t *bf;
  u_long regvalue;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::cfg_get_option: method called.");
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: key = " << key);
  str_upper(upper);

  for(bf = regmap_bitfields; bf->name != NULL; bf++)
  {
    if(strcmp(upper, bf->name) == 0)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: bitfield found");
      if(!(bf->flags & REGMAP_REG_READ))
      {
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_option: Bitfield " << key << " not readable!");
        THROW_FLX_EXCEPTION(REG_ACCESS, "Bitfield not readable!")
      }

      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: m_Bar_2_Base = 0x" << HEX(m_Bar_2_Base));
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: bf->address  = 0x" << HEX(bf->address));

      u_long *v = (u_long *)(m_Bar_2_Base + bf->address);
      regvalue = *v;
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: regvalue(1)  = 0x" << HEX(regvalue));
      regvalue = (regvalue & bf->mask) >> bf->shift;
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: bf->shift    = 0x" << HEX(bf->shift));
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: bf->mask     = 0x" << HEX(bf->mask));
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: regvalue(2)  = 0x" << HEX(regvalue));

      *value = regvalue;
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: *value       = 0x" << HEX(*value));

      found = 1;
      break;
    }
  }

  free(upper);

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::cfg_get_option: upper freed");
  if(!found)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_get_option: Bitfield " << key << " does not exist!");
    THROW_FLX_EXCEPTION(REG_ACCESS, "Bitfield does not exist!")
  }
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::cfg_get_option: end of method");
}


/******************************************************/
void FlxCard::cfg_set_reg(const char *key, u_long value)
/******************************************************/
{
  int found = 0;
  char *upper = strdup(key);
  regmap_register_t *reg;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::cfg_set_reg: method called for bit field " << key << " and value " << value);
  str_upper(upper);

  for(reg = regmap_registers; reg->name != NULL; reg++)
  {
    if(strcmp(upper, reg->name) == 0)
    {
      found = 1;

      if(!(reg->flags & REGMAP_REG_WRITE))
      {
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_set_reg: Register " << key << " not writeable!");
        THROW_FLX_EXCEPTION(REG_ACCESS, "Register not writeable!")
      }

      u_long *v = (u_long *)(m_Bar_2_Base + reg->address);
      *v = value;
      break;
    }
  }

  free(upper);
  if(!found)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_set_reg: Register " << key << " does not exist!");
    THROW_FLX_EXCEPTION(REG_ACCESS, "Register does not exist!")
  }
}


/*********************************************************/
void FlxCard::cfg_set_option(const char *key, u_long value)
/*********************************************************/
{
  regmap_bitfield_t *bf;
  int found = 0;
  char *upper = strdup(key);
  u_long regvalue;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::cfg_set_option: method called for bit field " << key << " and value " << value);
  str_upper(upper);

  for(bf = regmap_bitfields; bf->name != NULL; bf++)
  {
    if(strcmp(upper, bf->name) == 0)
    {
      if(!(bf->flags & REGMAP_REG_WRITE))
      {
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_set_option: Bitfield " << key << " not writeable!");
        THROW_FLX_EXCEPTION(REG_ACCESS, "Bitfield not writeable!")
      }

      u_long *v = (u_long *)(m_Bar_2_Base + bf->address);

      regvalue = *v;
      regvalue &=~ bf->mask;
      regvalue |= (value << bf->shift) & bf->mask;

      *v = regvalue;
      found = 1;
      break;
    }
  }

  free(upper);
  if(!found)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::cfg_set_option: Bitfield " << key << " does not exist!");
    THROW_FLX_EXCEPTION(REG_ACCESS, "Bitfield does not exist!")
  }
}


/**************************/
void FlxCard::irq_init(void)
/**************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::irq_init: Method called).");

  int iores = ioctl(m_fd, INIT_IRQ);
  if(iores < 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::irq_init: Error from ioctl(INIT_IRQ).");
    THROW_FLX_EXCEPTION(IOCTL, "Error from ioctl(INIT_IRQ)")
  }
}


/****************************************/
void FlxCard::irq_enable(u_long interrupt)
/****************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::irq_enable: Method called).");
  m_bar1->INT_TAB_ENABLE |= (1 << interrupt);
}


/********************************/
void FlxCard::irq_enable_all(void)
/********************************/
{
  u_int i;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::irq_enable_all: Method called).");
  for(i = 0; i < NUM_INTERRUPTS; i++)
    irq_enable(i);
}


/*****************************************/
void FlxCard::irq_disable(u_long interrupt)
/*****************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::irq_disable: Method called).");
  m_bar1->INT_TAB_ENABLE &= ~(1 << interrupt);

}


/*********************************/
void FlxCard::irq_disable_all(void)
/*********************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::irq_disable_all: Method called).");

  u_int i;
  for(i = 0; i < NUM_INTERRUPTS; i++)
    irq_disable(i);
}


/***************************/
void FlxCard::irq_wait(int n)
/***************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::irq_wait: Method called).");

  int iores = ioctl(m_fd, WAIT_IRQ, &n);
  if(iores < 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::irq_wait: Error from ioctl(WAIT_IRQ).");
    THROW_FLX_EXCEPTION(IOCTL, "Error from ioctl(WAIT_IRQ)")
  }
}


/****************************/
void FlxCard::irq_cancel(void)
/****************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::irq_cancel: Method called).");

  int iores = ioctl(m_fd, CANCEL_IRQ_WAIT);
  if(iores < 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::irq_cancel: Error from ioctl(CANCEL_IRQ_WAIT).");
    THROW_FLX_EXCEPTION(IOCTL, "Error from ioctl(CANCEL_IRQ_WAIT)")
  }
}


/***************************/
int FlxCard::card_model(void)
/***************************/
{
  u_long card_id = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::card_model: Method called).");

  cfg_get_option(BF_CARD_TYPE, &card_id);

  printf("card_mode: card_id is %lu \n.", card_id);
  card_id = FLX_710;
  printf("card_mode: card_id is now hard-set to %lu \n.", card_id);
  if(card_id != FLX_709 && card_id != FLX_710 && card_id != FLX_711)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::card_model: Cannot itentify card");
    THROW_FLX_EXCEPTION(HW, "Cannot itentify card")
  }

  return (int)card_id;
}


/*******************************************************/
void FlxCard::spi_write_to_addr(u_short addr, u_int byte)
/*******************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_write_to_addr: Method called");
  i2c_wait_not_full();

  u_long value = 0;
  if(addr == 0xFF)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::spi_read_from_addr: You DON'T want to lock the EEPROM");
    THROW_FLX_EXCEPTION(SPI, "You DON'T want to lock the EEPROM")
  }
  else
  {
    value |= ((u_long)addr);
    value |= ((u_long)byte << 4);
    spi_wr_set(value);
  }
}


/*********************************************/
u_int FlxCard::spi_read_from_addr(u_short addr)
/*********************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_read_from_addr: Method called");

  i2c_wait_not_full();
  u_long value = 0;

  value |= (0xE);
  value |= (addr << 4);
  spi_wr_set(value);
  i2c_wait_not_empty();
  spi_rd_set(1);      // pop

  u_long res = spi_rd_get(); // read

  if(res & SPI_EMPTY_FLAG)
    return (res & 0xffffffff);
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::spi_read_from_addr: Empty flag not set in result");
    THROW_FLX_EXCEPTION(SPI, "Empty flag not set in result")
  }
}


/**********************************/
u_int FlxCard::spi_read(u_short reg)
/**********************************/
{
  u_int value = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_read: Method called");

  value = spi_read_from_addr(reg);

  DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::spi_read: Set device and register " << reg);
  return value;
}


/**********************************************/
void FlxCard::spi_write(u_short reg, u_int data)
/**********************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_write: Method called");
  spi_write_to_addr(reg, data);

  DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::spi_write:  Set device register" << reg << " with data " << data);
}


/************************************************/
/* Ssssssservice functions (not part of the user API) */
/************************************************/


/**********************************************************/
u_long FlxCard::map_memory_bar(u_long pci_addr, size_t size)
/**********************************************************/
{
  void *vaddr;
  u_long offset;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::map_memory_bar: method called.");
  offset = pci_addr & 0xfff;  /* mmap seems to need a 4K alignment */
  pci_addr &= 0xfffffffffffff000ll;

  vaddr = mmap(0, size, (PROT_READ|PROT_WRITE), MAP_SHARED, m_fd, pci_addr);
  if (vaddr == MAP_FAILED)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::map_memory_bar: Error from mmap for pci_addr = " << pci_addr << " and size = " << size);
    THROW_FLX_EXCEPTION(MAPERROR, "Error from mmap")
  }

  return (u_long)vaddr + offset;
}


/*******************************************************/
void FlxCard::unmap_memory_bar(u_long vaddr, size_t size)
/*******************************************************/
{
  int ret;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::unmap_memory_bar: method called.");

  ret = munmap((void *)vaddr, size);
  if (ret)
  {
    DEBUG_TEXT(DFDB_VMERCC, 5, "unmap_memory_bar: Error from munmap, errno = 0x" << HEX(errno));
    THROW_FLX_EXCEPTION(MAPERROR, "Error from munmap")
  }
}


/*************************/
int FlxCard::spi_full(void)
/*************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_full: Method called.");

  u_long status = spi_wr_get();
  return (status & SPI_FULL_FLAG);
}


/**************************/
int FlxCard::spi_empty(void)
/**************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_empty: Method called");
  u_long status = spi_rd_get();
  return (status & SPI_EMPTY_FLAG);
}


/******************************************/
void FlxCard::eeprom_selection(char *eeprom)
/******************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::eeprom_selection: Method called.");

  if(m_card_model == FLX_709)
  {
    //Conditions depending on the card_id and abs_address.
    strcpy(eeprom, "ID_EEPROM");
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::eeprom_selection: Using VC-709");
  }
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::eeprom_selection: This type of card has not got an EEPROM");
    THROW_FLX_EXCEPTION(PARAM, "This type of card has not got an EEPROM")
  }
}


/**************************/
int FlxCard::i2c_empty(void)
/**************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_empty: Method called.");

  u_long status = i2c_rd_get();

  return (status & I2C_EMPTY_FLAG);
}


/*************************/
int FlxCard::i2c_full(void)
/*************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_full: Method called.");

  u_long status = i2c_wr_get();

  return (status & I2C_FULL_FLAG);
}


/***********************************/
u_long FlxCard::openBackDoor(int bar)
/***********************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::openBackDoor: Method called.");

  if(bar == 0)
    return m_Bar_0_Base;
  else if(bar == 1)
    return m_Bar_1_Base;
  else if(bar == 2)
    return m_Bar_2_Base;
  else
    THROW_FLX_EXCEPTION(PARAM, "Parameter bar is out of range")
}


/******************************/
u_long FlxCard::i2c_wr_get(void)
/******************************/
{
  u_long aux_value;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_wr_get: method called.");
  cfg_get_reg(REG_I2C_WR, &aux_value);
  return (aux_value);
}


/******************************/
u_long FlxCard::i2c_rd_get(void)
/******************************/
{
  u_long aux_value;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_rd_get: method called.");
  cfg_get_reg(REG_I2C_RD, &aux_value);
  return (aux_value);
}


/************************************/
void FlxCard::i2c_wr_set(u_long value)
/************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_wr_set: method called.");

  cfg_set_reg(REG_I2C_WR, value);
  usleep(I2C_DELAY);
}


/************************************/
void FlxCard::i2c_rd_set(u_long value)
/************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_wrd_set: method called.");

  cfg_set_reg(REG_I2C_RD, value);
  usleep(I2C_DELAY);
}


/***********************************/
void FlxCard::i2c_wait_not_full(void)
/***********************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_wait_not_full: method called.");

  //Set up watchdog
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 1000000;    // One second
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;       // Only one shot
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
  m_timeout = 0;

  while (i2c_full())
  {
    usleep(I2C_SLEEP);
    if (m_timeout)
    {
      THROW_FLX_EXCEPTION(I2C, "Timeout")
    }
  }

  //Stop watchdog
  timer.it_value.tv_usec = 0;    // Stop timer
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
}


/************************************/
void FlxCard::i2c_wait_not_empty(void)
/************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_wait_not_empty: method called.");

  //Set up watchdog
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 1000000;    // One second
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;       // Only one shot
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
  m_timeout = 0;

  while (i2c_empty())
  {
    usleep(I2C_SLEEP);
    if (m_timeout)
    {
      THROW_FLX_EXCEPTION(I2C, "Timeout")
    }
  }

  //Stop watchdog
  timer.it_value.tv_usec = 0;    // Stop timer
  setitimer(ITIMER_VIRTUAL, &timer, NULL);
}


/********************************************************************************************/
int FlxCard::check_digic_value(const char* str, u_char* port1, u_char* port2, u_char* address)
/********************************************************************************************/
{
  char *p_aux = NULL;
  char* pos = strchr(const_cast<char*>(str), ':');
  unsigned long int convres;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::check_digic_value: method called with string = " << str);

  if(pos == NULL)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_digic_value: Failed to find a : character.");
    return I2C_DEVICE_ERROR_NOT_EXIST;
  }

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: rest string = " << pos);
  ///*pos = '\0';

  char *pos2 = strchr((pos + 1), ':');

  if(pos2 == NULL)
  {
    convres = 999;
    convres = strtoul(str, &p_aux, 0);

    if(convres == 999)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_digic_value: Failed to decode the first port number");
      return I2C_DEVICE_ERROR_INVALID_PORT;
    }
    *port1 = (u_char)convres;
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: port1 = " << (u_int)*port1);

    convres = 999;
    convres = strtoul(pos + 1, &p_aux, 0);
    if(convres == 999)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_digic_value: Failed to decode the address");
      return I2C_DEVICE_ERROR_INVALID_ADDRESS;
    }
    *address = (u_char)convres;
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: address = " << (u_int)*address);
  }
  else
  {
    convres = 999;
    convres = strtoul(str, &p_aux, 0);
    if(convres == 999)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_digic_value: Failed to decode the first port number (2)");
      return I2C_DEVICE_ERROR_INVALID_PORT;
    }
    *port1 = (u_char)convres;
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: port1 = " << (u_int)*port1);

    convres = 999;
    convres = strtoul(pos + 1, &p_aux, 0);
    if(convres == 999)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_digic_value: Failed to decode the second port number");
      return I2C_DEVICE_ERROR_INVALID_PORT;
    }
    *port2 = (u_char)convres;
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: port2 = " << (u_int)*port2);

    convres = 999;
    convres = strtoul(pos2 + 1, &p_aux, 0);
    if(convres == 999)
    if(*p_aux != '\0')
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_digic_value: Failed to decode the address");
      return I2C_DEVICE_ERROR_INVALID_ADDRESS;
    }
    *address = (u_char)convres;
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: address = " << (u_int)*address);
  }

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: *port1 = " << *port1);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: *port2 = " << *port2);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_digic_value: *address = " << *address);

  return 0;
}



/********************************/
void FlxCard::str_upper(char *str)
/********************************/
{
  //DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::str_upper: method called.");
  do
  {
    *str = toupper((u_char) *str);
  } while (*str++);
  //DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::str_upper: method done.");
}


/***************************************************************************/
void FlxCard::check_port_value(const char *str, u_char *port1, u_char *port2)
/***************************************************************************/
{
  char *p_aux = NULL, *pos = strchr(const_cast<char*>(str), ':');

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::check_port_value: method called.");

  if(pos == NULL)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_port_value: Device does not exist.");
    THROW_FLX_EXCEPTION(I2C, "Device does not exist")
  }

  *pos = '\0';

  *port1 = (u_char)strtoul(str, &p_aux, 0);
  if(*p_aux != '\0')
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_port_value: Invalid port.");
    THROW_FLX_EXCEPTION(I2C, "Invalid port")
  }

  *port2 = (u_char)strtoul(pos + 1, &p_aux, 0);
  if(*p_aux != '\0')
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::check_port_value: Invalid address");
    THROW_FLX_EXCEPTION(I2C, "Invalid address")
  }
}


/*****************************************************************************************/
void FlxCard::i2c_write(u_char port1, u_char port2, u_char device, u_char reg, u_char data)
/*****************************************************************************************/
{
  int cont1 = 0, cont2 = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_write: method called.");

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_write: Port 1 = " << port1);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_write: Port 1 = " << port2);

  if(m_card_model == FLX_711)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_write: FLX711 is not supported");
    THROW_FLX_EXCEPTION(I2C, "FLX711 is not supported")
  }
  if(m_card_model == FLX_710)
    i2c_write_byte(I2C_ADDR_SWITCH1_FLX_710, port1);    //Configure the switch.
  if(m_card_model == FLX_709)
  {
    i2c_write_byte(I2C_ADDR_SWITCH1_FLX_709, port1);    //Configure the switch.
    i2c_write_byte(I2C_ADDR_SWITCH2_FLX_709, port2);    //configure switch2
  }

  i2c_write_byte_to_addr(device, reg, data);

  for(cont1 = 0; port1 > 1; cont1++)
    port1 = port1 / 2;
  for(cont2 = 0; port2 > 1; cont2++)
    port2 = port2 / 2;

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::check_port_value: Set port1 " << cont1 << " and port2 " << cont2 << " on swith to device " << device << " and register " << reg << " with data " << data);
}


/*****************************************************************************/
u_char FlxCard::i2c_read(u_char port1, u_char port2, u_char device, u_char reg)
/*****************************************************************************/
{
  u_char value = 0;
  int cont1 = 0, cont2 = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_read: method called with port1 = " << (u_int)port1 << ", port2 = " << (u_int)port2 << ", device = " << (u_int)device << " and reg = " << (u_int)reg);

  if(m_card_model == FLX_711)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::i2c_write: FLX711 is not supported");
    THROW_FLX_EXCEPTION(I2C, "FLX711 is not supported")
  }
  if(m_card_model == FLX_710)
    i2c_write_byte(I2C_ADDR_SWITCH1_FLX_710, port1);    //Configure the switch.
  if(m_card_model == FLX_709)
  {
    i2c_write_byte(I2C_ADDR_SWITCH1_FLX_709, port1);    //Configure the switch.
    i2c_write_byte(I2C_ADDR_SWITCH2_FLX_709, port2);    //configure switch2
  }

  value = i2c_read_byte(device, reg);

  for(cont1 = 0; port1 > 1; cont1++)
    port1 = port1 / 2;
  for(cont2 = 0; port2 > 1; cont2++)
    port2 = port2 / 2;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_read: Set port1 " << cont1 << " and port2 " << cont2 << " on swith to device " << (u_int)device << " and register " << (u_int)reg);
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::i2c_read: Value read = " << (u_int)value);
  return value;
}



/**************************************************************/
void FlxCard::gbt_initialization(u_long svn_version, long delay)
/**************************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_initialization: method called.");

  /*1.1- Set (TX & RX)USR ready for CH [0-23]. Write REG_9 & REG_10*/
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: *****Set TxUsrRdy & RxUsrRdy. Registers 0x5490 & 0x54A0.....");

  cfg_set_option(BF_GBT_TXUSRRDY_B2312, ALL_BITS);
  cfg_set_option(BF_GBT_TXUSRRDY_B1100, ALL_BITS);
  cfg_set_option(BF_GBT_RXUSRRDY_B2312, ALL_BITS);
  cfg_set_option(BF_GBT_RXUSRRDY_B1100, ALL_BITS);
	
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: Done!");

  /*1.2- GTH Quads Soft Reset. Write REG_11, 3bits CH[0-11], 3bits CH[12-23] --> 0x70007000 */
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: *****Soft reset of the Quads. Register 0x54B0.....");

  cfg_set_option(BF_GBT_GTTX_RESET_B0503, ALL_BITS);
  cfg_set_option(BF_GBT_GTTX_RESET_B0200, ALL_BITS);
  usleep(WAIT_TIME);
  //Write REG_A ---> 0
  cfg_set_option(BF_GBT_GTTX_RESET_B0503, 0x00000000);
  cfg_set_option(BF_GBT_GTTX_RESET_B0200, 0x00000000);

  sleep(2);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: Done!");

  if(delay < 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::gbt_initialization: File or FW version not found. Delay won't be set");
    THROW_FLX_EXCEPTION(GBT, "File or FW version not found. Delay won't be set")
  }
  else
  {
    if(delay == 0)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: Calculating delay values from default file for version " << svn_version);

      if((delay = gbt_version_delay(svn_version, NULL)) == FLX_GBT_FILE_NOT_FOUND)
      {
        DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::gbt_initialization: Default file ../flx_propagation_delay.conf not found");
        //MJ: does not seem to be an error THROW_FLX_EXCEPTION(GBT, "Default file ../flx_propagation_delay.conf not found")
      }
      else
      {
	if(delay == FLX_GBT_VERSION_NOT_FOUND)
	{
	  DEBUG_TEXT(DFDB_FELIXCARD, 5, "FlxCard::gbt_initialization: Version not found in database. Delay values not set!");
          THROW_FLX_EXCEPTION(GBT, "Version not found in database. Delay values not set!")
	}
	else
	{
	  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: Delay values = " << delay);
	  cfg_set_option(BF_GBT_TX_TC_DLY_VALUE1, delay);
	  cfg_set_option(BF_GBT_TX_TC_DLY_VALUE2, delay);
        }
      }
    }
    else
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: Writing delay values.....");
      cfg_set_option(BF_GBT_TX_TC_DLY_VALUE1, delay);
      cfg_set_option(BF_GBT_TX_TC_DLY_VALUE2, delay);
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_initialization: Done!");
    }
  }
}



/********************************************************************/
void FlxCard::gbt_tx_configuration(int channel_tx_mode, int alignment)
/********************************************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_tx_configuration: method called.");

  ///////////////*2- GBT TX configuration*////////////////////////////

  /*2.1- Quads Soft TX reset. Write (REG_14), 3bits CH[0-11], 3bits CH[12-23] --> 0x70007000 */

  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: *****Quads TX soft reset. Register 0x54E0.....");

  cfg_set_option(BF_GBT_SOFT_TX_RESET_B0503, ALL_BITS);
  cfg_set_option(BF_GBT_SOFT_TX_RESET_B0200, ALL_BITS);
  usleep(WAIT_TIME);
  //Write REG_14 ---> 0
  cfg_set_option(BF_GBT_SOFT_TX_RESET_B0503, 0x00000000);
  cfg_set_option(BF_GBT_SOFT_TX_RESET_B0200, 0x00000000);
  sleep(2);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: Done!");

  /*2.2- Set TX mode CH [0-23]. Write REG_22, 2 bits/CH. FEC=00 ; Wide-Bus=01 */
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: *****Configuring TX mode. Register 0x5560.....");

  if(channel_tx_mode != FLX_GBT_TMODE_FEC)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: FEC-->0x55555500555555");
    cfg_set_option(BF_GBT_DATA_TXFORMAT_B4724, 0x555555);
    cfg_set_option(BF_GBT_DATA_TXFORMAT_B2300, 0x555555);
  }
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: WideBus-->0x0");
    cfg_set_option(BF_GBT_DATA_TXFORMAT_B4724, 0x00000000000000);
    cfg_set_option(BF_GBT_DATA_TXFORMAT_B2300, 0x00000000000000);
  }

  /*2.3- GBT TX latency Optimization configuration. Write REG_20*/
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: *****Set TX latency optimization configuration. Register 0x5540.....");
  cfg_set_option(BF_GBT_TX_OPT, 0x000000000000);	//Set Tx Domain Crossing Latency Configuration.
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: Done!");

  /*2.4 TX time domain crossing selection. Write REG_26, CH[0-23], 1 bit/CH. Align One time=0x0 ; Continuous aling=0x1*/
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: *****Set TX time domain crossing mode. Register 0x55A0.....");
  if(alignment == FLX_GBT_ALIGNMENT_CONTINUOUS)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: CONTINUOUS-->0x0FFF0FFF");
    cfg_set_option(BF_GBT_TX_TC_METHOD_B2312, ALL_BITS);
    cfg_set_option(BF_GBT_TX_TC_METHOD_B1100, ALL_BITS);
  }
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: ONE-->0x0");
    cfg_set_option(BF_GBT_DATA_TXFORMAT_B4724, 0x00000000000000);
    cfg_set_option(BF_GBT_DATA_TXFORMAT_B2300, 0x00000000000000);
  }
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: *****GBT TX reset. Register 0x5580.....");

  /*2.5- GBT TX reset. Write REG_24, CH [0-23] (1 bit/CH)---> 1 */
  cfg_set_option(BF_GBT_TX_RESET_B2312, ALL_BITS);
  cfg_set_option(BF_GBT_TX_RESET_B1100, ALL_BITS);
  usleep(WAIT_TIME);
  //Write REG_24 ----> 0
  cfg_set_option(BF_GBT_TX_RESET_B2312, 0x00000000);
  cfg_set_option(BF_GBT_TX_RESET_B1100, 0x00000000);
  sleep(1);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_tx_configuration: Done!");
}


/******************************************************************************************************/
int FlxCard::gbt_rx_configuration(int channel_rx_mode, int soft_alignment, u_int demux_mode, int topbot)
/******************************************************************************************************/
{
  u_int error = 0, wrong_channels = 0, cont = 0;
  u_long number_channels, alingment_status = 0, fec_status = 0, aux_value = 0, outsel = 0, topbot_database = 0x00060F42;	//Virtual value.  //Value saved from 0x2510 on software alignment or 0x2760 when FSM is used.

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_rx_configuration: method called.");
  /*3-GBT RX configuration*/

  /*3.1- RX latency Optimization configuration. Write REG_21, 0x0*/
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Set RX latency optimization configuration. Register 0x5550.....");
  cfg_get_option(BF_GBT_RX_OPT, &aux_value);
  aux_value = aux_value & 0xFFFFFFFF000000;						//Bits 47-24 are reserved.
	
  cfg_set_option(BF_GBT_RX_OPT, aux_value);									      //Latency Optimization are 0s.
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Done!");

  /*3.2 GTH RX reset. Write REG_12, CH [0-23] ---> 0x0FFF0FFF */
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****GTH RX reset. Register 0x54C0.....");
  cfg_set_option(BF_GBT_GTRX_RESET_B2312, ALL_BITS);
  cfg_set_option(BF_GBT_GTRX_RESET_B1100, ALL_BITS);
  usleep(WAIT_TIME);
  cfg_set_option(BF_GBT_GTRX_RESET_B2312, 0x0000000);
  cfg_set_option(BF_GBT_GTRX_RESET_B1100, 0x0000000);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Done!");

  /*3.3 Set GBT RX encoding mode. Write REG_23, 2 bits/CH CH[0-23]. FEC=00 ; Wide-Bus=01 */
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Configuring RX mode. Register 0x5570.....");
  if(channel_rx_mode != FLX_GBT_TMODE_FEC)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: FEC-->0x55555500555555");
    cfg_set_option(BF_GBT_DATA_RXFORMAT_B4724, 0x555555);
    cfg_set_option(BF_GBT_DATA_RXFORMAT_B2300, 0x555555);
  }
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: WideBus-->0x0");
    cfg_set_option(BF_GBT_DATA_RXFORMAT_B4724, 0x0000000000000000);
    cfg_set_option(BF_GBT_DATA_RXFORMAT_B2300, 0x0000000000000000);
  }

  /*3.4 RX alignment. If REG_2 bit 2==0 --> Skip, else bit 2==1 --> software RX alignment*/
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Channel alignment......");
  if(soft_alignment == 0)
  {
    if(topbot == 0)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: FSM selected");
      sleep(1);
    }
    else
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Database value used");
      cfg_set_reg(REG_GBT_TOPBOT, topbot_database); /* CHECK REGISTER */
      sleep(1);
    }
  }
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Using software for alignment");
    cfg_set_option(BF_GBT_MODE_CTRL_RX_ALIGN_SW, soft_alignment);
    cfg_set_option(BF_GBT_MODE_CTRL_DESMUX_USE_SW, demux_mode);

    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Detecting number of channels.....");
    cfg_get_option(BF_NUM_OF_CHANNELS, &number_channels);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: " << number_channels << " channels detected");

    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Starting alignment......");
    wrong_channels = gbt_software_alignment(number_channels);
    if(wrong_channels != 0)
    {		//Software alignment
      error = 1;
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Not all channels align, something must be wrong");
      //MJ: does not seem to be an error THROW_FLX_EXCEPTION(GBT, "Not all channels align, something must be wrong")
    }
  }

  /*3.5 GBT RX reset. Write REG_25---> 0x0FFF0FFF; Write REG_25---> 1*/
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****GBT TX reset. Register 0x5590.....");
  cfg_set_option(BF_GBT_RX_RESET_B2312, ALL_BITS);
  cfg_set_option(BF_GBT_RX_RESET_B1100, ALL_BITS);
  usleep(WAIT_TIME);
  //Write REG_25 ----> 0
  cfg_set_option(BF_GBT_RX_RESET_B2312, 0x0000000);
  cfg_set_option(BF_GBT_RX_RESET_B1100, 0x0000000);
  sleep(1);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Done!");

  /*3.6 Multiplexer descrambler output.*/
  if(demux_mode)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Using recomended multiplexer descrambler output. Register 0x5740.....");
    //If REG_2 bit 0==0, Skip; if 1: Read REG_20, REG_OUTSEL_CALC. CH[0-23].
    cfg_get_option(BF_GBT_TX_OPT, &outsel);
    cfg_set_reg(REG_GBT_OUTMUX_SEL, outsel);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Written on register 0x55B0");
  }

  /*3.7 Write REG_1 bit 0 --->1; Write REG_1 bit 0 --->0*/
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Checking alignment status. Register 0x5730.....");
  usleep(WAIT_TIME);
  cfg_set_option(BF_GBT_GENERAL_CTRL, 0x1);
  usleep(WAIT_TIME);
  cfg_set_option(BF_GBT_GENERAL_CTRL, 0x0);
  usleep(WAIT_TIME);

  //Read REG_19 REG_ALIGNMENT_DONE CH[0-23].  If 0, phase alignment?
  cfg_get_reg(REG_GBT_ALIGNMENT_DONE, &alingment_status); /* CHECK REGISTER */
  if(wrong_channels == 0)
  {
    for (cont = 0; cont < number_channels; cont++)
    {
      if((alingment_status&(1ul << cont)) == 0)
      {
	wrong_channels++;
	DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Channel " << cont << " not aligned");
      }
    }
  }
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Final check:");
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Alingment_status = " << alingment_status);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Checking header locked. Register 0x5720.....");
  cfg_get_reg(REG_GBT_RX_IS_HEADER, &alingment_status);
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: Header_locked = " << alingment_status);

  if(channel_rx_mode == FLX_GBT_TMODE_FEC)
  {
    /*Check REG_13 if FEC is enable.*/
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: *****Checking FEC errors. Register 0x5750.....");
    cfg_get_reg(REG_GBT_ERROR, &fec_status);
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_rx_configuration: FEC error status =" << fec_status);
  }
  return wrong_channels;
}


/******************************************************/
int FlxCard::gbt_software_alignment(int number_channels)
/******************************************************/
{
  int error = 0, channel = 0;
  u_int wrong_channels = 0;
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_software_alignment: method called.");

  for(channel = 0; channel < number_channels; channel++)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_software_alignment: ------Trying to align channel " << channel << " ......");
    error = gbt_channel_alignment(channel);
    if (error != 0)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_software_alignment: Not align!");
      error = 0;
      wrong_channels++;
    }
    else
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_software_alignment: Channel align!");
    }
  }
  return wrong_channels;
}


/***********************************************/
int FlxCard::gbt_channel_alignment(u_int channel)
/***********************************************/
{
  u_long phase_found = 0, phase_table[10];
  u_int oddeven_flag = 0, ch = 0, phase = 0, topfind_flag = 0;
  int topbotfinal_flag = 0, error = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_channel_alignment: method called.");
  //4 Software RX alignment. (Using 240M clock) METHOD PER CHANNEL (x24)/

  //Adjusting channel number to bit number.
  if(channel < 12)
    ch = channel;
  else
   ch = channel + 4;

  //SET TOPBOT & ODDEVEN bits-->0
  gbt_topbot_oddeven_set(0, 0, ch);
  //Check the phase
  cfg_get_reg(REG_GBT_ALIGNMENT_DONE, &phase_found);
  phase_found = phase_found&(u_long)(1ull << ch);

  //Trying alignment with topbot=0.
  topfind_flag = gbt_topbot_0_alignment(ch, &phase_found, &oddeven_flag);

  //No phase with topbot=0 ---> topbot=1.
  if(topfind_flag == 0)
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_channel_alignment: ****Not good phase for TOPBOT=0, checking TOPBOT=1.....");
    gbt_topbot_oddeven_set(1,0,ch);
    //Check the phase
    cfg_get_reg(REG_GBT_ALIGNMENT_DONE, &phase_found);
    phase_found = phase_found&(u_long)(1ull << ch);

    //TOPBOT 1 alignment.
    if(gbt_topbot_1_alignment(ch, &phase_found, &oddeven_flag)==(-1))
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_channel_alignment: ****Not phase found");
      error =- 1;
    }
    else
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_channel_alignment: ****Final values: TOPBOT = 1   ODDEVEN = " << oddeven_flag);
    }
  }
  else
  {
    DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_channel_alignment: ****Good phase for TOPBOT=0, checking TOPBOT=1.....");
    //Topbot=0 has a phase, check topbot=1.
    for(phase = 0; phase < 10; phase++)
    {
      cfg_get_reg(REG_GBT_CLK_SAMPLED, &phase_table[phase]);
      phase_table[phase] = phase_table[phase]&(u_long)(1ull << ch);
      //Switching the phase.
      gbt_shift_phase(ch);
    }

    if(phase_table[1] > 0)
    {
      if(phase_table[0] == 0)
        topbotfinal_flag = 1;
      else
        topbotfinal_flag = 0;
    }
    else
    {
      if(phase_table[5] == 0)
        topbotfinal_flag = 0;
      else
        topbotfinal_flag = 1;
    }

/*
    if(verbose>1)
    {
      sequence_found = 0;
      printf("[ ");
      sequence_control = (phase_table[0] >> ch);
      for(phase = 0; phase < 10; phase++)
      {
	printf("%d ", (phase_table[phase] >> ch));
	if((phase_table[phase]>>ch) != sequence_control)
	{
	  sequence_found++;
	  sequence_control = (phase_table[phase] >> ch);
	}
      }
      if((phase_table[0] >> ch) != sequence_control)
        sequence_found++;
      if(sequence_found != 2)
      {
        printf("]-->Bad sequence!\n");
      else
        printf("]-->Ok\n");
      }
    }
*/
    if(topbotfinal_flag == 1)
    {
      //TOPBOT=1 recomended
      //Set CH TOPBOT=1
      gbt_topbot_oddeven_set(1, oddeven_flag, ch);
      //Switching 5 phases
      for(phase = 0; phase < 5; phase++)
      {
        phase_found = gbt_shift_phase(ch);
      }
    }
  }

  for(phase = 0; phase < 10; phase++)
  {
    gbt_shift_phase(ch);
    cfg_get_reg(REG_GBT_CLK_SAMPLED, &phase_table[phase]);
    phase_table[phase] = phase_table[phase]&(u_long)(1ull << ch);
  }

/*
  if(verbose > 1)
  {
    sequence_found = 0;
    printf("[ ");
    sequence_control = (phase_table[0] >> ch);
    for(phase = 0; phase < 10; phase++)
    {
      printf("%d ", (phase_table[phase] >> ch));
      if((phase_table[phase]>>ch) != sequence_control)
      {
        sequence_found++;
        sequence_control = (phase_table[phase] >> ch);
      }
    }
    if((phase_table[0] >> ch) != sequence_control)
      sequence_found++;
    if(sequence_found != 2)
      printf("]-->Bad sequence!\n");
    else
      printf("]-->Ok\n");
    }
  }

  if(verbose)
  {
    if(topbotfinal_flag == 1)
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_channel_alignment: ****Final values: TOPBOT=1   ODDEVEN= " << oddeven_flag);
    }
    else
    {
      DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::gbt_channel_alignment: "****Final values: TOPBOT=0   ODDEVEN= " << oddeven_flag);
    }
    cfg_get_option("GBT_ALIGNMENT_DONE", &phase_found);
    printf("***Alignment final status: 0x%04x \n", phase_found);
  }
*/
  return error;
}


/*************************************************************************/
void FlxCard::gbt_topbot_oddeven_set(u_int topbot, u_int oddeven, u_int ch)
/*************************************************************************/
{
  static u_long top_bot = 0, odd_even = 0; 	//Initially all the register are 0s.

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_topbot_oddeven_set: method called.");

  if(oddeven == 0)
  {
    //Set CH ODDEVEN=0
    odd_even = odd_even & (~(u_long)(1ull << ch));
    cfg_set_reg(REG_GBT_ODD_EVEN, odd_even); /* CHECK REGISTER */
  }
  else
  {
    //Set ODDEVEN=1.
    odd_even = odd_even | (u_long)(1ull << ch);
    cfg_set_reg(REG_GBT_ODD_EVEN, odd_even);
  }

  if(topbot == 0)
  {
    //Set CH TOPBOT=0
    top_bot = top_bot & (~(u_long)(1ull << ch));
    cfg_set_reg(REG_GBT_TOPBOT, top_bot);
  }
  else
  {
    //Set CH TOPBOT=1
    top_bot = top_bot | (u_long)(1ull << ch);
    cfg_set_reg(REG_GBT_TOPBOT, top_bot);
  }

  //Alingment Status reset
  cfg_set_option(BF_GBT_GENERAL_CTRL, 1);
  usleep(MICRO_WAIT_TIME);
  cfg_set_option(BF_GBT_GENERAL_CTRL, 0);
  usleep(WAIT_TIME);
}


/*************************************************************************************/
int FlxCard::gbt_topbot_1_alignment(u_int ch, u_long *phase_found, u_int *oddeven_flag)
/*************************************************************************************/
{
  int topbotfinal_flag = 0, phase_cnt = 10;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_topbot_1_alignment: method called.");
  *oddeven_flag = 0;

  while(phase_cnt != 0)
  {
    if(*phase_found == 0)
    {
      if(phase_cnt > 1)
      {
	phase_cnt--;
	//Switching the phase.
	*phase_found = gbt_shift_phase(ch);
      }
      else
      {
	if(*oddeven_flag == 0)
	{
	  //TOPBOT & ODDEVEN--->1
	  gbt_topbot_oddeven_set(1, 1, ch);
	  phase_cnt = 10;
	  *oddeven_flag = 1;
	  //Switching to first phase.
	  *phase_found= gbt_shift_phase(ch);
	}
	else
	{
	  phase_cnt = 0;
	  gbt_topbot_oddeven_set(0, 0, ch);		//Leave Top and EVEN ==0
	  topbotfinal_flag =- 1;
	}
      }
    }
    else
    {
      phase_cnt = 0;
      topbotfinal_flag = 1;
    }
  }
  return topbotfinal_flag;
}


/*************************************************************************************/
int FlxCard::gbt_topbot_0_alignment(u_int ch, u_long *phase_found, u_int *oddeven_flag)
/*************************************************************************************/
{
  int topfind_flag = 0, phase_cnt = 10;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_topbot_0_alignment: method called.");
  *oddeven_flag = 0;
  //Trying alignment with topbot=0.

  while(phase_cnt != 0)
  {
    if(*phase_found == 0)
    {
      if(phase_cnt > 1)
      {
	phase_cnt--;
	//Switching the phase.
	*phase_found = gbt_shift_phase(ch);
      }
      else
      {
	if(oddeven_flag == 0)
	{
	  //If no phase and oddeven=0 --> oddeven=1
	  gbt_topbot_oddeven_set(0, 1, ch);
	  phase_cnt = 10;
	  *oddeven_flag = 1;
	  //Switching to first phase & check.
	  *phase_found = gbt_shift_phase(ch);
	}
	else
	{
	  //No phase in topbot=1.
	  phase_cnt = 0;	//Leaving the while.
	  topfind_flag = 0;
	}
      }
    }
    else
    {
      //Good phase found
      phase_cnt = 0;	//Leaving the while.
      topfind_flag = 1;
    }
  }
  return topfind_flag;
}


/***************************************/
u_long FlxCard::gbt_shift_phase(u_int ch)
/***************************************/
{
  u_long phase_found = 0;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::gbt_shift_phase: method called.");

  //Switching the phase.
  cfg_set_reg(REG_GBT_RXSLIDE, (u_long)(1ull << ch));
  usleep(MICRO_WAIT_TIME);
  cfg_set_reg(REG_GBT_RXSLIDE, 0);
  usleep(MICRO_WAIT_TIME);
  cfg_set_reg(REG_GBT_RXSLIDE, (u_long)(1ull << ch));
  usleep(MICRO_WAIT_TIME);
  cfg_set_reg(REG_GBT_RXSLIDE, 0);
  usleep(MICRO_WAIT_TIME);

  //Alingment Status reset
  cfg_set_option(BF_GBT_GENERAL_CTRL, 1);
  usleep(MICRO_WAIT_TIME);
  cfg_set_option(BF_GBT_GENERAL_CTRL, 0);
  usleep(WAIT_TIME);

  //Check
  cfg_get_reg(REG_GBT_ALIGNMENT_DONE, &phase_found);
  phase_found = phase_found&(u_long)(1ull << ch);

  return phase_found;
}


/******************************************************************************/
int FlxCard::check_digic_value2(const char *str, u_long *version, u_long *delay)  //MJ: There is also a method with the name check_digic_value. Are they doing the same thing? -> they handle strings of different format
/******************************************************************************/
{
  char *p_aux = NULL, *pos = strchr(const_cast<char*>(str), ':');

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::check_digic_value2: method called.");

  if(pos == NULL)
    return -1;

  *pos = '\0';

  *version = strtoul(str, &p_aux, 0);
  if(*p_aux != '\0')
    return -2;

  *delay = strtoul(pos + 1, &p_aux, 0);
  if(*delay == 0)
    return -3;

  return 0;
}


/******************************/
u_long FlxCard::spi_wr_get(void)
/******************************/
{
  u_long aux_value;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_wr_get: method called.");
  cfg_get_reg(REG_SPI_WR, &aux_value);
  return (aux_value);
}


/******************************/
u_long FlxCard::spi_rd_get(void)
/******************************/
{
  u_long aux_value;

  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_rd_get: method called.");
  cfg_get_reg(REG_SPI_RD, &aux_value);
  return (aux_value);
}


/************************************/
void FlxCard::spi_wr_set(u_long value)
/************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_wr_set: method called.");
  cfg_set_reg(REG_SPI_WR, value);
  usleep(SPI_DELAY);
}


/************************************/
void FlxCard::spi_rd_set(u_long value)
/************************************/
{
  DEBUG_TEXT(DFDB_FELIXCARD, 15, "FlxCard::spi_rd_set: method called.");
  cfg_set_reg(REG_SPI_RD, value);
  usleep(SPI_DELAY);
}


/*************************************************/
void FlxCard::watchdogtimer_handler(int /*signum*/)
/*************************************************/
{
  m_timeout = 1;
  DEBUG_TEXT(DFDB_FELIXCARD, 20, "FlxCard::watchdogtimer_handler: Timer expired");
}


spi_device_t spi_devices[] =
{
  {
    "CLOCK_CDCE",
    "CDCE62005",
    0x0
  },
  { NULL, NULL, 0 }
};