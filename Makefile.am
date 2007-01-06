lib_LTLIBRARIES = libobby.la

#  current  : revision :      age
#     ^     :     ^    :       ^
#     |     :   minor  :       |
# interface :  changes : compatibility
libobby_la_LDFLAGS = -version-info 0:0:0 -release 0

INCLUDES = $(libobby_CFLAGS) -I../inc/

pkginclude_HEADERS = \
	inc/position.hpp \
	inc/user.hpp \
	inc/record.hpp \
	inc/delete_record.hpp \
	inc/insert_record.hpp \
	inc/user_table.hpp \
	inc/client_user_table.hpp \
	inc/server_user_table.hpp \
	inc/host_user_table.hpp \
	inc/line.hpp \
	inc/document.hpp \
	inc/client_document.hpp \
	inc/server_document.hpp \
	inc/host_document.hpp \
	inc/buffer.hpp \
	inc/client_buffer.hpp \
	inc/server_buffer.hpp \
	inc/host_buffer.hpp
#	inc/rendezvous.hpp

libobby_la_SOURCES = \
	src/common.cpp \
	src/position.cpp \
	src/user.cpp \
	src/record.cpp \
	src/insert_record.cpp \
	src/delete_record.cpp \
	src/user_table.cpp \
	src/client_user_table.cpp \
	src/server_user_table.cpp \
	src/host_user_table.cpp \
	src/line.cpp \
	src/document.cpp \
	src/client_document.cpp \
	src/server_document.cpp \
	src/host_document.cpp \
	src/buffer.cpp \
	src/client_buffer.cpp \
	src/server_buffer.cpp \
	src/host_buffer.cpp
#	src/rendezvous.cpp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = lobby-1.0.pc

