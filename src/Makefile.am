lib_LTLIBRARIES = libobby.la

#  current  : revision :      age
#     ^     :     ^    :       ^
#     |     :   minor  :       |
# interface :  changes : compatibility
libobby_la_LDFLAGS = -version-info 0:0:0 -release 0.2

libobby_la_LIBADD = $(libobby_LIBS) @extra_libraries@

if WIN32
libobby_la_LDFLAGS += -no-undefined
endif

libobby_la_CPPFLAGS = $(libobby_CFLAGS) -I../inc @extra_includes@

# gettext
libobby_la_CPPFLAGS += -DLOCALEDIR=\""$(datadir)/locale"\"

libobby_la_SOURCES = common.cpp
libobby_la_SOURCES += format_string.cpp
libobby_la_SOURCES += error.cpp
libobby_la_SOURCES += position.cpp
libobby_la_SOURCES += duplex_signal.cpp
libobby_la_SOURCES += ptr_iterator.cpp
libobby_la_SOURCES += vector_time.cpp
libobby_la_SOURCES += rsa.cpp
libobby_la_SOURCES += sha1.cpp
libobby_la_SOURCES += user.cpp
libobby_la_SOURCES += user_table.cpp
libobby_la_SOURCES += line.cpp
libobby_la_SOURCES += document.cpp
libobby_la_SOURCES += operation.cpp
libobby_la_SOURCES += no_operation.cpp
libobby_la_SOURCES += split_operation.cpp
libobby_la_SOURCES += insert_operation.cpp
libobby_la_SOURCES += delete_operation.cpp
libobby_la_SOURCES += record.cpp
libobby_la_SOURCES += jupiter_algorithm.cpp
libobby_la_SOURCES += jupiter_client.cpp
libobby_la_SOURCES += jupiter_server.cpp
libobby_la_SOURCES += document_packet.cpp

libobby_la_SOURCES += document_info.cpp
libobby_la_SOURCES += local_document_info.cpp
libobby_la_SOURCES += client_document_info.cpp
libobby_la_SOURCES += server_document_info.cpp
libobby_la_SOURCES += host_document_info.cpp
libobby_la_SOURCES += buffer.cpp
libobby_la_SOURCES += local_buffer.cpp
libobby_la_SOURCES += client_buffer.cpp
libobby_la_SOURCES += server_buffer.cpp
libobby_la_SOURCES += host_buffer.cpp

if WITH_HOWL
libobby_la_CPPFLAGS += $(libhowl_CFLAGS)
libobby_la_LIBADD   += $(libhowl_LIBS)
libobby_la_SOURCES  += zeroconf.cpp
endif

