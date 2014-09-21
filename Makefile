################################################################################
# GNU make should only be needed in maintainer mode now
################################################################################
# Please run 'configure' first (generate it with autogen.sh)
################################################################################

SHELL=/bin/bash

srcdir     = .
top_srcdir = .

PACKAGE = icewm
VERSION = 1.3.8

PREFIX = /usr
BINDIR = /usr/X11R6/bin
LIBDIR = /usr/share/icewm
CFGDIR = /etc/icewm
LOCDIR = /usr/share/locale
KDEDIR = /usr/share
DOCDIR = /usr/share/doc
MANDIR = /usr/share/man

EXEEXT = 

INSTALL    = /bin/install -c
INSTALLDIR = /bin/install -c -m 755 -d
INSTALLBIN = ${INSTALL}
INSTALLLIB = ${INSTALL} -m 644
INSTALLETC = ${INSTALL} -m 644
INSTALLMAN = ${INSTALL} -m 644
MKFONTDIR  = /bin/mkfontdir

DESTDIR     =

################################################################################

BINFILES    = $(top_srcdir)/src/icewm$(EXEEXT) $(top_srcdir)/src/icewm-session$(EXEEXT) $(top_srcdir)/src/icesh$(EXEEXT) $(top_srcdir)/src/icewmhint$(EXEEXT) $(top_srcdir)/src/icewmbg$(EXEEXT) $(top_srcdir)/src/icewmtray$(EXEEXT) $(top_srcdir)/src/icehelp$(EXEEXT)  icewm-set-gnomewm
LIBFILES    = lib/preferences lib/winoptions lib/keys \
              lib/menu lib/toolbar # lib/programs
DOCFILES    = README BUGS CHANGES COPYING AUTHORS INSTALL VERSION icewm.lsm
MANFILES    = icewm.1
XPMDIRS     = icons ledclock taskbar mailbox cursors
THEMES      = nice motif win95 warp3 warp4 metal2 gtk2 Infadel2 nice2 \
              icedesert yellowmotif

all:		base nls
install:	install-base install-nls 

base icesound icehelp:
	@cd src && $(MAKE) $@

docs:
	@cd doc && $(MAKE) all

nls:
	@cd po && $(MAKE) all

srcclean:
	@cd src && $(MAKE) clean

clean:  srcclean
	@cd doc && $(MAKE) clean

distclean:
	@-$(MAKE) clean
	rm -f *~ config.cache config.log config.status install.inc \
	sysdep.inc src/config.h \
	lib/preferences \
	lib/menu lib/programs lib/keys lib/winoptions lib/toolbar

maintainer-clean: distclean
	rm -f icewm.spec icewm.lsm Makefile configure src/config.h.in
	@cd doc && $(MAKE) maintainer-clean

check:
	@cd src && $(MAKE) check >/dev/null

dist:	distclean docs configure

# Makefile TABS *SUCK*
install-base: base
	@echo ------------------------------------------
	@echo "Installing binaries in $(DESTDIR)$(BINDIR)"
	@$(INSTALLDIR) "$(DESTDIR)$(BINDIR)"
	@for bin in $(BINFILES); do \
             $(INSTALLBIN) "$${bin}" "$(DESTDIR)$(BINDIR)"; \
         done
	
	@echo "Installing presets and icons in $(DESTDIR)$(LIBDIR)"
	@$(INSTALLDIR) "$(DESTDIR)$(LIBDIR)"
	#-@$(INSTALLDIR) "$(DESTDIR)$(CFGDIR)"
	@for lib in $(LIBFILES); do \
             $(INSTALLLIB) "$${lib}" "$(DESTDIR)$(LIBDIR)"; \
         done

	@for xpmdir in $(XPMDIRS); do \
	     if test -d "lib/$${xpmdir}"; then \
		$(INSTALLDIR) "$(DESTDIR)$(LIBDIR)/$${xpmdir}"; \
		for pixmap in "lib/$${xpmdir}/"*.xpm; do \
		    $(INSTALLLIB) "$${pixmap}" "$(DESTDIR)$(LIBDIR)/$${xpmdir}"; \
		done; \
	    fi; \
        done
	
	@echo ------------------------------------------
	@for theme in $(THEMES); do \
	     SRCDIR="$(top_srcdir)" \
	     DESTDIR="$(DESTDIR)" \
	     LIBDIR="$(LIBDIR)" \
	     XPMDIRS="$(XPMDIRS)" \
	     INSTALLDIR="$(INSTALLDIR)" \
	     INSTALLLIB="$(INSTALLLIB)" \
	     MKFONTDIR="$(MKFONTDIR)" \
	     $(top_srcdir)/utils/install-theme.sh "$${theme}"; \
	done
	@#for a in $(ETCFILES) ; do $(INSTALLETC) "$$a" $(CFGDIR) ; done
	@echo ------------------------------------------

install-docs: docs
	@echo ------------------------------------------
	@rm -fr "$(DESTDIR)$(DOCDIR)/icewm-$(VERSION)"
	@$(INSTALLDIR) "$(DESTDIR)$(DOCDIR)/icewm-$(VERSION)"
	@echo "Installing documentation in $(DESTDIR)$(DOCDIR)"
	@$(INSTALLLIB) $(DOCFILES) "$(DESTDIR)$(DOCDIR)/icewm-$(VERSION)"
	@$(INSTALLLIB) "$(top_srcdir)/doc/"*.sgml "$(DESTDIR)$(DOCDIR)/icewm-$(VERSION)"
	@$(INSTALLLIB) "$(top_srcdir)/doc/"*.html "$(DESTDIR)$(DOCDIR)/icewm-$(VERSION)"
	@echo ------------------------------------------

install-nls: nls
	@echo ------------------------------------------
	@cd po && $(MAKE) install
	@echo ------------------------------------------

install-man:
	@$(INSTALLDIR) "$(DESTDIR)$(MANDIR)/man1"
	@for man in $(MANFILES); do \
		$(INSTALLMAN) doc/$$man.man $(DESTDIR)$(MANDIR)/man1/$$man; \
	done

install-desktop:
	@echo ------------------------------------------
	@$(INSTALLDIR) "$(DESTDIR)/usr/share/xsessions"
	@$(INSTALLDIR) "$(DESTDIR)/usr/share/applications"
	@$(INSTALLLIB) "$(top_srcdir)/lib/icewm-session.desktop" "$(DESTDIR)/usr/share/xsessions/icewm-session.desktop"
	@$(INSTALLLIB) "$(top_srcdir)/lib/icewm.desktop" "$(DESTDIR)/usr/share/applications/icewm.desktop"
	@echo ------------------------------------------