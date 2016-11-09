/************************************************************************/
/*									*/
/*  This is the common header file for the IO_RCC 			*/
/*  library & applications						*/
/*									*/
/*   6. Jun. 02  MAJO  created						*/
/*									*/
/*******C 2002 - The software with that certain something****************/

#ifndef _IO_RCC_COMMON_H
#define _IO_RCC_COMMON_H

#ifdef __KERNEL__
  #include <linux/types.h>
  #define P_ID_IO_RCC 8   // Needs to be re-defined here since we do not want to include rcc_error.h at this level
#else
  #include <sys/types.h>
#endif

typedef u_int IO_ErrorCode_t;

//Board types
#define VP_UNKNOWN 0
#define VP_PSE     1
#define VP_PMC     2
#define VP_100     3
#define VP_CP1     4

//Stolen from /usr/src/linuxcern/includelinux/ioport.h
#define IORESOURCE_IO           0x00000100
#define IORESOURCE_MEM          0x00000200

typedef struct
{
  u_int offset;
  u_int data;
  u_int size;
} IO_RCC_IO_t;

typedef struct
{
  u_int board_manufacturer;
  u_int board_type;
  u_int operating_system_type;
  u_int operating_system_version;
} HostInfo_t;

typedef struct
{
  u_int vid;
  u_int did;
  u_int occ;
  u_int handle;
} IO_PCI_FIND_t;

typedef struct
{
  u_int handle;
  u_int offs;
  u_int func;
  u_int data;
  u_int size;
} IO_PCI_CONF_t;

typedef struct
{
  u_int base;
  u_int size;
  u_int flags;
} pci_bar;

typedef struct
{
  u_int handle;
  pci_bar bar[6];
} pci_info_t;


// Error codes
enum
{
  IO_RCC_SUCCESS = 0,
  IO_RCC_ERROR_FAIL = (P_ID_IO_RCC <<8)+1,
  IO_RCC_FILE,
  IO_RCC_NOTOPEN,
  IO_RCC_MMAP,
  IO_RCC_MUNMAP,
  IO_RCC_ILLMANUF,
  IO_RCC_IOFAIL,
  IO_PCI_TABLEFULL,
  IO_PCI_NOT_FOUND,
  IO_PCI_ILL_HANDLE,
  IO_PCI_UNKNOWN_BOARD,
  IO_PCI_CONFIGRW,
  IO_PCI_REMAP,
  IO_RCC_ILL_OFFSET,
  IO_RCC_NO_CODE
};

#endif
