noinst_PROGRAMS = 
	record \
	buffer \
	bufferd \
	2buffer \
	curses
#	discover

LDADD = $(libobby_LIBS) ../.libs/libobby.a
INCLUDES = $(libobby_CFLAGS) -I../inc/

record_SOURCES = record.cpp
buffer_SOURCES = buffer.cpp
bufferd_SOURCES = bufferd.cpp
2buffer_SOURCES = 2buffer.cpp
#discover_SOURCES = discover.cpp

curses_SOURCES = curses.cpp
curses_LDADD = $(LDADD) -lncurses

