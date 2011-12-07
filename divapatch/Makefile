TARGET = divapatch
OBJS =  main.o exports.o imports.o logger.o hook.o reader.o search.o utility.o

LIBS =
WARN_FLAGS=-Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Wunreachable-code -Wno-unused-parameter
CFLAGS = -O2 -G0 -std=c99 $(WARN_FLAGS)

ifeq ($(DEBUG), 1)
CFLAGS+=-DDEBUG
endif

ASFLAGS = $(CFLAGS)

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PSP_FW_VERSION=500

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
