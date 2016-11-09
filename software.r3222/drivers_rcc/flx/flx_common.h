#ifndef _FLX_COMMON_H
#define _FLX_COMMON_H

#ifdef __KERNEL__
  #include <linux/types.h>
#else
  #include <sys/types.h>
#endif


/* ioctl "switch" flags */
#define FLX_MAGIC 'y'

#define GETCARDS            _IOW(FLX_MAGIC, 1, int)
#define SETCARD             _IOR(FLX_MAGIC, 2, card_params_t*)
#define GET_TLP             _IOW(FLX_MAGIC, 3, int)
#define WAIT_IRQ            _IOW(FLX_MAGIC, 4, int)
#define FAKE_IRQ            _IOWR(FLX_MAGIC, 6, int)
#define CANCEL_IRQ_WAIT     _IO(FLX_MAGIC, 8)
#define INIT_IRQ            _IO(FLX_MAGIC, 9)
#define RESET_IRQ_COUNTERS  _IO(FLX_MAGIC, 10)
#define RESET_IRQ_FLAGS     _IO(FLX_MAGIC, 11)
#define MASK_IRQ            _IOW(FLX_MAGIC, 12, int)
#define UNMASK_IRQ          _IOW(FLX_MAGIC, 13, int)

typedef struct
{
  u_int handle;
  u_int offs;
  u_int func;
  u_int data;
  u_int size;
} IO_PCI_CONF_t;

#define MAXCARDS				4   // Max. number of FLX cards

typedef struct
{
    struct pci_dev *pciDevice;
    unsigned int slot;
    unsigned int baseAddressBAR0;
    unsigned int sizeBAR0;
    unsigned int baseAddressBAR1;
    unsigned int sizeBAR1;
    unsigned int baseAddressBAR2;
    unsigned int sizeBAR2;
    unsigned int baseAddressBAR3;
    unsigned int sizeBAR3;

    /* void* writeIndexBuffer;
    uint64_t writeIndexHandle;
    int nDmaPages; //number of pre-allocated DMA buffers - can be modified when running insmod - defaults to MAXDMABUF
    int dmaPageSize;
    void* primaryDmaBuffer[MAXDMABUF];
    uint64_t primaryDmaHandle[MAXDMABUF]; */
} card_params_t;

#endif

