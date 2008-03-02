#ifndef __MRIM_H__
#define __MRIM_H__

#include <QtGlobal>
#include <QAbstractSocket>
#include <QObject>
#include <QTcpSocket>

#include "../../src/protocol_base.h"
#include "proto.h"
#include "mrim_services.h"
typedef mrim_packet_header_t mrimHeader;

// place your code here

class MrimProtocol: public SProtocol//, public QTcpSocket
{
	Q_OBJECT
	Q_INTERFACES(SProtocol)
	
	quint32 m_seq; // seq - special number for every message
	QString m_srvName; // server address
	quint16 m_port; // server port
	
	mrimHeader genMrimHeader(u_long type, u_long dlen, u_long seq);
	quint16 genMsgType(quint16);
	quint16 genMrimType(quint16);
	void Login();
	
protected slots:
	void readSrv();
	
public:
	void init();
	MrimProtocol();
	~MrimProtocol();
	static void writeUL(QDataStream&, quint32);
	static void writeLPS(QDataStream&, QString);
	quint32 readUL(QDataStream&);
	QString readLPS(QDataStream&);
	void putHeader(QDataStream&, mrimHeader);
	void readHeader(QDataStream&, mrimHeader&);
	QByteArray readData(QDataStream&, quint32 size);
	
public slots:
	void Login(QString, QString);
	void Logout();
	void listen();
	void toSend(SIntMsg&);
	void parsed(SIntMsg&);
	void fillUi(quint16, QWidget*);
};

#endif // __MRIM_H__
