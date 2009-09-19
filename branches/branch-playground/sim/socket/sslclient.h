
#ifndef SIM_SSLCLIENT_H
#define SIM_SSLCLIENT_H

#ifdef ENABLE_OPENSSL

#include "buffer.h"
#include "socketnotify.h"
#include "socket.h"

typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;
typedef struct bio_st BIO;

namespace SIM
{
	class EXPORT SSLClient : public SocketNotify, public Socket
	{
	public:
		SSLClient(Socket*);
		~SSLClient();
		virtual int read(char *buf, unsigned int size);
		virtual void write(const char *buf, unsigned int size);
		virtual void connect(const QString &host, unsigned short port);
		virtual void close();
		virtual unsigned long localHost();
		virtual void pause(unsigned);
		bool connected() const { return m_bSecure; }
		Socket *socket() const { return sock; }
		void setSocket(Socket *s);
		bool init();
		void accept();
		void connect();
		void shutdown();
		void process(bool bInRead=false, bool bWantRead=false);
		void write();
		void clear();
	protected:
		virtual bool initSSL() = 0;
		bool initTLS1(bool bDH, bool forceSSL = true);
		bool initBIO();
		Buffer wBuffer;
		virtual void connect_ready();
		virtual void read_ready();
		virtual void write_ready();
		virtual void error_state(const QString &err, unsigned code = 0);
		Socket *sock;
		enum State
		{
			SSLAccept,
			SSLConnect,
			SSLShutdown,
			SSLWrite,
			SSLConnected
		};
		State state;
		bool m_bSecure;
		SSL_CTX *mpCTX;
		SSL     *mpSSL;
		BIO     *mrBIO;
		BIO     *mwBIO;
	};

}

#endif
#endif

// vim: set expandtab:

