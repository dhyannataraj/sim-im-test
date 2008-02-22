#ifndef __GUI_CLIENT_H__
#define __GUI_CLIENT_H__

#include "client_base.h"
#include "msg_base.h"

#include "ui_msg_wnd.h"
#include "contactlist.h"

#include <QByteArray>
#include <QList>
#include <QTextEdit>
#include <QWidget>
#include <QString>
#include <QObject>

//const char* SMsgWndID = "SMsgWnd";
//const char* SContactListID = "SContactList";

class SCommonUi: public QObject
{
	Q_OBJECT
	
	QString m_proto;
	QString m_ID;
	QTextEdit *m_edit;
	
protected:
	QWidget *m_wnd;
	void InitCommonUi(QWidget *wnd, QString descr, QString proto="");
public:
	
	virtual ~SCommonUi()
	{
		if(m_wnd)
			delete m_wnd;
	}
	
//	QWidget* getUi() { return m_wnd; }
	
	bool setUi(QWidget *wnd)
	{
		if(!wnd)
			return false;
		m_wnd = wnd;
		return true;
	}
	
	bool loadUi(QString path);
public slots:
	QString getDescr() { return m_proto + ':' + m_ID; }
	QString getID() { return m_ID; }
	QString getProto() { return m_proto; }
	void getTextMsg(STextMsg&);
	virtual void getBinMsg(SIntMsg&) = 0;
	void show() { m_wnd->show(); }
	void hide() { m_wnd->hide(); }
	
};

class SMsgWnd: public SCommonUi 
{
	Q_OBJECT
	
	Ui::msgWnd msgWnd;
	
	bool isInit;
	
	bool findUi();
	
public:
	SMsgWnd();
	void Init(QString descr, QString proto, QWidget *wnd=0);
	QWidget* getUi()
	{
		if(isInit)
			return m_wnd; 
		return 0;
	}

	QTextEdit *editField;
	QTextEdit *msgField;
	QWidget *protoWidg;
	QWidget *userWidg;
	QWidget *msgWidg;
	
public slots:
	void loadCustomUi(QString path) {}
	void getBinMsg(SIntMsg&) { }
signals:
	void debug(QString);
};

// place your code here
class SUiClient: public SClient
{
	Q_OBJECT

	QList<SCommonUi*> m_UIs;
	QList<QWidget *> m_windows;
	
	SContactList m_cl;
	
	void registerUi(SCommonUi *widg)
	{
		m_UIs.append(widg);
		connect(widg, SIGNAL(debug(QString)), this, SIGNAL(debug(QString))); 
	}
	void genContact(QByteArray&);
public:
	SUiClient();
	~SUiClient() { }
	//void Init();
	void createMsgWindow(QString, QString);
	
public slots:
	void getMsg(SIntMsg&);
	void getMsg(STextMsg&);
	void showAllWindows();

signals:
	void msgCreated(STextMsg&);
	void windowCreated();
	void debug(QString);
	void createWidget(quint16, QWidget *, QString);
};

#endif // __GUI_CLIENT_H__
