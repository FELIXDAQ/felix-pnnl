### NOTE:
  -- This folder is maintaned by Andrea Borga (andrea.borga@nikhef.nl)
  -- This is a copy of the sources distributed with the FELIX RPM maintaned by Markus Joos (markus.joos@cern.ch)
  -- Once installed the RPM puts the sources in /usr/src/tdaq_sw_for_Flx-X.Y.Z
  -- In turn the RPM packages the sources of the drivers in https://svnweb.cern.ch/trac/atlastdaq/browser/DAQ/DataFlow


## EC: don't build the drivers, as below, unless you really want to. I don't see the point of the needed Make-ology, since they've given us the .ko's ...
## build the drivers:
$ cd src
$ make

## EC: ... just drop Markus's pre-built cmem_rcc-3.10.0-327.13.1.el7.x86_64.ko and io_rcc-3.10.0-327.13.1.el7.x86_64.ko
## into ./drivers and then do below to install them.


## load the drivers:

$ cd scripts
$ chmod 775 drivers_flx_local
$ sudo ./drivers_flx_local start 

## make the libraries available:

if you don't have access to AFS or you didn't install the drivers from RPM

$ export LD_LIBRARY_PATH=/path_to_the_svn_checkout/software/drivers_rcc/lib64

otherwise the libraries will be available at /usr/lib64 (or AFS)

## content of the drivers_rcc folder:

.
├── cmem_rcc
│   ├── cmem_rcc_common.h
│   ├── cmem_rcc_drv.h
│   └── cmem_rcc.h
├── DFDebug
│   ├── DFDebug.h
│   └── GlobalDebugSettings.h
├── drivers
├── flx
│   └── flx_common.h
├── io_rcc
│   ├── io_rcc_common.h
│   ├── io_rcc_driver.h
│   └── io_rcc.h
├── lib64
│   ├── libcmem_rcc.so
│   ├── libDFDebug.so
│   ├── libgetinput.so
│   ├── libio_rcc.so
│   └── librcc_error.so
├── rcc_error
│   └── rcc_error.h
├── README.txt
├── ROSGetInput
│   └── get_input.h
├── ROSRCDdrivers
│   └── tdaq_drivers.h
├── script
│   └── drivers_flx
│   └── drivers_flx_local
└── src
    ├── cmem_rcc.c
    ├── flx.c
    ├── io_rcc.c
    └── Makefile

For more detailed documentation please refere to:
  https://espace.cern.ch/ATLAS-FELIX/Shared%20Documents/SW_Documentation/FLX_support_SW.docx?Web=1
