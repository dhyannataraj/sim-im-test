/***************************************************************************
                          msgedit.h  -  description
                             -------------------
    begin                : Sat Mar 16 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MSGEDIT_H
#define _MSGEDIT_H

#include "defs.h"
#include "cfg.h"

#include <qsplitter.h>
#include <qlabel.h>

class QCheckBox;
class QVGroupBox;
class QHGroupBox;
class QLabel;
class QFrame;
class QCheckBox;
class QPushButton;
class QPopupMenu;
class CPushButton;
class QComboBox;
class QLineEdit;
class EditSpell;
class ICQMessage;
class ICQEvent;
class UserTbl;
class TextShow;
class PictPushButton;
class EditFile;
class UserTab;
class History;
class MsgView;
class WndCancel;
class CToolButton;
class PictButton;
class Tmpl;

class SmileLabel : public QLabel
{
    Q_OBJECT
public:
    SmileLabel(int id, QWidget *parent);
signals:
    void clicked(int id);
protected:
    void mouseReleaseEvent(QMouseEvent*);
    int id;
};

class SmilePopup : public QFrame
{
    Q_OBJECT
public:
    SmilePopup(QWidget *parent);
signals:
    void insert(const QString &text, bool indent, bool checkNewLine, bool removeSelected);
protected slots:
    void labelClicked(int id);
};

class MsgEdit : public QSplitter
{
    Q_OBJECT
public:
    MsgEdit(QWidget *parent, unsigned long Uin=0);
    ~MsgEdit();
    void setUin(unsigned long uin);
    void setMessage(ICQMessage *m=NULL, bool bMark=false, bool bInTop=false, bool bSaveEdit=false);
    ICQMessage *message() { return msg; }
    void markAsRead();
    bool canSpell();
    bool canSend();
    QString userName();
    int tabId;
    UserTab *tab;
    unsigned long	Uin;
    bool load(std::istream &s, string &part);
    void save(std::ostream &s);
    bool bMultiply;
public slots:
    void realSend();
    void send();
    void spell();
    void textChanged();
    void editTextChanged();
    void textChanged(const QString&);
    void spellDone(bool);
    void makeMessage();
    void setParam(unsigned long);
    void setState();
    void action(int);
    void action(int, bool bSaveEdit);
    void showMessage(unsigned long msgId);
signals:
    void setSendState(bool);
    void setStatus(const QString&);
    void setStatus(const QString&, int);
    void setMessageType(const QString &icon, const QString &text);
    void destroyChild(int id);
    void showUsers(bool, unsigned long);
protected slots:
    void setMsgBackgroundColor();
    void setMsgForegroundColor();
    void setBold(bool);
    void setItalic(bool);
    void setUnder(bool);
    void setFont();
    void editFontChanged(const QFont &f);
    void sendClick();
    void nextClick();
    void replyClick();
    void quoteClick();
    void forwardClick();
    void grantClick();
    void refuseClick();
    void messageReceived(ICQMessage*);
    void processEvent(ICQEvent*);
    void acceptMessage();
    void declineMessage(int);
    void closeToggle(bool);
    void toggleMultiply();
    void chatChanged();
    void ftChanged();
    void modeChanged(bool);
    void insertSmile();
    void topReady(const QString &res);
    void bottomReady(const QString &res);
protected:
    Tmpl *tmpl;
    MsgView *msgView;
    QFrame  *frmEdit;
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void closeEvent(QCloseEvent *e);
    string smsChunk();
    QString trim(const QString&);
    QString chunk(const QString &s, int len);
    bool isLatin1(const QString &s);
    void fillPhones();
    void setupNext();
    History *mHistory;
    History *history();
    ICQMessage *msg;

    CToolButton	*btnCloseSend;
    PictButton	*btnSend;

    CToolButton	*btnBgColor;
    CToolButton	*btnFgColor;
    CToolButton	*btnBold;
    CToolButton	*btnItalic;
    CToolButton	*btnUnder;
    CToolButton	*btnFont;
    CToolButton *btnSmile;
#ifdef USE_SPELL
    CToolButton	*btnSpell;
#endif
    PictButton	*btnNext;
    PictButton	*btnReply;
    PictButton	*btnForward;
    PictButton	*btnQuote;
    PictButton	*btnGrant;
    PictButton	*btnRefuse;
    PictButton	*btnAccept;
    PictButton	*btnDecline;
    CToolButton	*btnMultiply;

    TextShow   *view;
    QHGroupBox *phone;
    QHGroupBox *url;
    QHGroupBox *file;
    QLabel	   *lblFile;
    QComboBox  *phoneEdit;
    QLineEdit  *urlEdit;
    EditFile   *fileEdit;
    QHGroupBox *lblUsers;
    QPopupMenu *declineMenu;
    UserTbl    *users;
    EditSpell *edit;
    ICQEvent *sendEvent;
    bool bCloseSend;
    bool bInIgnore;
    bool bFirstShow;
    QString msgTail;
    WndCancel *wndCancel;
};

#endif

