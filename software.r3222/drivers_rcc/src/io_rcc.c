/************************************************************************/
/*									*/
/* File: io_rcc.c							*/
/*									*/
/* IO RCC driver							*/
/*									*/
/*  7. Jun. 02  MAJO  created						*/
/*									*/
/************ C 2005 - The software with that certain something *********/

/************************************************************************/
/*NOTES:								*/
/*- This driver should work on kernels from 2.6.9 onwards		*/
/************************************************************************/

#include <linux/init.h>           //MJ: for 2.6, p30
#include <linux/module.h>         //p30
#include <linux/moduleparam.h>    //MJ: for 2.6, p30
#include <linux/kernel.h>         //e.g. for printk
#include <linux/stat.h>           //MJ: for 2.6, e.g. for module_param
#include <linux/pci.h>            //e.g. for   pci_find_device
#include <linux/errno.h>          //e.g. for EFAULT
#include <linux/fs.h>             //e.g. for register_chrdev
#include <linux/sched.h>          //MJ: for current->pid (first needed with SLC6)
#include <linux/proc_fs.h>        //e.g. for create_proc_entry
#include <linux/mm.h>             //e.g. for vm_operations_struct
#include <linux/version.h>
#include <linux/slab.h>           //e.g. for kmalloc
#include <linux/cdev.h>           //e.g. for cdev_alloc
#include <asm/uaccess.h>          //e.g. for copy_from_user
#include <asm/io.h>               //e.g. for inl
#include "io_rcc/io_rcc_driver.h"
#include "ROSRCDdrivers/tdaq_drivers.h"


/*********************************** Notes **************************************/
/*										*/
/* In 2.6 driver parameters (such as "debug") can be accessed via /sys/modlue/  */
/* Therefore it may be possible to remove them from the writable /proc file     */
/*										*/
/* Rubini (p83) recommends to replace /proc by /sysfs				*/
/* The seq_file service could be used for proc_read				*/
/* and /sysfs for proc_write. The details need to be understood 		*/
/* (and can be tricky)								*/
/*										*/
/* It is not clear if SLC4 will provide support for the devfs. If though the	*/
/* creation of nodes could be simplified. Seems devfs was replaced by udev.	*/
/*										*/
/* The book of Rubini is based on 2.6.10. SLC4 is 2.6.9				*/
/********************************************************************************/


/*********/
/*Globals*/
/*********/
static int debug = 0, errorlog = 1;
static u_int board_type;
static char *proc_read_text;
static pci_devices_t pcidev[IO_MAX_PCI];
static dev_t major_minor;
static struct cdev *io_rcc_cdev;


/***************************************************************/
/* Use /sbin/modinfo <module name> to extract this information */
/***************************************************************/
MODULE_DESCRIPTION("PCI IO driver");
MODULE_AUTHOR("Markus Joos, CERN/PH");
MODULE_VERSION("2.0");
MODULE_LICENSE("Private: Contact markus.joos@cern.ch");
module_param (debug, int, S_IRUGO | S_IWUSR);     //MJ: for 2.6 p37
MODULE_PARM_DESC(debug, "1 = enable debugging   0 = disable debugging");
module_param (errorlog, int, S_IRUGO | S_IWUSR);  //MJ: for 2.6 p37
MODULE_PARM_DESC(errorlog, "1 = enable error logging   0 = disable error logging");


//MJ: Not actually required. Just for kdebug
struct vm_operations_struct io_rcc_vm_ops =
{
  .close = io_rcc_vmaClose,
  .open = io_rcc_vmaOpen,      //MJ: Note the comma at the end of the list!
};


static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .unlocked_ioctl = io_rcc_ioctl,
  .open           = io_rcc_open,
  .mmap           = io_rcc_mmap,
  .release        = io_rcc_release,
  .read           = io_rcc_read_procmem,
  .write          = io_rcc_proc_write,

};



/*****************************/
/* Standard driver functions */
/*****************************/

