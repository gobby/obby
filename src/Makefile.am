lib_LTLIBRARIES = libobby.la

#  current  : revision :      age
#     ^     :     ^    :       ^
#     |     :   minor  :       |
# interface :  changes : compatibility
libobby_la_LDFLAGS = -version-info 0:0:0 -release 0

libobby_la_LIBADD = $(libobby_LIBS)

if WIN32
libobby_la_LDFLAGS += -no-undefined
endif

libobby_la_CPPFLAGS = $(libobby_CFLAGS) -I../inc

libobby_la_SOURCES = common.cpp
libobby_la_SOURCES += position.cpp
libobby_la_SOURCES += duplex_signal.cpp
libobby_la_SOURCES += ptr_iterator.cpp
libobby_la_SOURCES += user.cpp
libobby_la_SOURCES += record.cpp
libobby_la_SOURCES += insert_record.cpp
libobby_la_SOURCES += delete_record.cpp
libobby_la_SOURCES += user_table.cpp
libobby_la_SOURCES += client_user_table.cpp
libobby_la_SOURCES += server_user_table.cpp
libobby_la_SOURCES += host_user_table.cpp
libobby_la_SOURCES += line.cpp
libobby_la_SOURCES += document.cpp
libobby_la_SOURCES += client_document.cpp
libobby_la_SOURCES += server_document.cpp
libobby_la_SOURCES += host_document.cpp
libobby_la_SOURCES += buffer.cpp
libobby_la_SOURCES += client_buffer.cpp
libobby_la_SOURCES += server_buffer.cpp
libobby_la_SOURCES += host_buffer.cpp

if WITH_HOWL
libobby_la_CPPFLAGS += $(libhowl_CFLAGS)
libobby_la_LIBADD   += $(libhowl_LIBS)
libobby_la_SOURCES  += zeroconf.cpp
endif

