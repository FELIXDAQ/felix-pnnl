KERNEL_VERSION := $(shell uname -r)
KERNEL_MODLIB  := /lib/modules/$(KERNEL_VERSION)
KERNEL_SOURCES := $(shell test -d $(KERNEL_MODLIB)/source && echo $(KERNEL_MODLIB)/source || echo $(KERNEL_MODLIB)/build)

CPPFLAGS      += -I$(PWD)/../flx -I$(PWD)/.. -DBUILD_64_BIT -DCOMPILE_DRIVER -DDONT_USE_BPA -DDRIVER_ERROR -DDRIVER_DEBUG -DRELEASE_NAME=\"${RELEASE_NAME}\" -DCVSTAG=\"$(CVSTAG)\"
EXTRA_CFLAGS  := -I$(PWD)/../flx -I$(PWD)/.. -DBUILD_64_BIT -DCOMPILE_DRIVER -DDONT_USE_BPA -DDRIVER_ERROR -DDRIVER_DEBUG -DRELEASE_NAME=\"${RELEASE_NAME}\" -DCVSTAG=\"$(CVSTAG)\" -DFLX_TAG=\"$(FLX_TAG)\" 

RELEASE_NAME := "tdaq611_for_felix"
CVSTAG       := "undefined"
FLX_TAG      := "Markus_code"


obj-m   := cmem_rcc.o io_rcc.o flx.o
KDIR	:= $(KERNEL_SOURCES)
PWD	:= $(shell pwd)
default:

	echo "$(KERNEL_SOURCES) " $(KERNEL_SOURCES)
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) RELEASE_NAME="\"$(RELEASE_NAME)\"" CVSTAG="\"$(CVSTAG)\"" FLX_TAG="\"$(FLX_TAG)\"" modules
	mkdir -p ../drivers 
	mv cmem_rcc.ko ../drivers/
	mv io_rcc.ko ../drivers/
	mv flx.ko ../drivers/
clean:
	$(RM) cmem_rcc.ko.unsigned cmem_rcc.mod.c cmem_rcc.mod.o cmem_rcc.o modules.order Module.symvers .cmem_rcc* .tmp_versions/*