/**************************/
static int io_rcc_init(void)  //MJ: Declared "static" because we do not want to export it
/**************************/
{
  int loop, ecode = 0;
  u_char reg;
  static struct proc_dir_entry *io_rcc_file;

  //MJ: for 2.6//SET_MODULE_OWNER(&fops);

  // Read the board identification (works only on VP110/31x). MJ: Do we still need this?
  outb(BID1, CMOSA);
  reg = inb(CMOSD);
  kdebug(("io_rcc(io_rcc_init): BID1 = 0x%02x\n", reg));

  if (!(reg & 0x80))
  {
    kdebug(("io_rcc(io_rcc_init): Unable to determine board type\n"));
    board_type = VP_UNKNOWN;
  }
  else
  {
    outb(BID2, CMOSA);
    reg = inb(CMOSD);
    kdebug(("io_rcc(io_rcc_init): BID2 = 0x%02x\n", reg));

    reg &= 0x1f;  // Mask board ID bits
    board_type = VP_UNKNOWN;
         if (reg == 2) board_type = VP_PSE;  // VP-PSE
    else if (reg == 3) board_type = VP_PSE;  // VP-PSE
    else if (reg == 4) board_type = VP_PSE;  // VP-PSE
    else if (reg == 5) board_type = VP_PMC;  // VP-PMC
    else if (reg == 6) board_type = VP_100;  // VP-100
    else if (reg == 7) board_type = VP_CP1;  // VP-CP1
    else  board_type = 0;
    if (board_type == VP_PSE)
      {kdebug(("io_rcc(io_rcc_init): Board type = VP-PSE\n"));}
    if (board_type == VP_PMC)
      {kdebug(("io_rcc(io_rcc_init): Board type = VP-PMC\n"));}
    if (board_type == VP_100)
      {kdebug(("io_rcc(io_rcc_init): Board type = VP-100\n"));}
    if (board_type == VP_CP1)
      {kdebug(("io_rcc(io_rcc_init): Board type = VP-CP1\n"));}
    if (!board_type)
      {kdebug(("io_rcc(io_rcc_init): Unable to determine board type(2)\n"));}
  }

  ecode = alloc_chrdev_region(&major_minor, 0, 1, "io_rcc"); //MJ: for 2.6 p45
  if (ecode)
  {
    kerror(("io_rcc(io_rcc_init): failed to obtain device numbers\n"));
    goto fail1;
  }

  proc_read_text = (char *)kmalloc(MAX_PROC_TEXT_SIZE, GFP_KERNEL);
  if (proc_read_text == NULL)
  {
    ecode = -ENOMEM;
    kerror(("io_rcc(io_rcc_init): error from kmalloc\n"));
    goto fail2;
  }

  io_rcc_file = proc_create("io_rcc", 0644, NULL, &fops);
  if (io_rcc_file == NULL)
  {
    kerror(("io_rcc(io_rcc_init): error from call to create_proc_entry\n"));
    ecode = -EFAULT;
    goto fail3;
  }

  for(loop = 0; loop < IO_MAX_PCI; loop++)
    pcidev[loop].pid = 0;

  io_rcc_cdev = (struct cdev *)cdev_alloc();      //MJ: for 2.6 p55
  io_rcc_cdev->ops = &fops;
  ecode = cdev_add(io_rcc_cdev, major_minor, 1);  //MJ: for 2.6 p56
  if (ecode)
  {
    kerror(("io_rcc(io_rcc_init): error from call to cdev_add()\n"));
    goto fail4;
  }

  kdebug(("io_rcc(io_rcc_init): driver loaded; major device number = %d\n", MAJOR(major_minor)));
  return(0);

  fail4:
    remove_proc_entry("io_rcc", NULL);

  fail3:
    kfree(proc_read_text);

  fail2:
    unregister_chrdev_region(major_minor, 1); //MJ: for 2.6 p45

  fail1:
    return(ecode);
}


/******************************/
static void io_rcc_cleanup(void)
/******************************/
{
  cdev_del(io_rcc_cdev);                     //MJ: for 2.6 p56
  remove_proc_entry("io_rcc", NULL);
  kfree(proc_read_text);
  unregister_chrdev_region(major_minor, 1);  //MJ: for 2.6 p45
  kdebug(("io_rcc(io_rcc_cleanup): driver removed\n"));
}


module_init(io_rcc_init);    //MJ: for 2.6 p16
module_exit(io_rcc_cleanup); //MJ: for 2.6 p16


/***********************************************************/
static int io_rcc_open(struct inode *ino, struct file *filep)
/***********************************************************/
{
  int loop;
  private_stuff *pptr;

  //reserve space to store information about the devices managed by this "file"
  pptr = (private_stuff *)kmalloc(sizeof (private_stuff), GFP_KERNEL);
  if (pptr == NULL)
  {
    kerror(("io_rcc_drv(io_rcc_open): error from kmalloc\n"));
    return(-EFAULT);
  }

  //Initialize the space
  for (loop = 0; loop < IO_MAX_PCI; loop++)
    pptr->linked[loop] = 0;

  filep->private_data = pptr;
  kdebug(("io_rcc_drv(io_rcc_open): private_data = 0x%08lx\n", (u_long)filep->private_data));

  return(0);
}


/**************************************************************/
static int io_rcc_release(struct inode *ino, struct file *filep)
/**************************************************************/
{
  int loop;
  private_stuff *pptr;

  kdebug(("io_rcc(release): pid = %d\n", current->pid));
  pptr = (private_stuff *) filep->private_data;

  // Release orphaned links to PCI devices
  for(loop = 0; loop < IO_MAX_PCI; loop++)
  {
    if (pptr->linked[loop] == 1)
    {
      kdebug(("io_rcc(release): Orphaned PCI device unlinked (pid=%d: vid=0x%08x did=0x%08x)\n", pcidev[loop].pid, pcidev[loop].vid, pcidev[loop].did));
      pcidev[loop].pid = 0;
      pptr->linked[loop] = 0;
    }
  }

  kfree(pptr);
  return(0);
}


/****************************************************************/
static long io_rcc_ioctl(struct file *file, u_int cmd, u_long arg)
/****************************************************************/
{
  private_stuff *pptr;

  pptr = (private_stuff *) file->private_data;

  switch (cmd)
  {
    case IOPEEK:
    {
      IO_RCC_IO_t params;
      int ret;

      ret = copy_from_user(&params, (void *)arg, sizeof(IO_RCC_IO_t));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPEEK): error %d from copy_from_user\n",ret));
    return(-EFAULT);
      }

      if (params.size == 4)
      {
        params.data = inl(params.offset);
        kdebug(("io_rcc(ioctl,IOPEEK): 0x%08x (int) read from 0x%08x\n", params.data, params.offset));
      }
      if (params.size == 2)
      {
        params.data = inw(params.offset);
        kdebug(("io_rcc(ioctl,IOPEEK): 0x%08x (word) read from 0x%08x\n", params.data, params.offset));
      }
      if (params.size == 1)
      {
        params.data = inb(params.offset);
        kdebug(("io_rcc(ioctl,IOPEEK): 0x%08x (byte) read from 0x%08x\n", params.data, params.offset));
      }

      if (copy_to_user((void *)arg, &params, sizeof(IO_RCC_IO_t)) != 0)
      {
    kerror(("io_rcc(ioctl,IOPEEK): error from copy_to_user\n"));
    return(-EFAULT);
      }
      break;
    }

    case IOPOKE:
    {
      IO_RCC_IO_t params;
      int ret;

      ret = copy_from_user(&params, (void *)arg, sizeof(IO_RCC_IO_t));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPOKE): error %d from copy_from_user\n",ret));
    return(-EFAULT);
      }

      if (params.size == 4)
      {
        kdebug(("io_rcc(ioctl,IOPOKE): writing 0x%08x (int) to 0x%08x\n", params.data, params.offset));
        outl(params.data, params.offset);
        wmb();    // Recommended by Rubini on p247
      }
      if (params.size == 2)
      {
        kdebug(("io_rcc(ioctl,IOPOKE): writing 0x%08x (word) to 0x%08x\n", params.data, params.offset));
        outw(params.data&0xffff, params.offset);
        wmb();
      }
      if (params.size == 1)
      {
        kdebug(("io_rcc(ioctl,IOPOKE): writing 0x%08x (byte) to 0x%08x\n", params.data, params.offset));
        outb(params.data&0xff, params.offset);
        wmb();
      }

      if (copy_to_user((void *)arg, &params, sizeof(IO_RCC_IO_t)) != 0)
      {
    kerror(("io_rcc(ioctl,IOPOKE): error from copy_to_user\n"));
    return(-EFAULT);
      }
      break;
    }

