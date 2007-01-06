# These tests are now self-contained and link against the libtool library
# in ../.libs, so they compile without libobby being installed.
#
# This has two drawbacks:
#  1) You are not reminded to ``make install'' the library before there are
#     new functions available not only for the tests.
#  2) libtool will create some strange wrapper scripts around the actual
#     tests, but they are not installed anyway.
LDADD = -L../src/.libs -lobby $(libobby_LIBS)
AM_CPPFLAGS = -I../inc/ $(libobby_CFLAGS)

if WITH_HOWL
LDADD    += $(libhowl_LIBS)
AM_CPPFLAGS += $(libhowl_CFLAGS)
endif

noinst_PROGRAMS =  record
noinst_PROGRAMS += buffer
noinst_PROGRAMS += bufferd
noinst_PROGRAMS += 2buffer
noinst_PROGRAMS += curses

record_SOURCES = record.cpp
buffer_SOURCES = buffer.cpp
bufferd_SOURCES = bufferd.cpp
2buffer_SOURCES = 2buffer.cpp

curses_SOURCES = curses.cpp
curses_LDADD = $(LDADD) -lncurses

if WITH_HOWL
noinst_PROGRAMS += discover
discover_SOURCES = discover.cpp
endif

