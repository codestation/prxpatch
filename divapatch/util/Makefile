CXX := g++
TARGET := divaconvert
LIBS :=
INCLUDES :=
LIBPATH :=
WARN_FLAGS=-Wall -Wextra -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wconversion -Wunreachable-code
CXXFLAGS := -O2 -pipe -Wno-unused-result $(WARN_FLAGS)
LDFLAGS := -Wl,-O1 -Wl,--sort-common -Wl,--as-needed
RM = rm -f
OBJS = convert.o

all: $(OBJS)
	@$(CXX) $(LDFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -o $(TARGET)
	@echo LINK $(TARGET)

.cpp.o:
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $*.cpp
	@echo CC $*.cpp

clean:
	@-$(RM) $(OBJS) $(TARGET)
	@echo CLEAN $(TARGET)

.PHONY: all clean
