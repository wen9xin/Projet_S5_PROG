CC=$(GCC_CROSS)
CCAS=$(GCC_CROSS)
AM_CFLAGS=-mbig-endian
AM_CCASFLAGS=-mbig-endian
AM_LDFLAGS=-nostdlib -T $(srcdir)/linker_script -n --entry main
AM_LDFLAGS+=-Wl,-EB

if HAVE_ARM_COMPILER
noinst_PROGRAMS=example1 example2 example3 example4 exampleAddrMode2 exampleAddrMode3 exampleDataProcessing exampleAddrMode1 exampleAddrMode2_2 exampleLoadStore insertion_sort

all-am: $(PROGS)
endif

example1_SOURCES=example1.s
example2_SOURCES=example2.s
example3_SOURCES=example3.s
example4_SOURCES=example4.s
exampleDataProcessing_SOURCES=exampleDataProcessing.s
exampleAddrMode1_SOURCES=exampleAddrMode1.s
exampleLoadStore_SOURCES=exampleLoadStore.s
exampleAddrMode2_SOURCES=exampleAddrMode2.s
exampleAddrMode3_SOURCES=exampleAddrMode3.s
exampleAddrMode2_2_SOURCES=exampleAddrMode2_2.s
insertion_sort_SOURCES=insertion_sort.c

EXTRA_DIST=linker_script

clean-local:
	$(RM) $(PROGS)
