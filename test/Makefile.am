noinst_PROGRAMS = record bufferd discover curses

LDADD = $(libobby_LIBS) ../.libs/libobby.a
INCLUDES = $(libobby_CFLAGS) -I../inc/

record_SOURCES = record.cpp
bufferd_SOURCES = bufferd.cpp
discover_SOURCES = discover.cpp

curses_SOURCES = curses.cpp
curses_LDADD = $(LDADD) -lncurses