/*
    case IOGETID:
    {
      if (copy_to_user((void *)arg, &board_type, sizeof(int)) != 0)
      {
        kerror(("io_rcc(ioctl,IOGETID): error from copy_to_user\n"));
        return(-EFAULT);
      }
      break;
    }
*/
    case IOPCILINK:
    {
      int ret, dnum;
      u_int loop;
      IO_PCI_FIND_t find;
      struct pci_dev *dev = NULL;

      ret = copy_from_user(&find, (void *)arg, sizeof(IO_PCI_FIND_t));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPCILINK): error %d from copy_from_user\n",ret));
    return(-EFAULT);
      }

      //Find a free slot in the pcidev array
      //MJ: for now I am not checking if the same device has already been opened by an other call
      dnum = -1;
//MJ-SMP: to be protected (spinlock)
      for(loop = 0; loop < IO_MAX_PCI; loop++)
      {
        if (pcidev[loop].pid == 0)
        {
          dnum = loop;
          pcidev[dnum].pid = current->pid;  //Reserve this array element
          pptr->linked[dnum] = 1;           //Remember to which "file" this mapping belongs to
          break;
        }
      }
//MJ-SMP: end of protected zone

      if (dnum == -1)
      {
        kerror(("io_rcc(ioctl,IOPCILINK): Device table is full\n"));
        return(-IO_PCI_TABLEFULL);
      }

      for(loop = 0; loop < find.occ; loop++)
      {
        dev = pci_get_device(find.vid, find.did, dev);  //Find N-th device  //MJ: for 2.6 p313
        if (dev == NULL)
        {
          kerror(("io_rcc(ioctl,IOPCILINK): No device found\n"));
          pcidev[dnum].pid = 0;                         //array element no longer required
          pptr->linked[dnum] = 0;
          return(-IO_PCI_NOT_FOUND);
        }
      }

      pci_dev_put(dev);          //MJ: for 2.6 p314
      ret = pci_enable_device(dev);    //MJ: for 2.6 p314
      if (ret)
      {
        kerror(("io_rcc(ioctl,IOPCILINK): error from pci_enable_device\n"));
        return(-EFAULT);
      }

      kdebug(("io_rcc(ioctl,IOPCILINK):Device found\n"));
      kdebug(("io_rcc(ioctl,IOPCILINK):devfn =0x%08x\n", dev->devfn));
      kdebug(("io_rcc(ioctl,IOPCILINK):irq   =0x%08x\n", dev->irq));

      pcidev[dnum].vid     = find.vid;
      pcidev[dnum].did     = find.did;
      pcidev[dnum].occ     = find.occ;
      pcidev[dnum].dev_ptr = dev;

      find.handle = dnum;

      ret = copy_to_user((void *)arg, &find, sizeof(IO_PCI_FIND_t));
      if (ret)
      {
        kerror(("io_rcc(ioctl,IOPCILINK): error from copy_to_user\n"));
        return(-EFAULT);
      }
      break;
    }

    case IOPCIINFO:
    {
      int ret, loop;
      pci_info_t info;

      ret = copy_from_user(&info, (void *)arg, sizeof(pci_info_t));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPCIINFO): error %d from copy_from_user\n", ret));
    return(-EFAULT);
      }

      kdebug(("io_rcc(ioctl,IOPCIINFO): Info requested for handle %d\n", info.handle));

      for (loop = 0; loop < 6; loop++)
      {
        info.bar[loop].base  = pci_resource_start(pcidev[info.handle].dev_ptr, loop);
        info.bar[loop].size  = pci_resource_len(pcidev[info.handle].dev_ptr, loop);
        info.bar[loop].flags = pci_resource_flags(pcidev[info.handle].dev_ptr, loop);
        kdebug(("io_rcc(ioctl,IOPCIINFO): BAR %d:\n", loop));
        kdebug(("io_rcc(ioctl,IOPCIINFO): base  = 0x%08x:\n", info.bar[loop].base));
        kdebug(("io_rcc(ioctl,IOPCIINFO): end   = 0x%08x:\n", info.bar[loop].size));
        kdebug(("io_rcc(ioctl,IOPCIINFO): flags = 0x%08x:\n", info.bar[loop].flags));
      }

      ret = copy_to_user((void *)arg, &info, sizeof(pci_info_t));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPCIINFO): error %d from copy_to_user\n", ret));
    return(-EFAULT);
      }

      break;
    }

    case IOPCIUNLINK:
    {
      u_int dnum;
      int ret;

      ret = copy_from_user(&dnum, (void *)arg, sizeof(int));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPCIUNLINK): error %d from copy_from_user\n",ret));
    return(-EFAULT);
      }

      if (pcidev[dnum].pid == 0)
      {
        kerror(("io_rcc(ioctl,IOPCICONFR): Illegal handle\n"));
    return(-IO_PCI_ILL_HANDLE);
      }

      pcidev[dnum].pid = 0;
      pptr->linked[dnum] = 0;

      break;
    }

    case IOPCICONFR:
    {
      int ret;
      IO_PCI_CONF_t params;
      u_int idata;
      u_short sdata;
      u_char cdata;

      ret = copy_from_user(&params, (void *)arg, sizeof(IO_PCI_CONF_t));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPCICONFR): error %d from copy_from_user\n",ret));
    return(-EFAULT);
      }

      if (pcidev[params.handle].pid == 0)
      {
        kerror(("io_rcc(ioctl,IOPCICONFR): Illegal handle\n"));
    return(-IO_PCI_ILL_HANDLE);
      }

      if (params.size == 4)
      {
        ret = pci_read_config_dword(pcidev[params.handle].dev_ptr, params.offs, &idata);
        params.data = idata;
      }
      if (params.size == 2)
      {
        ret = pci_read_config_word(pcidev[params.handle].dev_ptr, params.offs, &sdata);
        params.data = sdata;
      }
      if (params.size == 1)
      {
        ret = pci_read_config_byte(pcidev[params.handle].dev_ptr, params.offs, &cdata);
        params.data = cdata;
      }

      if (ret)
      {
        kerror(("io_rcc(ioctl,IOPCICONFR): ERROR from pci_read_config_xxx\n"));
    return(-IO_PCI_CONFIGRW);
      }

      ret = copy_to_user((void *)arg, &params, sizeof(IO_PCI_CONF_t));
      if (ret)
      {
        kerror(("io_rcc(ioctl,IOPCICONFR): error from copy_to_user\n"));
        return(-EFAULT);
      }

      break;
    }

    case IOPCICONFW:
    {
      int ret;
      IO_PCI_CONF_t params;

      ret = copy_from_user(&params, (void *)arg, sizeof(IO_PCI_CONF_t));
      if (ret)
      {
    kerror(("io_rcc(ioctl,IOPCICONFW): error %d from copy_from_user\n",ret));
    return(-EFAULT);
      }

      if (pcidev[params.handle].pid == 0)
      {
        kerror(("io_rcc(ioctl,IOPCICONFW): Illegal handle\n"));
    return(-IO_PCI_ILL_HANDLE);
      }

      if (params.size == 4)
        ret = pci_write_config_dword(pcidev[params.handle].dev_ptr, params.offs, params.data);
      if (params.size == 2)
        ret = pci_write_config_dword(pcidev[params.handle].dev_ptr, params.offs, params.data & 0xffff);
      if (params.size == 1)
        ret = pci_write_config_dword(pcidev[params.handle].dev_ptr, params.offs, params.data & 0xff);

      if (ret)
      {
        kerror(("io_rcc(ioctl,IOPCICONFW): ERROR from pci_write_config_xxxx\n"));
    return(-IO_PCI_CONFIGRW);
      }

      break;
    }
  }
  return(0);
}


