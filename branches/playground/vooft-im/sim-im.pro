TEMPLATE = app
HEADERS += src/sim.h \
 src/protocol_base.h \
 src/service_base.h \
 src/msg_base.h \
 src/ev_types.h \
 src/client_base.h \
 src/gui_client.h \
 src/common_gui.h
SOURCES += src/sim.cpp src/main.cpp src/gui_client.cpp src/common_gui.cpp
QT += core network gui
CONFIG += uitools
FORMS += ui/logindlgbase.ui \
 ui/searchallbase.ui \
 ui/searchbase.ui \
 ui/msgcfgbase.ui \
 ui/newprotocolbase.ui \
 ui/msg_wnd.ui \
 ui/msg_tabs.ui
