## Process this file with automake to produce Makefile.in
##
## $Id: Makefile.am,v 1.1.1.1 2009/10/24 17:48:09 reynolds Exp $

bin_PROGRAMS = signature
signature_SOURCES = signature.c signature.h format.c format.h \
		    quote.c quote.h
man_MANS = signature.1
EXTRA_DIST = $(man_MANS) signature.spec signature.lsm

# Plenty of compiler warnings
CFLAGS = @CFLAGS@ -Wall -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align \
	 -Wwrite-strings -Waggregate-return -Wstrict-prototypes \
	 -Wmissing-prototypes -Wmissing-declarations -Wnested-externs \
	 -pedantic -ansi

signature.ps:	$(srcdir)/signature.1
		groff -Tps -man $(srcdir)/signature.1 > $(srcdir)/signature.ps

rpm:		distcheck
		rpm -ta --clean --sign $(srcdir)/signature-$(VERSION).tar.gz
