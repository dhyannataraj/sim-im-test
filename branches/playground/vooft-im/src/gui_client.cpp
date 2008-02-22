#include "gui_client.h"
#include "msg_base.h"
#include "ev_types.h"
#include "ui_msg_wnd.h"
#include "ui_msg_tabs.h"

#include <QByteArray>
#include <QTextEdit>
#include <QUiLoader>
#include <QWidget>
#include <QString>
#include <QObject>
#include <QtGlobal>

void SCommonUi::getTextMsg(STextMsg &msg)
{
	m_edit = m_wnd->findChild<QTextEdit *>("text_buffer");
	if(!m_edit)
		return;
	m_edit->clear();
	m_edit->insertHtml(msg.msg);
}

void SCommonUi::InitCommonUi(QWidget *wnd, QString descr, QString proto)
{
	m_proto = proto;
	m_ID = descr;
	m_wnd = wnd;
	m_edit = wnd->findChild<QTextEdit *>("text_buffer");
}

SMsgWnd::SMsgWnd()
{

}

void SMsgWnd::Init(QString descr, QString proto, QWidget *wnd)
{
	if(!wnd || !wnd->findChild<QTextEdit *>("text_buffer"))
	{
		wnd = new QWidget();
		msgWnd.setupUi(wnd);
		emit debug("wrong QWidget *wnd, creating new one...");
	}
	InitCommonUi(wnd, descr, proto);
	isInit = findUi();
	emit debug("isInit == " + QString::number(isInit));
}

bool SMsgWnd::findUi()
{
	editField = m_wnd->findChild<QTextEdit *>("msgEdit");
	msgField = m_wnd->findChild<QTextEdit *>("msgsView");
	protoWidg = m_wnd->findChild<QWidget *>("protoWidget");
	userWidg = m_wnd->findChild<QWidget *>("userInfoWidget");
	msgWidg = m_wnd->findChild<QWidget *>("msgWidget");
	
	return (editField && msgField && protoWidg && userWidg && msgWidg);
}

SUiClient::SUiClient()
{
	m_cl.show();
	for(int i=GUI_EVENTS+1; i<END_OF_GUI_EVENTS; i++)
		m_types.append(i);
}

void SUiClient::createMsgWindow(QString ID, QString protoName)
{
	SMsgWnd *messageWindow = new SMsgWnd();
	connect(messageWindow, SIGNAL(debug(QString)), this, SIGNAL(debug(QString)));
	messageWindow->Init(ID, protoName, 0);
	registerUi(messageWindow);
	
	connect(this, SIGNAL(windowCreated()), this, SLOT(showAllWindows()));
	
	Ui::msgTabs msgTabs;
	QWidget *tabWindow = new QWidget();
	msgTabs.setupUi(tabWindow);
	
	QTabWidget *tabs = tabWindow->findChild<QTabWidget *>("tabs");
	if(!tabs)
		emit debug("can't find tabs");
	tabs->clear();
	tabs->addTab(messageWindow->getUi(), "msg");

	emit debug("tabs->count() == " + QString::number(tabs->count()));
	
	emit createWidget(SMsgWndProto, messageWindow->protoWidg, protoName);
	emit createWidget(SMsgWndUserInfo, messageWindow->userWidg, protoName);
//	emit createWidget(SMsgBtns, messageWindow->msgWidg, protoName);

	m_windows.append(tabWindow);
	
//	emit windowCreated();	
}

void SUiClient::showAllWindows()
{
	foreach(QWidget *ui, m_windows)
	{
		ui->show();
	}
}

void SUiClient::getMsg(STextMsg &msg)
{
	// TODO
}

void SUiClient::getMsg(SIntMsg &msg)
{
	if(!msg.parsed)
		return;
	switch(msg.type)
	{
		case SGenContact: genContact(msg.data); 
	}
}

void SUiClient::genContact(QByteArray &block)
{
	SContact *contact = SContact::genContact(block);
	// TODO
}
