lib_LTLIBRARIES = libobby.la

INCLUDES = $(libobby_CFLAGS) -I../inc/

pkginclude_HEADERS = \
	inc/buffer.hpp \
	inc/client_buffer.hpp \
	inc/delete_record.hpp \
	inc/insert_record.hpp \
	inc/position.hpp \
	inc/record.hpp \
	inc/server_buffer.hpp

libobby_la_SOURCES = \
	src/position.cpp \
	src/record.cpp \
	src/insert_record.cpp \
	src/delete_record.cpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = lobby-1.0.pc

