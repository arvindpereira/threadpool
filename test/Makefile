
include ../config.mk

SOURCES = TArray.cc TSLL.cc TThread.cc TThreadPool.cc TArray.hh TSLL.hh TThread.hh TThreadPool.hh
OBJECTS = TThread.o TThreadPool.o

%.o:	%.cc
	$(CC) -c $(CFLAGS) -I../src $< -o $@ 

all: thrtest

thrtest:	../libthrpool.a TRNG.o TRNG.cc TRNG.hh TTimer.o TTimer.cc TTimer.hh main.cc main.o
	$(CC) $(CFLAGS) -o thrtest main.o TRNG.o TTimer.o ../libthrpool.a $(LFLAGS)

clean:
	rm -rf *.o *~ thrtest

