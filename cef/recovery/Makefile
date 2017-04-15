TARGET = recovery
OBJS = main.o menu.o installer.o plugins.o utils.o pspPSAR.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -Wno-unused-variable
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1

USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_user -lpspkubridge -lpspreg -lz

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak