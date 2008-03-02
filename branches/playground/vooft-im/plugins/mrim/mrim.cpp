#include <QtGlobal>
#include <QtPlugin>
#include <QTextStream>
#include <QString>
#include <QStringList>

#include "mrim.h"
#include "proto.h"
#include "../../src/ev_types.h"
#include "../../src/msg_base.h"
#include "mrim_services.h"

MrimProtocol::MrimProtocol()
{

}

void MrimProtocol::init()
{
	m_ProtoName = "mrim";
	m_Logined = false;

	connect(this, SIGNAL(disconnected()), this, SLOT(Logout()));

// Internal services

// Put to external function adding of service
	UpholdSvc *uphold = new UpholdSvc();
	RegisterService(uphold);
	connect(this, SIGNAL(connected(bool)), uphold, SLOT(genUphold()));
	
	ContactListParser *cl_parser = new ContactListParser();
	RegisterService(cl_parser);
// Internal services END
}

MrimProtocol::~MrimProtocol()
{
	if(isConnected())
		Logout();
}

void MrimProtocol::toSend(SIntMsg &msg)
{
/*
struct is:
	quint32 seq; // uses for replying messages
	char a[]; // any data to send
*/
	debug_log("toSend()");
	QByteArray block;
	QDataStream out(msg.data), in(&block, QIODevice::WriteOnly);
	
	in.setByteOrder(QDataStream::LittleEndian);
	
	mrimHeader hdr = genMrimHeader(genMrimType(msg.type), 0, 0);
	
	out >> hdr.seq;
	if(!hdr.seq)
		hdr.seq = m_seq++;
	
	putHeader(in, hdr);
	
/*	quint8 tmp;
	
	while(!out.atEnd())
	{
		out >> tmp;
		in << tmp;
	}*/
	
	hdr.dlen = block.size() - HEADER_SIZE;
	
	in.device()->seek(0);
	
	putHeader(in, hdr);
	
	quint64 i = write(block);
	
	debug_log("bytes written: " + QString::number(i));
	
}

void MrimProtocol::parsed(SIntMsg &msg)
{
	if(msg.parsed)
	{
		emit processed(msg); // if msg was arrived from service -> send to clients
//		debug_log("msg processed => send to clients");
	}
	else
	{
//		debug_log("msg not processed => send to services");
		emit recieved(msg); // if it was arrived from clients -> send to services
	}
}

void MrimProtocol::Login(QString login, QString pass)
{
	m_ID = login;
	m_Pass = pass;
	m_seq = 1;
	
	connectToHost("mrim.mail.ru", 2042);
	debug_log("Connecting to host mrim.mail.ru:2042");
	
	waitForConnected();
	
	if(!isConnected())
	{
		debug_log("Can't connect to host mrim.mail.ru:2042");
		emit connected(false);
		return;
	}
	
	connect(this, SIGNAL(readyRead()), this, SLOT(readSrv()));

}

void MrimProtocol::readSrv()
{
	QTextStream in(this);
	QString str;
	
	in >> str;
	
	QStringList list = str.split(':');
	debug_log(str);
	if(list.size()<2)
	{
		debug_log("Can't get server name");
		emit connected(false);
		return;
	}
	
	m_srvName = list.at(0);
	m_port = list.at(1).toInt();
	
	debug_log("Choosen server " + m_srvName + ':' + QString::number(m_port));
	debug_log("Disconnected from server mrim.mail.ru:2042");
	disconnectFromHost();
	
	disconnect(this, SIGNAL(readyRead()), this, SLOT(readSrv()));
	
	connect(this, SIGNAL(readyRead()), this, SLOT(listen()));
	
	connectToHost(m_srvName, m_port);
	
// Sending MRIM_CS_HELLO
	mrimHeader header = genMrimHeader(MRIM_CS_HELLO, 0, ++m_seq);
	
	QByteArray block;
	
	QDataStream out(&block, QIODevice::WriteOnly);
	
	out.setByteOrder(QDataStream::LittleEndian);
	
	putHeader(out, header);
	
	debug_log("Sended MRIM_CS_HELLO...");

	qint64 i = write(block);
	
	debug_log("Bytes written: " + QString::number(i));
}

mrimHeader MrimProtocol::genMrimHeader(u_long type, u_long dlen, u_long seq)
{
	mrim_packet_header_t header;
	header.magic = CS_MAGIC;
	header.proto = PROTO_VERSION;
	header.seq = seq;
	header.msg = type;
	//m_myIp = "0.0.0.0";
	//m_myPort = 0;
	header.from = 0;
	header.fromport = 0;
	header.dlen = dlen;
	
	return header;
}

