
#ifndef SIM_SERVERSOCKETNOTIFY_H
#define SIM_SERVERSOCKETNOTIFY_H

#include <QString>

#include "simapi.h"

namespace SIM
{
	class Socket;
	class TCPClient;
	class ServerSocket;
	class EXPORT ServerSocketNotify
	{
	public:
		ServerSocketNotify();
		virtual ~ServerSocketNotify();
		virtual bool accept(Socket*, unsigned long ip) = 0;
		virtual void bind_ready(unsigned short port) = 0;
		virtual bool error(const QString &err) = 0;
		virtual void bind(unsigned short mixPort, unsigned short maxPort, TCPClient *client);
#ifndef WIN32
		virtual void bind(const char *path);
#endif
		void setListener(ServerSocket *listener);
		ServerSocket *m_listener;
	};
}

#endif

// vim: set expandtab:
