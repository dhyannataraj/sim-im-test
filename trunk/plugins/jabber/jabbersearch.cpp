/***************************************************************************
                          jabbersearch.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabbersearch.h"
#include "jabberclient.h"
#include "jabber.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qobjectlist.h>
#include <qregexp.h>

#include <vector>

using namespace std;

class CComboBox : public QComboBox
{
public:
    CComboBox(QWidget *parent, const char *name);
    void addItem(const QString &label, const char *value);
    QString value();
protected:
    vector<string> m_values;
};

CComboBox::CComboBox(QWidget *parent, const char *name)
        : QComboBox(parent, name)
{
}

void CComboBox::addItem(const QString &label, const char *value)
{
    m_values.push_back(value);
    insertItem(label);
}

QString CComboBox::value()
{
    unsigned index = currentItem();
    if (index >= m_values.size())
        return "";
    return QString::fromUtf8(m_values[index].c_str());
}

JabberSearch::JabberSearch(QWidget *receiver, JabberClient *client, const char *jid, const QString &name)
        : QChildWidget(NULL)
{
    m_client = client;
    m_jid    = jid;
    m_name	 = name;
    m_nPos	 = 0;
    m_receiver = receiver;
    QVBoxLayout *vlay = new QVBoxLayout(this);
    vlay->setMargin(6);
    lay = new QGridLayout(vlay);
    lay->setSpacing(4);
    vlay->addStretch();
    m_bDirty = false;
}

void JabberSearch::addWidget(JabberAgentInfo *data)
{
    QWidget *widget = NULL;
    bool bJoin = false;
    if (data->Type){
        if (!strcmp(data->Type, "text-single")){
            widget = new QLineEdit(this, data->Field);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
        }else if (!strcmp(data->Type, "fixed") || !strcmp(data->Type, "instructions")){
            if (data->Value){
                QString text = i18(data->Value);
                text = text.replace(QRegExp("  +"), "\n");
                QLabel *label = new QLabel(text, this);
                label->setAlignment(WordBreak);
                widget = label;
                bJoin = true;
            }
        }else if (!strcmp(data->Type, "list-single")){
            CComboBox *box = new CComboBox(this, data->Field);
            for (unsigned i = 0; i < data->nOptions; i++){
                const char *label = get_str(data->OptionLabels, i);
                const char *val   = get_str(data->Options, i);
                if (label && val)
                    box->addItem(i18(label), val);
            }
            widget = box;
        }else if (!strcmp(data->Type, "key")){
            if (data->Value)
                m_key = data->Value;
        }else if (!strcmp(data->Type, "username")){
            widget = new QLineEdit(this, "username");
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            set_str(&data->Label, "Username");
        }else if (!strcmp(data->Type, "nick")){
            widget = new QLineEdit(this, "nick");
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            set_str(&data->Label, "Nick");
        }else if (!strcmp(data->Type, "password")){
            widget = new QLineEdit(this, "password");
            static_cast<QLineEdit*>(widget)->setEchoMode(QLineEdit::Password);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            set_str(&data->Label, "Password");
        }
    }
    if (widget){
        if (bJoin){
            lay->addMultiCellWidget(widget, m_nPos, m_nPos, 0, 1);
        }else{
            lay->addWidget(widget, m_nPos, 1);
            if (data->Label){
                QLabel *label = new QLabel(i18(data->Label), this);
                label->setAlignment(AlignRight);
                lay->addWidget(label, m_nPos, 0);
                label->show();
            }
        }
        widget->show();
        m_nPos++;
        m_bDirty = true;
        QTimer::singleShot(0, this, SLOT(setSize()));
    }
}

void JabberSearch::setSize()
{
    if (!m_bDirty || (parent() == NULL))
        return;
    m_bDirty = false;
    for (QWidget *p = this; p; p = p->parentWidget()){
        QSize s  = p->sizeHint();
        QSize s1 = QSize(p->width(), p->height());
        p->setMinimumSize(s);
        p->resize(QMAX(s.width(), s1.width()), QMAX(s.height(), s1.height()));
        if (p->layout())
            p->layout()->invalidate();
        if (p == topLevelWidget())
            break;
    }
    QWidget *t = topLevelWidget();
    QSize s = t->sizeHint();
    t->resize(QMAX(t->width(), s.width()), QMAX(t->height(), s.height()));
    t->adjustSize();
}

#if 0
static const char *any_data[] =
    {
        I18N_NOOP("First (Given)"),
        I18N_NOOP("Last (Family)"),
        I18N_NOOP("Nick (Alias)"),
        I18N_NOOP("Email"),
        I18N_NOOP("Select the speed of the search.  \"Fast\" matches your string to the beginning of the field only (ie. \"b\" would yield Bob,Bill,etc...)  \"Slower\" matches your string anywhere in the field (ie. \"b\" would yield Bob, Bill, Caleb, Robbie, etc...)"),
        I18N_NOOP("Search Speed"),
        I18N_NOOP("Fast / Less accurate"),
        I18N_NOOP("Slower / More extensive")
        I18N_NOOP("Full name")
        I18N_NOOP("First name")
        I18N_NOOP("Last name")
        I18N_NOOP("Nickname")
        I18N_NOOP("E-mail")
        I18N_NOOP("Username")
        I18N_NOOP("Password")
        I18N_NOOP("Enter your MSN Messenger account and password. Example: user@hotmail.com.  Nickname is optional.")
        I18N_NOOP("Enter your AIM screenname or ICQ UIN and the password for that account")
        I18N_NOOP("Enter your YAHOO! Messenger Username and Password.")
        I18N_NOOP("You need a x:data capable client to register.")
        I18N_NOOP("Enter nick you want to register.")
        I18N_NOOP("Complete the form to submit your searchable attributes in the Jabber User Directory")
        I18N_NOOP("Fill in all of the fields to add yourself to the JUD.")
        I18N_NOOP("Fill in a field to search for any matching Jabber User (POSIX regular expressions allowed)")
        I18N_NOOP("Fill in a field to search for any matching Jabber users.")
    };
#endif

QString JabberSearch::i18(const char *text)
{
    if ((text == NULL) || (*text == 0))
        return "";
    QString res = QString::fromUtf8(text);
    QCString str = res.latin1();
    QString  tstr = i18n(str);
    if (tstr == QString(str))
        return res;
    return tstr;
}

bool JabberSearch::canSearch()
{
    bool bRes = false;

    QObjectList *l = queryList("QLineEdit");
    QObjectListIt it( *l );
    QObject *obj;

    while ((obj = it.current()) != 0 ){
        if (!static_cast<QLineEdit*>(obj)->text().isEmpty()){
            bRes = true;
            break;
        }
        ++it;
    }
    delete l;
    return bRes;
}

QString JabberSearch::condition()
{
    QString res;

    QObjectList *l = queryList("QLineEdit");
    QObjectListIt it( *l );
    QObject *obj;

    while ((obj = it.current()) != 0 ){
        QLineEdit *edit = static_cast<QLineEdit*>(obj);
        if (!edit->text().isEmpty()){
            if (!res.isEmpty())
                res += ";";
            res += edit->name();
            res += "=";
            res += quoteChars(edit->text(), ";");
        }
        ++it;
    }
    delete l;

    l = queryList("QComboBox");
    QObjectListIt it1( *l );
    while ((obj = it1.current()) != 0 ){
        CComboBox *box = static_cast<CComboBox*>(obj);
        if (!res.isEmpty())
            res += ";";
        res += box->name();
        res += "=";
        res += box->value();
        ++it1;
    }
    delete l;

    return res;
}

#ifndef WIN32
#include "jabbersearch.moc"
#endif

