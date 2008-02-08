TEMPLATE = app
HEADERS += src/sim.h \
 src/protocol_base.h \
 src/service_base.h \
 src/msg_base.h \
 src/ev_types.h \
 src/client_base.h
SOURCES += src/sim.cpp src/main.cpp
QT += core network gui
