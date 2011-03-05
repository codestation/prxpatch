all:
	cd kernel && $(MAKE) -f Makefile all

clean:
	cd kernel && $(MAKE) -f Makefile clean
