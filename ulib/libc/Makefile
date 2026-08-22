
TOP_SRCDIR = ../../

include $(TOP_SRCDIR)/Makefile.common

all: libc.a

libc.a:
	echo "-----------------------------------------------------------------"
	$(MAKE) -C string all
	$(MAKE) -C stdio all
	$(MAKE) -C stdlib all
	$(MAKE) -C math all
	$(MAKE) -C sys all
	x86_64-elf-ar -r libc.a stdio/*.o string/*.o stdlib/*.o math/*.o sys/*.o
	x86_64-elf-ranlib libc.a
	echo "-----------------------------------------------------------------"

clean:
	$(MAKE) -C stdio clean
	$(MAKE) -C string clean
	$(MAKE) -C stdlib clean
	$(MAKE) -C math clean
	$(MAKE) -C sys clean
	rm -f libc.a

