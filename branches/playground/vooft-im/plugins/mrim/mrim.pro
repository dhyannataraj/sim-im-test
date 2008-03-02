SOURCES += mrim.cpp mrim_services.cpp
HEADERS += mrim.h \
 ../../src/msg_base.h \
 ../../src/protocol_base.h \
 ../../src/service_base.h \
 ../../src/ev_types.h \
 mrim_services.h \
 proto.h \
 ../../src/client_base.h
TEMPLATE = lib
QT += network core
CONFIG += plugin
DESTDIR = ../
