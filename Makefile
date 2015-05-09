
##	     Copyright 2015 Johann 'Myrkraverk' Oskarsson
##		       <johann@myrkraverk.com>

## Heavily edited sample Makefile.

XEMACS_BIN=`xemacs -batch -eval '(princ invocation-directory)'`
XEMACS_SYSTEM=`xemacs -batch -eval '(princ exec-directory)'`
XEMACS_MODULE_DIRECTORY=`xemacs -batch -eval '(princ module-directory)'`
RM=rm -f
CP=cp
ELLCC=${XEMACS_BIN}/ellcc
CFLAGS=-I. -I${XEMACS_SYSTEM}/include `curl-config --cflags`
LDFLAGS=`curl-config --libs`
LD=$(ELLCC) --mode=link
MKINIT=$(ELLCC) --mode=init
SRCS=curl.c
OBJS=$(SRCS:.c=.o)

.c.o:
	$(ELLCC) --mode=verbose $(CFLAGS) -c $<

MODNAME=curl
MODVER=7.41.0
MODTITLE="libcurl bindings for XEmacs"

all: $(MODNAME).ell

distclean: clean

clean:
	$(RM) $(MODNAME).ell $(OBJS) $(MODNAME)_docs.o $(MODNAME)_docs.c

install: all
	$(CP) $(MODNAME).ell $(XEMACS_MODULE_DIRECTORY)

uninstall:
	$(RM) $(XEMACS_MODULE_DIRECTORY)/$(MODNAME).ell

$(MODNAME).ell: $(OBJS) $(MODNAME)_docs.o
	$(LD) $(LDFLAGS) --mode=verbose --mod-output=$@ $(OBJS) \
	$(MODNAME)_docs.o

$(MODNAME)_docs.o: $(MODNAME)_docs.c
$(MODNAME)_docs.c: $(SRCS)
	ELLMAKEDOC=${XEMACS_SYSTEM}/make-docfile $(MKINIT) --mod-output=$@ \
	--mod-name=$(MODNAME) --mod-version=$(MODVER) \
	--mod-title=$(MODTITLE) $(SRCS)

