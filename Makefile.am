SUBDIRS = src . test

pkginclude_HEADERS =  inc/common.hpp
pkginclude_HEADERS += inc/position.hpp
pkginclude_HEADERS += inc/user.hpp
pkginclude_HEADERS += inc/record.hpp
pkginclude_HEADERS += inc/delete_record.hpp
pkginclude_HEADERS += inc/insert_record.hpp
pkginclude_HEADERS += inc/user_table.hpp
pkginclude_HEADERS += inc/client_user_table.hpp
pkginclude_HEADERS += inc/server_user_table.hpp
pkginclude_HEADERS += inc/host_user_table.hpp
pkginclude_HEADERS += inc/line.hpp
pkginclude_HEADERS += inc/document.hpp
pkginclude_HEADERS += inc/client_document.hpp
pkginclude_HEADERS += inc/server_document.hpp
pkginclude_HEADERS += inc/host_document.hpp
pkginclude_HEADERS += inc/buffer.hpp
pkginclude_HEADERS += inc/client_buffer.hpp
pkginclude_HEADERS += inc/server_buffer.hpp
pkginclude_HEADERS += inc/host_buffer.hpp

if WITH_HOWL
pkginclude_HEADERS += inc/rendezvous.hpp
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = lobby-1.0.pc

