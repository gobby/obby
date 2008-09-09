INCLUDES = -I$(top_srcdir)/inc

noinst_LTLIBRARIES = libserialise.la
libserialise_la_CPPFLAGS = $(libobby_CFLAGS) @extra_includes@
libserialise_la_LIBADD = $(libobby_LIBS) $(LIBINTL) @extra_libraries@

libserialise_la_SOURCES = error.cpp
libserialise_la_SOURCES += token.cpp
libserialise_la_SOURCES += attribute.cpp
libserialise_la_SOURCES += object.cpp
libserialise_la_SOURCES += parser.cpp

