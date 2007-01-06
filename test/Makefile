# Hint: All those tests should not depend on libobby being installed.
#       So they should include the headers with #include "file.hpp" instead
#       of the normal ones and they should link against the library build
#       in the toplevel directory.

APPS = record bufferd discover curses

CFLAGS = -I../inc `pkg-config --cflags ../lobby-1.0.pc`
LIBS = -L../.libs `pkg-config --libs ../lobby-1.0.pc`

all: $(APPS)

record: record.cpp
	g++ record.cpp -o record $(CFLAGS) $(LIBS)

bufferd: bufferd.cpp
	g++ bufferd.cpp -o bufferd $(CFLAGS) $(LIBS)

discover: discover.cpp
	g++ discover.cpp -o discover $(CFLAGS) $(LIBS)

curses: curses.cpp
	g++ curses.cpp -o curses $(CFLAGS) $(LIBS) -lcurses

clean:
	rm -f $(APPS)

