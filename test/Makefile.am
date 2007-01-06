check_PROGRAMS = serialise text jupiter
TESTS = serialise text jupiter

AM_CPPFLAGS        = -I../inc/ $(libobby_CFLAGS)
LDADD              = $(libobby_LIBS)

serialise_SOURCES  = test_serialise.cpp
serialise_LDADD    = -L../src/serialise -lserialise
serialise_SOURCES += ../src/common.cpp # gettext

text_SOURCES       = test_text.cpp
text_SOURCES      += ../src/text.cpp
text_LDADD         = -L../src/serialise -lserialise
text_SOURCES      += ../src/user.cpp
text_SOURCES      += ../src/user_table.cpp
text_SOURCES      += ../src/colour.cpp
text_SOURCES      += ../src/common.cpp # gettext

jupiter_SOURCES    = test_jupiter.cpp
jupiter_SOURCES   += ../src/jupiter_algorithm.cpp
jupiter_SOURCES   += ../src/jupiter_client.cpp
jupiter_SOURCES   += ../src/jupiter_server.cpp
jupiter_SOURCES   += ../src/jupiter_error.cpp
jupiter_SOURCES   += ../src/jupiter_undo.cpp
jupiter_LDADD      = -L../src/serialise -lserialise
jupiter_SOURCES   += ../src/text.cpp
jupiter_SOURCES   += ../src/document.cpp
jupiter_SOURCES   += ../src/user.cpp
jupiter_SOURCES   += ../src/user_table.cpp
jupiter_SOURCES   += ../src/colour.cpp
jupiter_SOURCES   += ../src/common.cpp # gettext

