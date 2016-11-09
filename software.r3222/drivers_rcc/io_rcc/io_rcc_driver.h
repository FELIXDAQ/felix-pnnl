/************************************************************************/
/*									*/
/*  This is the driver header file for the IO_RCC package		*/
/*									*/
/*   6. Jun. 02  MAJO  created						*/
/*									*/
/*******C 2012 - The software with that certain something****************/

#ifndef _IO_RCC_DRIVER_H
#define _IO_RCC_DRIVER_H

#include "io_rcc/io_rcc_common.h"   

#define IO_MAX_PCI         100     //Max. number of PCI devices linked at any time
#define MAX_PROC_TEXT_SIZE 0x10000 //The output of "more /proc/io_rcc" must not generate more characters than that

#define CMOSA              0x70
#define CMOSD              0x71
#define BID1               0x35
#define BID2               0x36


/********/
/*Macros*/
/********/
#ifdef DRIVER_DEBUG
  #define kdebug(x) {if (debug) printk x;}
#else
  #define kdebug(x)
#endif

#ifdef DRIVER_ERROR
  #define kerror(x) {if (errorlog) printk x;}
#else
  #define kerror(x)
#endif


/*********/
/* Types */
/*********/
typedef struct
{
  struct pci_dev *dev_ptr;
  u_int vid;
  u_int did;
  u_int occ;
  u_int pid;
} pci_devices_t;

typedef struct
{
  u_int linked[IO_MAX_PCI];
} private_stuff; 

struct io_proc_data_t
{
  char name[10];
  char value[100];
};


/************/
/*Prototypes*/
/************/
static void io_rcc_vmaClose(struct vm_area_struct *vma);
static void io_rcc_vmaOpen(struct vm_area_struct *vma);
static long io_rcc_ioctl(struct file *file, u_int cmd, u_long arg);
static int io_rcc_open(struct inode *ino, struct file *filep);
static int io_rcc_mmap(struct file *file, struct vm_area_struct *vma);
static int io_rcc_release(struct inode *ino, struct file *filep);
static ssize_t io_rcc_write_procmem(struct file *file, const char *buffer, u_long count, void *data);
static ssize_t io_rcc_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data);


/*************/
/*ioctl codes*/
/*************/
#define IO_RCC_MAGIC 'x'

#define IOPEEK      _IOR(IO_RCC_MAGIC, 37, IO_RCC_IO_t)
#define IOPOKE      _IOW(IO_RCC_MAGIC, 38, IO_RCC_IO_t)
#define IOPCILINK   _IOW(IO_RCC_MAGIC, 39, IO_PCI_FIND_t)
#define IOPCIUNLINK _IOW(IO_RCC_MAGIC, 40, int)
#define IOPCICONFR  _IOR(IO_RCC_MAGIC, 41, IO_PCI_CONF_t)
#define IOPCICONFW  _IOW(IO_RCC_MAGIC, 42, IO_PCI_CONF_t)
#define IOPCIINFO   _IOR(IO_RCC_MAGIC, 43, pci_info_t)

#endif
