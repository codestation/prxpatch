all:
	cd kernel && $(MAKE) -f Makefile all
	cd util && $(MAKE) -f Makefile all

clean:
	cd kernel && $(MAKE) -f Makefile clean
	cd util && $(MAKE) -f Makefile clean