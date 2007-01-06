lib_LTLIBRARIES = libobby.la

INCLUDES = $(libobby_CFLAGS) -I../inc/

pkginclude_HEADERS = \
	inc/position.hpp \
	inc/record.hpp \
	inc/delete_record.hpp \
	inc/insert_record.hpp \
	inc/buffer.hpp \
	inc/client_buffer.hpp \
	inc/server_buffer.hpp \
	inc/rendezvous.hpp

libobby_la_SOURCES = \
	src/common.cpp \
	src/position.cpp \
	src/record.cpp \
	src/insert_record.cpp \
	src/delete_record.cpp \
	src/buffer.cpp \
	src/client_buffer.cpp \
	src/server_buffer.cpp \
	src/rendezvous.cpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = lobby-1.0.pc

