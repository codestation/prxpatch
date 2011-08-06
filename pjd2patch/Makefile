TARGET = pjd2patch
OBJS =  main.o exports.o imports.o logger.o

all:
	psp-packer pjd2patch.prx

LIBS =
WARN_FLAGS=-Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Wunreachable-code
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
