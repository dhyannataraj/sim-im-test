SUBDIRS =mrim 
HEADERS += ../src/client_base.h \
 ../src/msg_base.h \
 ../src/protocol_base.h \
 ../src/service_base.h \
 ../src/ev_types.h
TEMPLATE = app
QT += network core
