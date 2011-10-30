CC := gcc
TARGET := divaconvert
LIBS :=
INCLUDES :=
LIBPATH :=
WARN_FLAGS=-Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Wunreachable-code
CFLAGS := -O2 -pipe -Wno-unused-result $(WARN_FLAGS)
LDFLAGS := -Wl,-O1 -Wl,--sort-common -Wl,--as-needed
RM = rm -f
OBJS = convert.o

all: $(OBJS)
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -o $(TARGET)
	@echo LINK $(TARGET)

.c.o:
	@$(CC) $(CFLAGS) $(INCLUDES) -c $*.c
	@echo CC $*.c

clean:
	@-$(RM) $(OBJS) $(TARGET)
	@echo CLEAN $(TARGET)

.PHONY: all clean
