TARGET = vshctrl
OBJS = main.o virtualpbpmgr.o isofs_driver/isofs_driver.o isofs_driver/umd9660_driver.o isofs_driver/csoread.o exports.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_kernel -lpsppower_driver -lpsprtc_driver -lpspusb_driver

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak