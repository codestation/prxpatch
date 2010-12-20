TARGET  = mhp3patch_user
OBJS    = main.o sceio.o mhp3kernel.o exports.o
# logger.o
LIBS    = 
CFLAGS = -Os -G0 -Wall
CXXFLAGS = $(CFLAGS)
ASFLAGS = $(CFLAGS)
LDFLAGS = -L.

USE_PSPSDK_LIBC = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak