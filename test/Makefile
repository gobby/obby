# Hint: All those tests should not depend on libobby being installed.
#       So they should include the headers with #include "file.hpp" instead
#       of the normal ones and they should link against the library build
#       in the toplevel directory.

APPS = record buffer bufferd

CFLAGS = -I../inc `pkg-config --cflags ../lobby-1.0.pc`
LIBS = -L../.libs `pkg-config --libs ../lobby-1.0.pc`

all: $(APPS)

record: record.cpp
	g++ record.cpp -o record $(CFLAGS) $(LIBS)

buffer: buffer.cpp
	g++ buffer.cpp -o buffer $(CFLAGS) $(LIBS) -DCLIENT_TEST

bufferd: buffer.cpp
	g++ buffer.cpp -o bufferd $(CFLAGS) $(LIBS) -DSERVER_TEST

clean:
	rm -f $(APPS)

