/***************************************************************************
                          chatwnd.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CHATWND_H
#define _CHATWND_H

#include "defs.h"
#include <qmainwindow.h>

class ICQEvent;
class ICQChat;

class QListBox;
class QTextEdit;
class QToolButton;
class QFile;
class TextShow;
class TransparentTop;

class ChatWindow : public QMainWindow
{
    Q_OBJECT
public:
    ChatWindow(ICQChat *chatMsg);
    ~ChatWindow();
    ICQChat *chat;
    virtual void setBackgroundPixmap(const QPixmap &pm);
protected slots:
    void processEvent(ICQEvent*);
    void sendLine();
    void toggleBold(bool);
    void toggleItalic(bool);
    void toggleUnderline(bool);
    void openLog();
    void setFgColor();
protected:
    bool bClientMode;
    QString chatHeader(unsigned long uin);
    QColor baseColor;
    void resizeEvent(QResizeEvent*);
    QListBox *lstUsers;
    QTextEdit *edtChat;
    TextShow *txtChat;
    QToolButton *btnSave;
    QToolButton *btnBold;
    QToolButton *btnItalic;
    QToolButton *btnUnderline;
    QToolButton *btnFgColor;
    QFile *logFile;
    QColor chatColor(unsigned long c);
    bool bInit;
    TransparentTop *transparent;
};

#endif

