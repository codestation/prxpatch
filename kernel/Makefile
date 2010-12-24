TARGET = mhp3patch
OBJS =  main.o libs.o imports.o exports.o bootsplash.o
# logger.o
LIBS = -lpspsystemctrl_kernel -lpspge
CFLAGS = -Os -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS = -L.

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
