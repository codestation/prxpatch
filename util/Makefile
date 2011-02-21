CC := gcc
TARGET := pjd2convert
LIBS :=
INCLUDES :=
LIBPATH :=
CFLAGS := -O2 -pipe -Wall -Wno-unused-result
LDFLAGS := -Wl,-O1 -Wl,--sort-common -Wl,--as-needed
RM = rm -f
OBJS = convert.o

all: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -o $(TARGET)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $*.c

clean:
	-$(RM) $(OBJS) $(TARGET)

.PHONY: all clean