/****************************************************/
static void io_rcc_vmaOpen(struct vm_area_struct *vma)
/****************************************************/
{
  kdebug(("io_rcc_vmaOpen: Called\n"));
}


/*****************************************************/
static void io_rcc_vmaClose(struct vm_area_struct *vma)
/*****************************************************/
{
  kdebug(("io_rcc_vmaClose: mmap released\n"));
}


/*******************************************************************/
static int io_rcc_mmap(struct file *file, struct vm_area_struct *vma)
/*******************************************************************/
{
  u_long size, offset;

  //vma->vm_flags |= VM_RESERVED;  // This memory must not be swapped out

  kdebug(("io_rcc_mmap: vma->vm_end       = 0x%016lx\n", (u_long)vma->vm_end));
  kdebug(("io_rcc_mmap: vma->vm_start     = 0x%016lx\n", (u_long)vma->vm_start));
  kdebug(("io_rcc_mmap: vma->vm_pgoff     = 0x%016lx\n", (u_long)vma->vm_pgoff));
  kdebug(("io_rcc_mmap: vma->vm_flags     = 0x%08x\n", (u_int)vma->vm_flags));
  kdebug(("io_rcc_mmap: PAGE_SHIFT        = 0x%016lx\n", (u_long)PAGE_SHIFT));
  kdebug(("io_rcc_mmap: PAGE_SIZE         = 0x%016lx\n", (u_long)PAGE_SIZE));

  size = vma->vm_end - vma->vm_start;
  offset = vma->vm_pgoff << PAGE_SHIFT;

  kdebug(("io_rcc_mmap: size                  = 0x%016lx\n", size));
  kdebug(("io_rcc_mmap: physical base address = 0x%016lx\n", offset));

  if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot))
  {
    kerror(("io_rcc_mmap: function remap_pfn_range failed \n"));
    return(-IO_PCI_REMAP);
  }
  kdebug(("io_rcc_mmap: remap_pfn_range OK, vma->vm_start = 0x%016lx\n", (u_long)vma->vm_start));

  vma->vm_ops = &io_rcc_vm_ops;

/*
  This code works for 32-bit Linux

  vma->vm_flags |= VM_RESERVED;  // This memory must not be swapped out
  vma->vm_flags |= VM_LOCKED;    // We also want to lock it   //MJ: for 2.6: this flag still exists but Rubini does not mention it

  kdebug(("io_rcc_mmap: vma->vm_end    = 0x%016lx\n", (u_long)vma->vm_end));
  kdebug(("io_rcc_mmap: vma->vm_start  = 0x%016lx\n", (u_long)vma->vm_start));
  kdebug(("io_rcc_mmap: vma->vm_offset = 0x%016lx\n", (u_long)vma->vm_pgoff));
  kdebug(("io_rcc_mmap: PAGE_SHIFT     = 0x%016lx\n", (u_long)PAGE_SHIFT));
  kdebug(("io_rcc_mmap: vma->vm_flags  = 0x%08x\n", (u_int)vma->vm_flags));

  size = vma->vm_end - vma->vm_start;
  offset = vma->vm_pgoff << PAGE_SHIFT;

  kdebug(("io_rcc_mmap: size   = 0x%016lx\n", size));
  kdebug(("io_rcc_mmap: offset = 0x%016lx\n", offset));

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
  if (remap_page_range(vma, vma->vm_start, offset, size, vma->vm_page_prot))
  {
    kerror(("io_rcc_mmap: function remap_page_range failed \n"));
    return(-IO_PCI_REMAP);
  }
  kdebug(("io_rcc_mmap: remap_page_range OK, vma->vm_start(2) = 0x%016lx\n", (u_long)vma->vm_start));
#else
  if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot))
  {
    kerror(("io_rcc_mmap: function remap_pfn_range failed \n"));
    return(-IO_PCI_REMAP);
  }
  kdebug(("io_rcc_mmap: remap_pfn_range OK, vma->vm_start(2) = 0x%016lx\n", (u_long)vma->vm_start));
#endif

  vma->vm_ops = &io_rcc_vm_ops;
*/
  return(0);
}


