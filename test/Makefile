APPS = record

all: $(APPS)

record: record.cpp
	g++ record.cpp -L../.libs -o record -lobby -lnet6

clean:
	rm -f $(APPS)
