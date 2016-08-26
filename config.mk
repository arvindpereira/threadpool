CC	=	cc
CFLAGS	=	-D_REENTRANT -O
LFLAGS	=	-lpthread
AR	=	ar rc
RANLIB	=	ranlib

ifndef OS
OS	=	$(shell uname -s)
endif

ifeq ($(OS),Linux)
CC	=	g++
CFLAGS	=	-D_REENTRANT -O2 -Wall -Wno-long-long -Wshadow -Wpacked -Wundef -Wchar-subscripts -Wold-style-cast -Woverloaded-virtual -pedantic -ansi
endif

ifeq ($(OS),Darwin)
CC	=	g++
CFLAGS	=	-D_REENTRANT -O2 -Wall -Wno-long-long -Wshadow -Wpacked -Wundef -Wchar-subscripts -Wold-style-cast -Woverloaded-virtual -pedantic -ansi
endif

ifeq ($(OS),SunOS)
CC	=	CC 
CFLAGS	=	-mt -fast -xarch=v8plusa -xO5
LFLAGS	=	-xarch=v8plusa -lpthread -lrt 
AR	=	CC -xarch=v8plusa -mt -xar -o
endif

ifeq ($(OS),AIX)
CC	=	xlC
CFLAGS	=	-O2 -qinline -qrtti
endif

ifeq ($(OS),HP-UX)
CC	=	aCC
CFLAGS	=	-mt -AA -fast
endif

