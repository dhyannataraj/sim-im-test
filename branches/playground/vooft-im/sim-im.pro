TEMPLATE = app
HEADERS += src/sim.h \
 src/protocol_base.h \
 src/service_base.h \
 src/msg_base.h \
 src/ev_types.h \
 src/client_base.h \
 src/gui_client.h \
 src/gui_codes.h \
 src/contacts.h \
 src/contactlist.h
SOURCES += src/sim.cpp \
 src/main.cpp \
 src/gui_client.cpp \
 src/gui_codes.cpp \
 src/contacts.cpp \
 src/contactlist.cpp
QT += core network gui
CONFIG += uitools
FORMS += ui/msg_wnd.ui ui/msg_tabs.ui ui/contact_list.ui
