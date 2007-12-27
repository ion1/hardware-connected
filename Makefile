BINARIES := hardware-connected

hardware-connected_SRC := hardware-connected.c list.c

include generic.mk

override CFLAGS += -std=gnu99
