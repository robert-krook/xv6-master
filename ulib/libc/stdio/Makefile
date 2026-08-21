
TOP_SRCDIR = ../../../

include $(TOP_SRCDIR)/Makefile.common

all: \
	ftell.o printf.o stdio.o sprintf.o
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o
