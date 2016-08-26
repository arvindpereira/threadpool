
# 2016-08-25 - Arvind Pereira <arvind.pereira@gmail.com> added a dynamically linked library target.
include ../config.mk

SOURCES = TThread.cc TThreadPool.cc TThread.hh TThreadPool.hh
OBJECTS = TThread.o TThreadPool.o
LDFLAGS	= -shared
SHRTARGET=libthrpool.so
CFLAGS = -fPIC -c

UNAME_S= $(shell uname -s)
ifeq ($(UNAME_S),Linux)
        LDFLAGS += -Wl,-soname,$(SHRTARGET)
endif
ifeq ($(UNAME_S),Darwin)
        CFLAGS += -dynamiclib
        LDFLAGS += -dynamiclib -fPIC
endif

all: libthrpool.a $(SHRTARGET)

libthrpool.a:	$(SOURCES) $(OBJECTS)
	$(AR) ../libthrpool.a $(OBJECTS)
	$(RANLIB) ../libthrpool.a


%.o: %.cc
	$(CC) -C $(CFLAGS) $< -o $@

$(SHRTARGET):	$(SOURCES) $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(SHRTARGET) $(OBJECTS)	
	mv $(SHRTARGET) ../$(SHRTARGET)

clean:
	rm -rf *.o *~ libthrpool.a libthrpool.so

