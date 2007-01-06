APPS = record buffer bufferd

all: $(APPS)

record: record.cpp
	g++ record.cpp -L../.libs -o record `pkg-config --cflags --libs lobby-1.0 howl`

buffer: buffer.cpp
	g++ buffer.cpp -L../.libs -o buffer `pkg-config --cflags --libs lobby-1.0 howl` -DCLIENT_TEST

bufferd: buffer.cpp
	g++ buffer.cpp -L../.libs -o bufferd `pkg-config --cflags --libs lobby-1.0 howl` -DSERVER_TEST

clean:
	rm -f $(APPS)
