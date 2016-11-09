/************************************************************************/
/*									*/
/* File: tdaq_drivers.h						        */
/*									*/
/* This is the common header file for all tdaq drivers		        */
/*									*/
/* 19. Jun. 06  MAJO  created						*/
/*									*/
/************ C 2006 - The software with that certain something *********/
#ifndef _TDAQ_DRIVERS_H 
#define _TDAQ_DRIVERS_H

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

#endif
