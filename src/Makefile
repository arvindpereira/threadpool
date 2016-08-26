
include ../config.mk

SOURCES = TThread.cc TThreadPool.cc TThread.hh TThreadPool.hh
OBJECTS = TThread.o TThreadPool.o

%.o:	%.cc
	$(CC) -c $(CFLAGS) $< -o $@ 

all: libthrpool.a

libthrpool.a:	$(SOURCES) $(OBJECTS)
	$(AR) ../libthrpool.a $(OBJECTS)
	$(RANLIB) ../libthrpool.a

clean:
	rm -rf *.o *~ libthrpool.a