void MrimProtocol::Login()
{
	if(m_Logined)
		return;
	
	QByteArray block;
	
	QDataStream out(&block, QIODevice::WriteOnly);
	
	out.setByteOrder(QDataStream::LittleEndian);

	mrimHeader hdr = genMrimHeader(MRIM_CS_LOGIN2, 0, ++m_seq);
	
	putHeader(out, hdr);
	writeLPS(out, m_ID);
	writeLPS(out, m_Pass);
	writeUL(out, quint32(STATUS_ONLINE));
	writeLPS(out, QString("SIM-IM"));

	hdr = genMrimHeader(MRIM_CS_LOGIN2, block.size()-HEADER_SIZE, m_seq);
	
	out.device()->seek(0);

	debug_log("Package size: " + QString::number(block.size()-HEADER_SIZE));
	
	putHeader(out, hdr);
	
	int i = write(block); 
	
	debug_log("Bytes written: " + QString::number(i)); 
}

void MrimProtocol::listen()
{
//	debug_log("listen()");
	while(bytesAvailable()<HEADER_SIZE)
	{ ;	}
	
	mrimHeader hdr;
	QDataStream out (this);
	
	out.setByteOrder(QDataStream::LittleEndian);

	readHeader(out, hdr);
	
	//out.skipRawData(HEADER_SIZE);
	
	SIntMsg msg;
	
//	debug_log("_AFTER_ QDataStream.pos = " + QString::number(out.device()->pos()));
	
	msg.type = genMsgType(hdr.msg);
	msg.data = readData(out, hdr.dlen);
	msg.parsed = false; 
	
	debug_log("dlen = " + QString::number(hdr.dlen));
	
	bool parsed = recieved(msg);
	
	if(!parsed)
		debug_log("Can't process package. Internal type: " + QString::number(msg.type) + ", mrim type: " + QString::number(hdr.msg, 16));

	if(hdr.msg==MRIM_CS_HELLO_ACK)
	{
		Login();
	}
}

quint16 MrimProtocol::genMsgType(quint16 type)
{
	quint16 result;
	
	debug_log("Type: 0x" + QString::number(type, 16));

	switch(type)
	{
		case MRIM_CS_HELLO_ACK: result = SPingMsg; break;
		case MRIM_CS_LOGIN_ACK: result = SLogin; emit connected(true); debug_log("logined"); break; 
		case MRIM_CS_LOGIN_REJ: result = SLoginRej; emit connected(false); break;
		case MRIM_CS_CONNECTION_PARAMS: result = SPingMsg; break;
		case MRIM_CS_CONTACT_LIST2: result = SContacts; debug_log("contact list"); break;
		case MRIM_CS_USER_INFO: result = SUserInfo; break; 
		
		default: debug_log("unknown type: 0x" + QString::number(type, 16)); result = -1;
	}
// 
	
	return result;
}

quint16 MrimProtocol::genMrimType(quint16 type)
{
	quint16 result = 0;
	
	switch(type)
	{
		case SUpholdConnect: result = MRIM_CS_PING; 
	}
	
	return result;
}

void MrimProtocol::writeUL(QDataStream &out, quint32 ul)
{
	out << ul;
}

void MrimProtocol::writeLPS(QDataStream &out, QString lps)
{
	writeUL(out, lps.length());
	
	QByteArray block = lps.toLatin1();
	char *c = block.data();
	
	out.writeRawData(c, lps.length());
}

quint32 MrimProtocol::readUL(QDataStream &in)
{
	quint32 result;
	
	while(bytesAvailable()<sizeof(quint32))
	{ ; }
		
	in >> result;
	
	return result;
}

QString MrimProtocol::readLPS(QDataStream &in)
{
	quint8 c;
	QString result;
	
	quint32 len = readUL(in);
	
	while(bytesAvailable()<len) 
	{ ; }
			
	for(quint32 i=0; i<len; i++)
	{
		in >> c;
		result += c;
	}
	
	return result;
}

void MrimProtocol::putHeader(QDataStream& out, mrimHeader header)
{
	writeUL(out, header.magic);
	writeUL(out, header.proto);
	writeUL(out, header.seq);
	writeUL(out, header.msg);
	writeUL(out, header.dlen);
	
//	writeUL(out, header.from);
//	writeUL(out, header.fromport);
	writeUL(out, 0);
	writeUL(out, 0);
	
	for(int i=0; i<16; i++)
		out << quint8(0); 
}

void MrimProtocol::readHeader(QDataStream &out, mrimHeader &header)
{
	out >> header.magic;
	out >> header.proto;
	out >> header.seq;
	out >> header.msg;
	out >> header.dlen;
	out >> header.from;
	out >> header.fromport;
	for(int i=0; i<16; i++)
		out >> header.reserved[i];
}

QByteArray MrimProtocol::readData(QDataStream &out, quint32 size)
{
	QByteArray result;
	QDataStream in(&result, QIODevice::WriteOnly);
	
	quint8 tmp;
	
	while(!out.atEnd())
	{
		out >> tmp;
		in << tmp;
	}
	
	return result;
}

void MrimProtocol::Logout()
{
	disconnectFromHost();
	debug_log("disconnected from server");
}

void MrimProtocol::fillUi(quint16 type, QWidget *wnd)
{
	switch(type)
	{
		case SMsgWndProto: break;
		case SMsgWndUserInfo: break;
		case SMsgBtns: break;
	}
}

Q_EXPORT_PLUGIN2(mrim, MrimProtocol)
