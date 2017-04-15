release: all
	psp-fixup-imports -m nid_map.txt $(TARGET).prx

TARGET = systemctrl
OBJS = main.o adrenaline.o executable_patch.o libraries_patch.o io_patch.o init_patch.o power_patch.o utility_patch.o sysmodpatches.o \
	   nodrm.o conf.o malloc.o kubridge.o systemctrl.o LflashFatfmt.o string_clone.o setjmp_clone.o imports.o exports.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak