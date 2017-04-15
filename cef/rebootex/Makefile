TARGET = rebootex
OBJS = main.o libc.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -nostartfiles -T linkfile.x -nostdlib

PSPSDK = $(shell psp-config --pspsdk-path)
include ../lib/build_bin.mak