/************************************************************************/
/*									*/
/* File: flx.c							        */
/*									*/
/* driver for the FELIX PCIe cards    				        */
/*									*/
/* Developed by J. Vermeulen (NIKHEF) bu using code from robinnp.c	*/
/* Now maintained by M. Joos (CERN)					*/
/*									*/
/************ C 2016 - The software with that certain something *********/

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/pagemap.h>
#include <linux/page-flags.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/delay.h>

#include "flx_common.h"


//MJ: TO-DO
//MJ: All "msi" strings could be changed too "msix" in order to indicate that the driver supports only MSI-X type interrupts

/********/
/*Macros*/
/********/
#define DRIVER_START_STOP_DEBUG

#ifdef DRIVER_START_STOP_DEBUG
  #define kdebugStartStop(x) {printk x;}   //MJ: Why dont we use kdebug instead?
#else
  #define kdebugStartStop(x)
#endif

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


/***********/
/*Constants*/
/***********/
#define PROC_MAX_CHARS        0x10000      //The max. length of the output of /proc/flx
#define PCI_VENDOR_ID_FLX_FW1 0x10ee       //Note: The different H/W types (709, 710 and 711) do not have specific F/W files
#define PCI_DEVICE_ID_FLX_FW1 0x7038       //      Therefore the device ID refers to the F/W release, not to the type of PCIe card
#define PCI_VENDOR_ID_FLX_FW2 0x10ee
#define PCI_DEVICE_ID_FLX_FW2 0x7039
#define FIRSTMINOR            0
#define MAXMSI		      8            // Max. number of interrupts (MSI-X) per card


/********************/
/*driver error codes*/
/********************/
#define RD_KMALLOC 1       //MJ: Why are 2 and 5-7 not used?
#define RD_CFU     3
#define RD_PROC    8


/************/
/*Prototypes*/
/************/
static int flx_init(void);
static int flx_Probe(struct pci_dev*, const struct pci_device_id*);
static int fill_proc_read_text(void);
static ssize_t flx_write_procmem(struct file *file, const char *buffer, size_t count, loff_t *startOffset);
static ssize_t flx_read_procmem(struct file *file, char *buf, size_t count, loff_t *startOffset);
static void flx_exit(void);
static void flx_Remove(struct pci_dev*);
int flx_mmap(struct file*, struct vm_area_struct*);
static long flx_ioctl(struct file *file, u_int cmd, u_long arg);
int flx_open(struct inode*, struct file*);
int flx_Release(struct inode*, struct file*);
int check_slot_device(u_int);
void flx_vmclose(struct vm_area_struct*);


/************/
/*Structures*/
/************/
static struct pci_device_id FLX_IDs[] =
{
  { PCI_DEVICE(PCI_VENDOR_ID_FLX_FW1, PCI_DEVICE_ID_FLX_FW1) },
  { PCI_DEVICE(PCI_VENDOR_ID_FLX_FW2, PCI_DEVICE_ID_FLX_FW2) },
  { 0, },
};

struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .mmap           = flx_mmap,
  .unlocked_ioctl = flx_ioctl,
  .open           = flx_open,
  .read           = flx_read_procmem,
  .write          = flx_write_procmem,
  .release        = flx_Release,
};

// needed by pci_register_driver fcall
static struct pci_driver flx_PCI_driver =
{
  .name     = "flx",
  .id_table = FLX_IDs,
  .probe    = flx_Probe,
  .remove   = flx_Remove,
};

// memory handler functions used by mmap
static struct vm_operations_struct flx_vm_ops =
{
  .close =  flx_vmclose,             // mmap-close
};

struct irqInfo_struct
{
  int interrupt;
  int card;
  int reserved;
};

typedef struct
{
  uint32_t dummy[2];
} Dummy64Struct;


typedef struct 
{
  Dummy64Struct Dummy0;
} BAR0CommonStruct;


/*********/
/*Globals*/
/*********/
char *devName = "flx";  //the device name as it will appear in /proc/devices
static char *proc_read_text;
static int debug = 1, errorlog = 1;
int cardsFound = 0, interruptCount = 0;
int msiblock = 8, irqFlag[MAXCARDS][MAXMSI] = {{0}}, msiStatus[MAXCARDS];   //MJ: should "msiblock = 8" be "msiblock = MAXMSI"
int irqMasked[MAXCARDS][MAXMSI];
unsigned int irqCount[MAXCARDS][MAXMSI];
uint64_t *memoryBase[MAXCARDS];
uint32_t* msixBar[MAXCARDS], msixPbaOffset[MAXCARDS];  //MJ: msixPbaOffset is only used in debug commands. Is it needed in the driver? What is it for?
card_params_t cards[MAXCARDS];
struct cdev *flx_cdev;
dev_t first_dev;
BAR0CommonStruct *registerBase[MAXCARDS];
static struct irqInfo_struct irqInfo[MAXCARDS][MAXMSI];
struct file *owner[MAXCARDS];

static DECLARE_WAIT_QUEUE_HEAD(waitQueue);
#ifdef MUTEX_ORG
  static DECLARE_MUTEX(procMutex);
  static DECLARE_MUTEX(ownerMutex);
#else
  static DEFINE_MUTEX(procMutex);
  static DEFINE_MUTEX(ownerMutex);
#endif

module_init(flx_init);
module_exit(flx_exit);

MODULE_DESCRIPTION("FLX driver");
MODULE_AUTHOR("Jos Vermeulen (Nikhef) and Markus Joos (CERN)");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_DEVICE_TABLE(pci, FLX_IDs);

MODULE_PARM_DESC(msiblock, "size of MSI block to enable");
module_param(msiblock, int, S_IRUGO);

MODULE_PARM_DESC(debug, "1 = enable debugging   0 = disable debugging");
module_param (debug, int, S_IRUGO | S_IWUSR);

MODULE_PARM_DESC(errorlog, "1 = enable error logging   0 = disable error logging");
module_param (errorlog, int, S_IRUGO | S_IWUSR);


#define USE_MSIX     //MJ: Why do we need this? Are there FELIX cards that do not support MSIX? In addition "USE_MSIX" does not seem to be used in the driver source code 
struct msix_entry msixTable[MAXCARDS][MAXMSI];


/***********************/
static int flx_init(void)
/***********************/
{
    int stat, cardNumber, major, interrupt;
    struct proc_dir_entry *procDir;

    for (cardNumber = 0; cardNumber < MAXCARDS; cardNumber++)
    {
        cards[cardNumber].pciDevice = NULL;
        registerBase[cardNumber] = NULL;
        memoryBase[cardNumber] = NULL;   //MJ: memoryBase does not seem to be used
        for (interrupt = 0; interrupt < MAXMSI; interrupt++)
        {
            irqCount[cardNumber][interrupt] = 0;
            irqMasked[cardNumber][interrupt] = 0;
        }
        owner[cardNumber] = NULL;
    }

    if (msiblock > MAXMSI)
    {
        printk(KERN_ALERT "flx(flx_init):msiblock > MAXMSI - setting to max (%d)\n", MAXMSI);
        msiblock = MAXMSI;  //MJ: We could be less friendly and cause the driver installation to fail if the user spceifies and out-of-bounds value
    }

    kdebugStartStop(("flx(flx_init): registering PCIDriver \n"));  
    stat = pci_register_driver(&flx_PCI_driver);
    if (stat < 0) 
    {
        printk(KERN_ALERT "flx(flx_init): Status %d from pci_register_driver\n", stat);
        return stat;
    }

    stat = alloc_chrdev_region(&first_dev, FIRSTMINOR, MAXCARDS, devName);
    if (stat == 0)
    {
        major = MAJOR(first_dev);  //MJ: major is not used anywhere
        kdebugStartStop((KERN_ALERT "flx(flx_init): major number is %d\n", major));

        flx_cdev = cdev_alloc();
        flx_cdev->ops = &fops;
        flx_cdev->owner = THIS_MODULE;
        stat = cdev_add(flx_cdev, first_dev, 1);
        if (stat == 0)
        {
            procDir = proc_create(devName, 0644, NULL, &fops);
            if (procDir == NULL)
            {
                kerror(("flx(flx_init): error from call to create_proc_entry\n"));
                return(-RD_PROC);
            }
        }
        else
        {
            printk(KERN_ALERT "flx(flx_init): cdev_add failed, driver will not load\n");  //Using both printk an kerror does not look tidy
            unregister_chrdev_region(first_dev, MAXCARDS);
            pci_unregister_driver(&flx_PCI_driver);
            return(stat);
        }
    }
    else
    {
        kerror(("flx_init: registering FLX driver failed.\n"));
        pci_unregister_driver(&flx_PCI_driver);
        return(stat);
    }

    proc_read_text = (char *)kmalloc(PROC_MAX_CHARS, GFP_KERNEL);
    if (proc_read_text == NULL)
    {
        kerror(("flx_init: error from kmalloc\n"));  //MJ: Do we have to unregister the driver and pci device if the kmalloc fails?
        return(-RD_KMALLOC);
    }

    printk(KERN_INFO "flx(flx_init): FLX driver loaded, found %d device(s)\n", cardsFound);
    return 0;
}


/************************/
static void flx_exit(void)
/************************/
{
    remove_proc_entry(devName, NULL /* parent dir */);
    kdebugStartStop(("flx(flx_exit): unregister device\n"));
    unregister_chrdev_region(first_dev, MAXCARDS);
    kdebugStartStop(("flx(flx_exit: unregister driver\n"));
    pci_unregister_driver(&flx_PCI_driver);
    cdev_del(flx_cdev);
    kfree(proc_read_text);
    printk(KERN_INFO "flx(flx__exit): driver removed\n");
}


/**********************************************/
static irqreturn_t irqHandler(int irq, void *dev
#if LINUX_VERSION_CODE==KERNEL_VERSION(2,6,18)     //MJ: Do we have to support pre-2.6.18 kernels?
        , struct pt_regs* regs
#endif
)
/**********************************************/
{
    struct irqInfo_struct *info;

    info = (struct irqInfo_struct*) dev;
    irqCount[info->card][info->interrupt] += 1;
    irqFlag[info->card][info->interrupt] = 1;
    wake_up_interruptible(&waitQueue);      //MJ: would it have any performance advantages if we used one wait queue per card?
    return IRQ_HANDLED;
}


/***********************************************************************/
static int flx_Probe(struct pci_dev *dev, const struct pci_device_id *id)
/***********************************************************************/
{
    int cardNumber, ret, bufferNumber;
    int interrupt, msixCapOffset, msixData, msixBarNumber, msixTableOffset, msixLength;
    uint32_t msixAddress;
    BAR0CommonStruct *registers;

    cardNumber = 0;
    // Find first available slot
    while (cards[cardNumber].pciDevice != NULL && cardNumber < MAXCARDS)
        cardNumber++;

    if (cardNumber < MAXCARDS) 
    {
        kdebug(("flx(flx_Probe): Initialising device nr %d (counting from 0)\n", cardsFound));
        ret = pci_enable_device(dev);
        cardsFound++;
        cards[cardNumber].pciDevice = dev;
    }
    else
    {
        printk(KERN_ALERT "flx(flx_Probe): Too many cards present, only %d is allowed\n", MAXCARDS);
        return -1; //MJ: unspecific error code
    }

    kdebug(("flx(flx_Probe): Reading configuration space for card %d :\n", cardNumber));
    cards[cardNumber].baseAddressBAR0 = pci_resource_start(dev, 0);
    cards[cardNumber].sizeBAR0        = pci_resource_len(dev, 0);
    cards[cardNumber].baseAddressBAR1 = pci_resource_start(dev, 1);
    cards[cardNumber].sizeBAR1        = pci_resource_len(dev, 1);
    cards[cardNumber].baseAddressBAR2 = pci_resource_start(dev, 2);
    cards[cardNumber].sizeBAR2        = pci_resource_len(dev, 2);

    kdebug(("flx(flx_Probe): ----> BAR0 start %x, end %x, size %x \n",
            cards[cardNumber].baseAddressBAR0,
            (unsigned int)pci_resource_end(dev, 0),
            cards[cardNumber].sizeBAR0));
    kdebug(("flx(flx_Probe): ----> BAR1 start %x, end %x, size %x \n",
            cards[cardNumber].baseAddressBAR1,
            (unsigned int)pci_resource_end(dev, 1),
            cards[cardNumber].sizeBAR1));

    /* remap the whole bar0 as registers */
    kdebug(("flx(flx_Probe): Remapping BAR0 as registers space, card %d, phys add/size: 0x%x 0x%x\n", cardNumber, cards[cardNumber].baseAddressBAR0, cards[cardNumber].sizeBAR0));
    registerBase[cardNumber] = ioremap_nocache(cards[cardNumber].baseAddressBAR0, cards[cardNumber].sizeBAR0);
    registers = registerBase[cardNumber];

    msixCapOffset = pci_find_capability(dev, PCI_CAP_ID_MSIX);
    if (msixCapOffset == 0)
    {
        // card may not have flx hardware loaded
        printk(KERN_ALERT "flx(flx_Probe): Failed to map MSI-X BAR for card %d\n", cardNumber);
        msixBar[cardNumber] = NULL;
        return -1; //MJ: unspecific error code
    }

    pci_read_config_dword(dev, msixCapOffset + 4, &msixData);
    msixBarNumber = msixData & 0xf;
    msixTableOffset = msixData & 0xfffffff0;
    kdebug(("flx(flx_Probe): MSIX Vector table BAR %d, offset %08x\n", msixBarNumber, msixTableOffset));
    pci_read_config_dword(dev, msixCapOffset + 8, &msixData);
    msixBarNumber = msixData & 0xf;
    msixPbaOffset[cardNumber] = msixData & 0xfffffff0;
    kdebug(("flx(flx_Probe): MSIX PBA          BAR %d, offset %08x\n", msixBarNumber, msixPbaOffset[cardNumber]));
    msixAddress = pci_resource_start(dev, msixBarNumber);
    msixLength = pci_resource_len(dev, msixBarNumber);
    msixBar[cardNumber] = ioremap_nocache(msixAddress, msixLength);

    if (msixBar[cardNumber] == NULL)
    {
        printk(KERN_ALERT "flx(flx_Probe): Failed to map MSI-X BAR\n for card %d\n", cardNumber);
        return -1; //MJ: unspecific error code
    }

    if (debug)
    {
        bufferNumber = msixTableOffset / sizeof(uint32_t);
        for (interrupt = 0; interrupt < 8 /*msiblock*/ ; interrupt++)
        {
            kdebug(("flx(flx_Probe): MSI-X table[%d] %08x %08x  %08x  %08x\n", interrupt, msixBar[cardNumber][bufferNumber], msixBar[cardNumber][bufferNumber + 1], msixBar[cardNumber][bufferNumber + 2], msixBar[cardNumber][bufferNumber + 3]));
            bufferNumber += 4;
        }

        if (msixPbaOffset[cardNumber] + 3 * sizeof(uint32_t) < msixLength)
        {
            kdebug(("flx(flx_Probe): MSI-X PBA      %08x %08x  %08x  %08x\n",
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t)],
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t) + 1],
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t) + 2],
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t) + 3]));
        }
        else
            printk(KERN_ALERT "flx(flx_Probe): PBA offset %x is outside of BAR%d, length=%x \n", msixPbaOffset[cardNumber], msixBarNumber, msixLength);
    }

    // setup interrupts
    for (interrupt = 0; interrupt < msiblock; interrupt++)
    {
        msixTable[cardNumber][interrupt].entry = interrupt;  //MJ: If (msiblock < MAXMSI) some elements of the array will not be initialized (which may not matter)
        kdebug(("flx(flx_Probe): filling interrupt table for interrupt %d, cardnumber %d\n", interrupt, cardNumber));
        kdebug(("flx(flx_Probe): entry in table %d\n", msixTable[cardNumber][interrupt].entry));
    }

    msiStatus[cardNumber] = pci_enable_msix(dev, msixTable[cardNumber], msiblock);
    kdebug(("After pci_enable_msix\n"));

    if (debug)
    {
        kdebug(("flx(flx_Probe): msix address %08x, length %4x\n", msixAddress, msixLength));
        bufferNumber = msixTableOffset / sizeof(uint32_t);
        for (interrupt = 0; interrupt < msiblock; interrupt++)
        {
            kdebug(("flx(flx_Probe): MSI-X table[%d] %08x %08x  %08x  %08x\n", interrupt, msixBar[cardNumber][bufferNumber], msixBar[cardNumber][bufferNumber+1], msixBar[cardNumber][bufferNumber+2], msixBar[cardNumber][bufferNumber+3]));
            bufferNumber += 4;
        }
        if (msixPbaOffset[cardNumber] + 3 * sizeof(uint32_t) < msixLength)
        {
            kdebug(("flx(flx_Probe): MSI-X PBA %08x \n", msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t)]));
        }
        else
            printk(KERN_ALERT "flx(flx_Probe): PBA offset %x is outside of BAR%d, length=%x \n", msixPbaOffset[cardNumber], msixBarNumber, msixLength);
    }

    if (msiStatus[cardNumber] != 0)
    {
        printk(KERN_ALERT "flx(flx_Probe): Failed to enable MSI-X interrupt block for card %d, enable returned %d\n", cardNumber, msiStatus[cardNumber]);
    }
    else
    {
        for (interrupt = 0; interrupt < msiblock; interrupt++)
        {
            kdebug(("flx(flx_Probe): Trying to register IRQ vector %d\n", msixTable[cardNumber][interrupt].vector));

            irqInfo[cardNumber][interrupt].interrupt = interrupt;  //MJ: If we have two cards then two elements of the array will have the same value (interrupt). Does that matter?
            irqInfo[cardNumber][interrupt].card      = cardNumber;
            irqInfo[cardNumber][interrupt].reserved  = 0;          //MJ: It seems ".reserved" is not used anywhere
            ret = request_irq(msixTable[cardNumber][interrupt].vector, irqHandler, 0, devName, &irqInfo[cardNumber][interrupt]);
            if (ret != 0)
            {
                printk(KERN_ALERT "flx(flx_Probe): Failed to register interrupt handler for MSI %d\n", interrupt);  //MJ: Why not use kerror?
            }
        }
    }

    // do reset

    if (debug)
    {
        kdebug(("flx(flx_Probe): msix address %08x, length %4x\n", msixAddress, msixLength));
        bufferNumber = msixTableOffset / sizeof(uint32_t);
        for (interrupt = 0; interrupt < msiblock; interrupt++)
        {
            kdebug(("flx(flx_Probe): MSI-X table[%d] %08x %08x  %08x  %08x\n", interrupt, msixBar[cardNumber][bufferNumber], msixBar[cardNumber][bufferNumber+1], msixBar[cardNumber][bufferNumber+2], msixBar[cardNumber][bufferNumber+3]));
            bufferNumber += 4;
        }

        if (msixPbaOffset[cardNumber] + 3 * sizeof(uint32_t) < msixLength)
        {
            kdebug(("flx(flx_Probe): MSI-X PBA %08x %08x  %08x  %08x\n",
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t)],
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t) + 1],
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t) + 2],
                    msixBar[cardNumber][msixPbaOffset[cardNumber] / sizeof(uint32_t) + 3]));
        }
        else
        {
            printk(KERN_ALERT "flx(flx_Probe): PBA offset %x is outside of BAR%d, length=%x \n", msixPbaOffset[cardNumber], msixBarNumber, msixLength);
        }
    }

    // other initialization ...
    return 0;
}


/*****************************************/
static void flx_Remove(struct pci_dev *dev)
/*****************************************/
{
    int cardNumber, interrupt;

    printk(KERN_ALERT "flx(flx_Remove):  called\n");
    for(cardNumber = 0; cardNumber < MAXCARDS; cardNumber++)
    {
        if (cards[cardNumber].pciDevice == dev)
        {
            printk(KERN_ALERT "flx(flx_Remove): for card %d\n", cardNumber);
            cards[cardNumber].pciDevice = NULL;
            owner[cardNumber] = NULL;
            cardsFound--;

            if (msiStatus[cardNumber] == 0)
            {
                for (interrupt = 0; interrupt < msiblock; interrupt++)
                {
                    kdebug(("flx(flx_Remove): unregestering interrupt %d, vector %d\n", interrupt, msixTable[cardNumber][interrupt].vector));
                    free_irq(msixTable[cardNumber][interrupt].vector, &irqInfo[cardNumber][interrupt]);
                }
            }
            pci_disable_msix(dev);
        }
    }
}


/************************************************/
int flx_open(struct inode *ino, struct file *file)
/************************************************/
{
    card_params_t *pdata;

    kdebug(("flx(flx_open): called\n"));
    pdata = (card_params_t *)kmalloc(sizeof(card_params_t), GFP_KERNEL);
    if (pdata == NULL)
    {
        kerror(("flx(flx_open): error from kmalloc\n"))
        return(-RD_KMALLOC);
    }
    
    pdata->slot = 0;
    file->private_data = (char *)pdata;
    return 0;
}


/***************************************************/
int flx_Release(struct inode *ino, struct file *file)
/***************************************************/
{
    card_params_t *pdata;
    int card;

    kdebug(("flx(flx_release): called\n"));
    for (card = 0; card < MAXCARDS; card++)
    {
        if (owner[card] == file)
            owner[card] = 0;
    }
    
    pdata = (card_params_t *)file->private_data;
    kdebug(("flx(flx_release): Releasing orphaned resources for slot %d\n", pdata->slot))
    kfree(file->private_data);
    return 0;
}


/*******************************/
int check_slot_device(u_int slot)  //MJ: function could be merged back into SETCARD ioctl (only called from one place)
/*******************************/
{
    if (slot >= MAXCARDS)
    {
        printk(KERN_ALERT "flx(check_slot_device): Invalid (%d) slot number\n", slot);
        return -1;
    }
    if (cards[slot].pciDevice == NULL)
    {
        printk(KERN_ALERT "flx(check_slot_device): No card at this (%d) slot!\n", slot);
        return -1;
    }
    return 0;
}


/**********************************************************************************************/
static ssize_t flx_read_procmem(struct file *file, char *buf, size_t count, loff_t *startOffset)
/**********************************************************************************************/
{
    int nchars = 0, loop1;
    static int len = 0;
    long offset = *startOffset;

    kdebug(("flx(flx_read_proc): Called with buf    = 0x%016lx\n", (u_long)buf));
    kdebug(("flx(flx_read_proc): Called with offset = %ld\n", offset));
    kdebug(("flx(flx_read_proc): Called with count  = %d\n", (int) count));

#ifdef MUTEX_ORG     //MJ: Why do we have two ways of dealing with the mutexes?
    if (down_interruptible(&procMutex))
#else
    if (mutex_lock_interruptible(&procMutex))
#endif
    {
        return 0;
    }

    if (offset == 0) 
        len = fill_proc_read_text();
    kdebug(("flx(flx_read_proc): len = %d\n", len));

    if (count < (len - offset))
        nchars = count;
    else
        nchars = len - offset;

    kdebug(("flx(flx_read_proc): min nchars         = %d\n", nchars));
    kdebug(("flx(flx_read_proc): position           = %ld\n", (offset & (PAGE_SIZE - 1))));

    if (nchars > 0)
    {
        for (loop1 = 0; loop1 < nchars; loop1++)
            buf[loop1 + (offset & (PAGE_SIZE - 1))] = proc_read_text[offset + loop1];
	    
        *startOffset = len + (offset & (PAGE_SIZE - 1));
    }
    else
        nchars = 0;

    kdebug(("flx(flx_read_proc): returning *start   = 0x%016lx\n", (u_long) * startOffset));
    kdebug(("flx(flx_read_proc): returning nchars   = %d\n", nchars));
    
#ifdef MUTEX_ORG
    up(&procMutex);
#else
    mutex_unlock(&procMutex);
#endif

    return(nchars);
}


/********************************************************************************************************/
static ssize_t flx_write_procmem(struct file *file, const char *buffer, size_t count, loff_t *startOffset)
/********************************************************************************************************/
{
  int len;
  char textReceived[100];

  kdebug(("flx(flx_write_proc): robin_write_procmem called\n"));

  if (count > 99) 
      len = 99;
  else 
      len = count;

  if (copy_from_user(textReceived, buffer, len))
  {
    kerror(("flx(flx_write_proc): error from copy_from_user\n"));
    return(-RD_CFU);
  }
  
  kdebug(("flx(flx_write_proc): len = %d\n", len));
  textReceived[len - 1] = '\0';
  kdebug(("flx(flx_write_proc): text passed = %s\n", textReceived));

  if (!strcmp(textReceived, "debug"))
  {
    debug = 1;
    kdebug(("flx(flx_write_proc): debugging enabled\n"));
  }
  
  if (!strcmp(textReceived, "nodebug"))
  {
    kdebug(("flx(flx_write_proc): debugging disabled\n"));
    debug = 0;
  }
  
  if (!strcmp(textReceived, "elog"))
  {
    kdebug(("flx(flx_write_proc): Error logging enabled\n"))
    errorlog = 1;
  }
  
  if (!strcmp(textReceived, "noelog"))
  {
    kdebug(("flx(flx_write_proc): Error logging disabled\n"))
    errorlog = 0;
  }
  return(len);
}


/**********************************/
static int fill_proc_read_text(void)
/**********************************/
{
    //MJ-SMP: protect this function (preferrably with a spinlock)  //MJ: Do we need the spinlock?
    int interrupt, cardIndex[MAXCARDS], index;
    u_int len, card;

    index = 0;
    for (card = 0; card < cardsFound; card++)
    {
        while (cards[index].pciDevice == NULL) index++;

        if (index < MAXCARDS)
        {
            cardIndex[card] = index;
            index++;
            kdebug(("flx(fill_proc_read_text): card %d has index %d\n", card, index));
        }
        else
        {
            kerror(("flx(fill_proc_read_text): Card indexing error\n"));
            return 0;
        }
    }

    kdebug(("flx(fill_proc_read_text): Creating text....\n"));
    len = 0;
    len += sprintf(proc_read_text + len, "FLX driver for release %s (based on SVN tag %s, flx.c revision %s)\n", RELEASE_NAME, CVSTAG, FLX_TAG);
    len += sprintf(proc_read_text + len, "\nDebug                       = %d\n", debug);
    len += sprintf(proc_read_text + len, "Number of cards detected    = %d\n", cardsFound);

    for (card = 0; card < cardsFound; card++)
    {
        // Addresses depend on firmware version
        uint32_t *pp = ioremap_nocache(cards[card].baseAddressBAR2, cards[card].sizeBAR2);

        unsigned int buildDate = *(pp + 4);
        unsigned int buildYear = *(pp + 5);
        unsigned int buildRevision = *(pp + 8);
        unsigned int genConstants = *(pp + 16);

        int buildMonth = (buildDate >> 24) & 0xff;
        int buildDay = (buildDate >> 16) & 0xff;
        int buildHour = (buildDate >> 8) & 0xff;
        int buildMinute = buildDate & 0xff;
        len += sprintf(proc_read_text + len, "\nCard %d: ", cardIndex[card]);
        len += sprintf(proc_read_text + len, "build revision: %x, ", buildRevision);
        len += sprintf(proc_read_text + len, "date: %2x-%x-20%x, time: %xh%x\n", buildDay, buildMonth, buildYear, buildHour, buildMinute);
        len += sprintf(proc_read_text + len, "Number of descriptors: %d, ", (genConstants & 0xff));
        len += sprintf(proc_read_text + len, "number of interrupts : %d\n", ((genConstants >> 8) & 0xff));

        if (msixBar[cardIndex[card]] != NULL)
        {
            len += sprintf(proc_read_text + len, "Interrupt count |");
            for (interrupt = 0; interrupt < msiblock; interrupt++)
                len += sprintf(proc_read_text + len, " %6d |", irqCount[cardIndex[card]][interrupt]);

            len += sprintf(proc_read_text + len, "\nInterrupt flag  |");
            for (interrupt = 0; interrupt < msiblock; interrupt++)
                len += sprintf(proc_read_text + len, " %6d |", irqFlag[cardIndex[card]][interrupt]);

            len += sprintf(proc_read_text + len, "\nInterrupt mask  |");
            for (interrupt = 0; interrupt < msiblock; interrupt++)
                len += sprintf(proc_read_text + len, " %6d |", irqMasked[cardIndex[card]][interrupt]);

            len += sprintf(proc_read_text + len, "\nMSIX PBA        %08x\n",  msixBar[cardIndex[card]][msixPbaOffset[cardIndex[card]] / sizeof(uint32_t)]);
            len += sprintf(proc_read_text + len, "\n");
        }
        else
            len += sprintf(proc_read_text + len, "No MSIX interrupts for card %d\n\n", card);
    }

    len += sprintf(proc_read_text + len, " \n");
    len += sprintf(proc_read_text + len, "The command 'echo <action> > /proc/flx', executed as root,\n");
    len += sprintf(proc_read_text + len, "allows you to interact with the driver. Possible actions are:\n");
    len += sprintf(proc_read_text + len, "debug   -> Enable debugging\n");
    len += sprintf(proc_read_text + len, "nodebug -> Disable debugging\n");
    len += sprintf(proc_read_text + len, "elog    -> Log errors to /var/log/message\n");
    len += sprintf(proc_read_text + len, "noelog  -> Do not log errors to /var/log/message\n");
    kdebug(("flx(fill_proc_read_text): Number of characters created = %d\n", len));
    return(len);
}


/*********************************************************/
int flx_mmap(struct file *file, struct vm_area_struct *vma)
/*********************************************************/
{
    u32 moff, msize;

    /* it should be "shared" memory */
    if ((vma->vm_flags & VM_WRITE) && !(vma->vm_flags & VM_SHARED))
    {
        printk(KERN_ALERT "flx(flx_mmap): writeable mappings must be shared, rejecting\n");
        return -1;  //MJ: unspecific error
    }

    msize = vma->vm_end - vma->vm_start;
    moff = vma->vm_pgoff;
    kdebug(("flx(flx_mmap): offset: 0x%x, size: 0x%x\n", moff, msize));
    moff = moff << PAGE_SHIFT;
    if (moff & ~PAGE_MASK)
    {
        printk(KERN_ALERT "flx(flx_mmap): offset not aligned: %u\n", moff);
        return(-1);  //MJ: unspecific error
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
    vma->vm_flags |= VM_RESERVED;
#else
    vma->vm_flags |= VM_DONTEXPAND;
    vma->vm_flags |= VM_DONTDUMP;
#endif

    // we do not want to have this area swapped out, lock it
    vma->vm_flags |= VM_LOCKED;
    if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, msize, vma->vm_page_prot) != 0)
    {
        printk(KERN_ALERT "flx(flx_mmap): remap page range failed\n");
        return -EAGAIN;
    }

    vma->vm_ops = &flx_vm_ops;
    return 0;
}


