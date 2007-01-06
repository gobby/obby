SUBDIRS = po inc src .
if BUILD_TESTS
SUBDIRS += test
endif

# Tell aclocal where to find `.m4' files.
ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = obby-0.2.pc

# gettext
EXTRA_DIST = config.rpath mkinstalldirs

# Fix some permissions if they are not set properly when building the tarball
dist-hook:
	chmod u+x $(distdir)/ltmain.sh $(distdir)/missing \
	          $(distdir)/install-sh $(distdir)/config.guess \
	          $(distdir)/config.sub $(distdir)/config.rpath \
	          $(distdir)/depcomp $(distdir)/mkinstalldirs

