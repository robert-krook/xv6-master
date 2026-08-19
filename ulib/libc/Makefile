
TOP_SRCDIR = ../../

include $(TOP_SRCDIR)/Makefile.common

all: lib2.a

lib2.a:
	echo "-----------------------------------------------------------------"
	$(MAKE) -C string all
	$(MAKE) -C stdio all
	$(MAKE) -C stdlib all
	x86_64-elf-ar -r libc2.a stdio/*.o string/*.o stdlib/*.o
	x86_64-elf-ranlib libc2.a
	echo "-----------------------------------------------------------------"

clean:
	$(MAKE) -C stdio clean
	$(MAKE) -C string clean
	$(MAKE) -C stdlib clean
