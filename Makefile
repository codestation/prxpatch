all:
	cd kernel && make -f Makefile all
	cd user && make -f Makefile all

clean:
	cd kernel && make -f Makefile clean
	cd user && make -f Makefile clean