/********************************************************************************************************/
static ssize_t io_rcc_proc_write(struct file *file, const char *buffer, size_t count, loff_t *startOffset)
/********************************************************************************************************/
{
  int len;
  char value[100];

  kdebug(("io_rcc_mmap(io_rcc_write_procmem): io_rcc_write_procmem called\n"));

  if(count > 99)
    len = 99;
  else
    len = count;

  if (copy_from_user(value, buffer, len))
  // if (copy_from_user(fb_data->value, buffer, len))
  {
    kerror(("io_rcc_mmap(io_rcc_write_procmem): error from copy_from_user\n"));
    return(-EFAULT);
  }

  kdebug(("io_rcc(io_rcc_proc_write): len = %d\n", len));
  value[len - 1] = '\0';
  kdebug(("io_rcc(io_rcc_proc_write): text passed = %s\n", value));

  if (!strcmp(value, "debug"))
  {
    debug = 1;
    kdebug(("io_rcc(io_rcc_proc_write): debugging enabled\n"));
  }

  if (!strcmp(value, "nodebug"))
  {
    kdebug(("io_rcc(io_rcc_proc_write): debugging disabled\n"));
    debug = 0;
  }

  if (!strcmp(value, "elog"))
  {
    kdebug(("io_rcc(io_rcc_proc_write): error logging enabled\n"))
    errorlog = 1;
  }

  if (!strcmp(value, "noelog"))
  {
    kdebug(("io_rcc(io_rcc_proc_write): error logging disabled\n"))
    errorlog = 0;
  }

  return len;
}


/*************************************************************************************************/
static ssize_t io_rcc_read_procmem(struct file *file, char *buf, size_t count, loff_t *startOffset)
/*************************************************************************************************/
{
  int loop, nchars = 0;
  static int len = 0;
  int offset = *startOffset;

  kdebug(("io_rcc_read_procmem: Called with buf    = 0x%08lx\n", (u_long)buf));
  // kdebug(("io_rcc_read_procmem: Called with *start = 0x%08lx\n", (u_long)*start));
  kdebug(("io_rcc_read_procmem: Called with offset = %ld\n", (u_long)offset));
  kdebug(("io_rcc_read_procmem: Called with count  = %d\n", (int)count));

  if (offset == 0)
  {
    kdebug(("io_rcc_read_procmem: Creating text....\n"));
    len = 0;
    len += sprintf(proc_read_text + len, "IO RCC driver for release %s (based on tag %s)\n", RELEASE_NAME, CVSTAG);
    len += sprintf(proc_read_text + len, "Dumping table of linked devices\n");
    len += sprintf(proc_read_text + len, "Handle | Vendor ID | Device ID | Occurrence | Process ID\n");
    for(loop = 0; loop < IO_MAX_PCI; loop++)
    {
      if (pcidev[loop].pid != 0)
      {
      len += sprintf(proc_read_text + len, "    %2d |", loop);
      len += sprintf(proc_read_text + len, "0x%08x |", pcidev[loop].vid);
      len += sprintf(proc_read_text + len, "0x%08x |", pcidev[loop].did);
      len += sprintf(proc_read_text + len, " 0x%08x |", pcidev[loop].occ);
      len += sprintf(proc_read_text + len, " 0x%08x\n", pcidev[loop].pid);
      }
    }

    len += sprintf(proc_read_text + len, " \n");
    len += sprintf(proc_read_text + len, "The command 'echo <action> > /proc/io_rcc', executed as root,\n");
    len += sprintf(proc_read_text + len, "allows you to interact with the driver. Possible actions are:\n");
    len += sprintf(proc_read_text + len, "debug   -> enable debugging\n");
    len += sprintf(proc_read_text + len, "nodebug -> disable debugging\n");
    len += sprintf(proc_read_text + len, "elog    -> Log errors to /var/log/messages\n");
    len += sprintf(proc_read_text + len, "noelog  -> Do not log errors to /var/log/messages\n");

    len += sprintf(proc_read_text + len, "Current values of the parameter(s)\n");
    len += sprintf(proc_read_text + len, "debug    = %d\n", debug);
    len += sprintf(proc_read_text + len, "errorlog = %d\n", errorlog);
  }
  kdebug(("io_rcc_read_procmem: number of characters in text buffer = %d\n", len));

  if (count < (len - offset))
    nchars = count;
  else
    nchars = len - offset;
  kdebug(("io_rcc_read_procmem: min nchars         = %d\n", nchars));

  if (nchars > 0)
  {
    if (copy_to_user(buf + (offset & (PAGE_SIZE - 1)), proc_read_text + offset, nchars))
    {
      kerror(("io_rcc(cmem_proc_read): error in from copy_to_user\n"));
      return(0);
    }
    *startOffset = len + (offset & (PAGE_SIZE - 1));
  }

  kdebug(("io_rcc_read_procmem: returning nchars   = %d\n", nchars));
  return(nchars);
}








