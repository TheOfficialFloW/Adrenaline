release: all
	pack-pbp EBOOT.PBP UPDATE.SFO ICON0.PNG NULL NULL NULL NULL updater.prx NULL

TARGET = updater
OBJS = main.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_user

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak