# These tests are now self-contained and link against the libtool library
# in ../.libs, so they compile without libobby being installed.
#
# This has two drawbacks:
#  1) You are not reminded to ``make install'' the library before there are
#     new functions available not only for the tests.
#  2) libtool will create some strange wrapper scripts around the actual
#     tests, but they are not installed anyway.
LDADD = -L../.libs -lobby $(libobby_LIBS)
INCLUDES = -I../inc/ $(libobby_CFLAGS)

noinst_PROGRAMS = \
	record \
	buffer \
	bufferd \
	2buffer \
	curses
#	discover

record_SOURCES = record.cpp
buffer_SOURCES = buffer.cpp
bufferd_SOURCES = bufferd.cpp
2buffer_SOURCES = 2buffer.cpp
#discover_SOURCES = discover.cpp

curses_SOURCES = curses.cpp
curses_LDADD = $(LDADD) -lncurses

