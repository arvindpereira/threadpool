# 2016-08-25 Arvind Pereira <arvind.pereira@gmail.com> added libthrpool.so target to clean.

all:
	$(MAKE) -C src
	$(MAKE) -C test

clean:
	$(MAKE) -C src clean
	$(MAKE) -C test clean
	@rm -f libthrpool.a libthrpool.so