/******************************************/
void flx_vmclose(struct vm_area_struct *vma)
/******************************************/
{
    kdebug(("flx(flx_mmap): closing mmap memory\n"));
}


/*************************************************************/
static long flx_ioctl(struct file *file, u_int cmd, u_long arg)
/*************************************************************/
{
    card_params_t *cardParams;
    static struct vm_area_struct *vmas, uvmas;
    unsigned int interrupt, card;
    char capabilityId, capabilityIdOffset;
    u_int address;
    u_short deviceControlRegister;
    int tlp, count;

    kdebug(("flx(flx_ioctl): entered\n"));
    vmas = &uvmas;   //MJ: purpose unclear

    switch(cmd)
    {
    case GETCARDS:
        kdebug(("flx(flx_ioctl) GETCARDS\n"));
        if (copy_to_user(((int*)arg), &cardsFound, sizeof(int)) != 0)
        {
            printk(KERN_ALERT "flx(flx_ioctl,GETCARDS): Copy cardsFound to user space failed!\n");
            return(-1); //MJ: unspecific error
        }
        break;
	
    case GET_TLP:
        kdebug(("flx(flx_ioctl) GET_TLP\n"));
        cardParams = (card_params_t*)file->private_data;
        card = cardParams->slot;
        address = 0x34;
        pci_read_config_byte(cards[card].pciDevice, address, &capabilityIdOffset);
        kdebug(("flx(flx_ioctl) GET_TLP first capabilityIdOffset %x\n", (u_char) capabilityIdOffset));
        // count protects against loop not terminating
        count = 0;
        while (count < 10)  //MJ: What is magic about the value 10?
        {
            pci_read_config_byte(cards[card].pciDevice, (u_int) capabilityIdOffset, &capabilityId);
            kdebug(("flx(flx_ioctl) GET_TLP capabilityIdOffset %x capabilityId %x\n", (u_char) capabilityIdOffset, capabilityId));
            if (capabilityId == 0x10)
                break;

            address = (u_int) (capabilityIdOffset + 1);
            pci_read_config_byte(cards[card].pciDevice, address, &capabilityIdOffset);
            kdebug(("flx(flx_ioctl) GET_TLP new capabilityIdOffset %x\n", (u_char) capabilityIdOffset));
            count ++;
        }
        if (count == 10)
        {
            printk(KERN_ALERT "flx(flx_ioctl,GET_TLP): Did not find capability with TLP id\n");
            return(-1); //MJ: unspecific error
        }
        address = capabilityIdOffset + 8;
        pci_read_config_word(cards[card].pciDevice, address, &deviceControlRegister);
        kdebug(("flx(flx_ioctl) GET_TLP new deviceControlRegister %x\n", deviceControlRegister));
        tlp = (deviceControlRegister >> 5) & 0x7;
        if (copy_to_user(((int*)arg), &tlp, sizeof(int)) != 0)
        {
            printk(KERN_ALERT "flx(flx_ioctl,GET_TLP): Copy value of TLP to user space failed!\n");
            return(-1); //MJ: unspecific error
        }
        break;
	
    case WAIT_IRQ:    //WAIT_DMA
        kdebug(("flx(flx_ioctl) WAIT_IRQ\n"));
        cardParams = (card_params_t*)file->private_data;
        card = cardParams->slot;
        if (copy_from_user(&interrupt, (void *)arg, sizeof(unsigned int)) != 0)
        {
            kerror(("flx(flx_ioctl WAIT_IRQ): error from copy_from_user\n"));
            return(-RD_CFU);
        }
        if (interrupt >= msiblock)
        {
            kerror(("flx(flx_ioctl) WAIT_IRQ): invalid interrupt specified %d\n", interrupt));
            return(-1);  //MJ: -1 may not be the best solution. We should define a RD_MSI error code
        }
        wait_event_interruptible(waitQueue, irqFlag[card][interrupt] == 1);  
	//MJ: Rubini recommends to check the return code of wait_event_interruptible.  
	//MJ: in case of a non-zero valu e -ERESTARTSYS should be returned
	
        irqFlag[card][interrupt] = 0;  
	//MJ: According to Rubini there can be problems with race conditions if two processes are waiting for the same interrupt.
	//MJ: Is that a requirement for us?
	
        kdebug(("flx(flx_ioctl) WAIT_IRQ): finished waiting for IRQ %d\n", interrupt));
        break;

    case CANCEL_IRQ_WAIT:
        kdebug(("flx(flx_ioctl) CANCEL_IRQ_WAIT  looping over %d interrupts\n", msiblock));
        cardParams = (card_params_t*)file->private_data;
        card = cardParams->slot;
        // set all flags to 1, wake_up_interruptible will only result in waking up a process if the flag specified
        // in wait_event_interruptible is set to 1
        for (interrupt = 0; interrupt < msiblock; interrupt++)
            irqFlag[card][interrupt] = 1;          //MJ: The access to the interrupt related data structures seems to be vulnerable to race conditions (two processes callinh different ioctls ate the same time)

        /* Wake up everybody who was waiting for an interrupt */
        wake_up_interruptible(&waitQueue);   //MJ: Woken up process will not know that the interrupt has not arrived.
        break;
	
    case RESET_IRQ_COUNTERS:
        kdebug(("flx(flx_ioctl) RESET_IRQ_COUNTERS: looping over %d interrupts\n", msiblock));
        cardParams = (card_params_t*)file->private_data;
        card = cardParams->slot;
        for (interrupt = 0; interrupt < msiblock; interrupt++)
            irqCount[card][interrupt] = 0;

        break;
	
    case RESET_IRQ_FLAGS:
        kdebug(("flx(flx_ioctl) RESET_IRQ_FLAGS: looping over %d interrupts\n", msiblock));
        cardParams = (card_params_t*)file->private_data;
        card = cardParams->slot;
        for (interrupt = 0; interrupt < msiblock; interrupt++)
            irqFlag[card][interrupt] = 0;

        break;
	
    case MASK_IRQ:
        cardParams = (card_params_t*)file->private_data;
        card = cardParams->slot;
        if (copy_from_user(&interrupt, (void *)arg, sizeof(unsigned int)) != 0)
        {
            kerror(("flx(flx_ioctl MASK_IRQ): error from copy_from_user\n"));
            return(-RD_CFU);
        }
        if (interrupt >= msiblock)
        {
            kerror(("flx(flx_ioctl) MASK_IRQ): invalid interrupt specified %d\n", interrupt));
            return(-1);  //MJ: unspecific error
        }
        // check that interrupt was not already masked
        if (irqMasked[card][interrupt] == 0)
        {
            disable_irq(msixTable[card][interrupt].vector);
            irqMasked[card][interrupt] = 1;
            kdebug(("flx(flx_ioctl) MASK_IRQ: masked interrupt %d\n", interrupt));
        }
        else
        {
            kdebug(("flx(flx_ioctl) MASK_IRQ: interrupt %d already masked -> no action\n", interrupt));
        }
        break;
	
    case UNMASK_IRQ:
        cardParams = (card_params_t*)file->private_data;
        card = cardParams->slot;
        if (copy_from_user(&interrupt, (void *)arg, sizeof(unsigned int)) != 0)
        {
            kerror(("flx(flx_ioctl UNMASK_IRQ): error from copy_from_user\n"));
            return(-RD_CFU);
        }
        if (interrupt >= msiblock)
        {
            kerror(("flx(flx_ioctl) UNMASK_IRQ): invalid interrupt specified %d\n", interrupt));
            return(-1);
        }
        kdebug(("flx(flx_ioctl) UNMASK_IRQ: unmasked interrupt %d\n", interrupt));
        // check that interrupt was not already unmasked
        if (irqMasked[card][interrupt] == 1)
        {
            enable_irq(msixTable[card][interrupt].vector);
            irqMasked[card][interrupt] = 0;
            kdebug(("flx(flx_ioctl) UNMASK_IRQ: unmasked interrupt %d\n", interrupt));
        }
        else
        {
            kdebug(("flx(flx_ioctl) UNMASK_IRQ: interrupt %d already unmasked -> no action\n", interrupt));
        }
        break;
	
    case SETCARD:
        cardParams = (card_params_t*)arg;
        card = cardParams->slot;
        kdebug(("flx(flx_ioctl) SETCARD: card = %d\n", card));
        if (check_slot_device(card) != 0)
        {
            printk(KERN_ALERT "flx(flx_ioctl ,SETCARD): Wrong slot !\n");
            return -6;  //MJ: replace 6 by macro
        }

#ifdef MUTEX_ORG
    if (down_interruptible(&ownerMutex))
#else
    if (mutex_lock_interruptible(&ownerMutex))
#endif
        {
            return -4;  //MJ: replace 4 by macro
        }
        owner[card] = file;
#ifdef MUTEX_ORG
    up(&ownerMutex);
#else
    mutex_unlock(&ownerMutex);
#endif

        cardParams = (card_params_t*)file->private_data;
        cardParams->slot = card;
        cardParams->baseAddressBAR0 = cards[card].baseAddressBAR0;
        cardParams->sizeBAR0 = cards[card].sizeBAR0;
        cardParams->baseAddressBAR1 = cards[card].baseAddressBAR1;
        cardParams->sizeBAR1 = cards[card].sizeBAR1;
        cardParams->baseAddressBAR2 = cards[card].baseAddressBAR2;
        cardParams->sizeBAR2 = cards[card].sizeBAR2;
        // OK, we have a valid slot, copy configuration back to user
        if (copy_to_user(((card_params_t*)arg), &cards[card], sizeof(card_params_t)) != 0)
        {
            printk(KERN_ALERT "flx(flx_ioctl ,SETCARD): Copy card_params_t to user space failed!\n");
            return(-1);  //MJ: unspecific error
        }
        kdebug(("flx(flx_ioctl, SETCARD): end of ioctl SETCARD\n"));
        break;
    default:
        printk(KERN_ALERT "flx(flx_ioctl) default: Unknown ioctl %x\n", cmd);
        return(-1);
    }

    return 0;
}

