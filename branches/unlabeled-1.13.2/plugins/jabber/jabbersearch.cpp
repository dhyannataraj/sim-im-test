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
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <qwidgetstack.h>
#include <qobjectlist.h>
#include <qregexp.h>
#include <qcheckbox.h>

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

const unsigned MAX_ELEMENTS = 8;

JabberSearch::JabberSearch(QWidget *receiver, JabberClient *client, const char *jid, const char *node, const QString &name, bool bRegister)
        : QChildWidget(NULL)
{
    m_client    = client;
    m_jid       = jid;
    if (node)
        m_node  = node;
    m_name	    = name;
    m_receiver	= receiver;
    m_bXData	= false;
    m_bFirst	= true;
	m_bRegister = bRegister;
    QVBoxLayout *vlay = new QVBoxLayout(this);
    vlay->setMargin(6);
    lay = new QGridLayout(vlay);
    lay->setSpacing(4);
    vlay->addStretch();
    m_bDirty = false;
}

typedef struct defFlds
{
    char	*tag;
    char	*name;
    bool	bRequired;
} defFlds;

static defFlds fields[] =
    {
        { "username", I18N_NOOP("Username"), true },
        { "nick", I18N_NOOP("Nick"), false },
        { "email", I18N_NOOP("EMail"), false },
        { "first", I18N_NOOP("First name"), false },
        { "last", I18N_NOOP("Last name"), false },
        { "age_min", I18N_NOOP("Age min"), false },
        { "age_max", I18N_NOOP("Age max"), false },
        { "city", I18N_NOOP("City"), false },
        { NULL, NULL, false }
    };

void JabberSearch::addWidget(JabberAgentInfo *data)
{
    QWidget *widget = NULL;
    bool bJoin = false;
    if (data->Type){
        if (!strcmp(data->Type, "x")){
            m_bXData = true;
        }else if (!strcmp(data->Type, "title")){
            if (data->Value && *data->Value)
                m_title = QString::fromUtf8(data->Value);
        }else if (!strcmp(data->Type, "text-single")){
            widget = new QLineEdit(this, data->Field);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            if (data->Value && *data->Value)
                static_cast<QLineEdit*>(widget)->setText(QString::fromUtf8(data->Value));
        }else if (!strcmp(data->Type, "text-private")){
            widget = new QLineEdit(this, data->Field);
            static_cast<QLineEdit*>(widget)->setEchoMode(QLineEdit::Password);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            if (data->Value && *data->Value)
                static_cast<QLineEdit*>(widget)->setText(QString::fromUtf8(data->Value));
        }else if (!strcmp(data->Type, "text-multi")){
            widget = new QMultiLineEdit(this, data->Field);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            if (data->Value && *data->Value)
                static_cast<QMultiLineEdit*>(widget)->setText(QString::fromUtf8(data->Value));
        }else if (!strcmp(data->Type, "boolean") && data->Label){
            widget = new QCheckBox(QString::fromUtf8(data->Label), this, data->Field);
            if (data->Value && *data->Value && (*data->Value != '0'))
                static_cast<QCheckBox*>(widget)->setChecked(true);
            set_str(&data->Label, NULL);
            bJoin = true;
        }else if (!strcmp(data->Type, "fixed")){
            if (data->Value){
                QString text = i18(data->Value);
                text = text.replace(QRegExp("  +"), "\n");
                if (m_bFirst){
                    if (!m_label.isEmpty())
                        m_label += "\n";
                    m_label += text;
                }else{
                    QLabel *label = new QLabel(text, this);
                    label->setAlignment(WordBreak);
                    widget = label;
                    bJoin = true;
                }
            }
        }else if (!strcmp(data->Type, "instructions")){
            if (data->Value){
                QString text = i18(data->Value);
                text = text.replace(QRegExp("  +"), "\n");
                if (!m_instruction.isEmpty())
                    m_instruction += "\n";
                m_instruction += text;
            }
        }else if (!strcmp(data->Type, "list-single")){
            CComboBox *box = new CComboBox(this, data->Field);
            int cur = 0;
            int n = 0;
            for (unsigned i = 0; i < data->nOptions; i++){
                const char *label = get_str(data->OptionLabels, i);
                const char *val   = get_str(data->Options, i);
                if (label && val){
                    box->addItem(i18(label), val);
                    if (data->Value && !strcmp(data->Value, val))
                        cur = n;
                    n++;
                }
            }
            box->setCurrentItem(cur);
            widget = box;
        }else if (!strcmp(data->Type, "key")){
            if (data->Value)
                m_key = data->Value;
        }else if (!strcmp(data->Type, "password")){
            widget = new QLineEdit(this, "password");
            static_cast<QLineEdit*>(widget)->setEchoMode(QLineEdit::Password);
            connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
            connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
            set_str(&data->Label, "Password");
        }else if (!strcmp(data->Type, "online")){
            widget = new QCheckBox(this, "online");
            static_cast<QCheckBox*>(widget)->setText(i18n("Online only"));
            bJoin = true;
        }else if (!strcmp(data->Type, "sex")){
            CComboBox *box = new CComboBox(this, data->Field);
            box->addItem("", "0");
            box->addItem(i18n("Male"), "1");
            box->addItem(i18n("Female"), "2");
            set_str(&data->Label, I18N_NOOP("Gender"));
            widget = box;
        }else{
            defFlds *f;
            for (f = fields; f->tag; f++)
                if (!strcmp(data->Type, f->tag))
                    break;
            if (f->tag){
                widget = new QLineEdit(this, f->tag);
                connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
                connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
                if (data->Value && *data->Value)
                    static_cast<QLineEdit*>(widget)->setText(QString::fromUtf8(data->Value));
                set_str(&data->Label, f->name);
                if (f->bRequired && m_bRegister)
                    data->bRequired = true;
            }else if (data->Label){
                widget = new QLineEdit(this, data->Field);
                connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
                connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
                if (data->Value && *data->Value)
                    static_cast<QLineEdit*>(widget)->setText(QString::fromUtf8(data->Value));
            }
        }
    }else{
        if (!m_widgets.empty()){
            unsigned nCols = (m_widgets.size() + MAX_ELEMENTS - 1) / MAX_ELEMENTS;
            unsigned nRows = (m_widgets.size() + nCols - 1) / nCols;
            unsigned start = 0;
            if (!m_label.isEmpty()){
                QLabel *label = new QLabel(m_label, this);
                label->setAlignment(WordBreak);
                lay->addMultiCellWidget(label, 0, 0, 0, nCols * 2 + 1);
                m_label = "";
                start = 1;
            }
            unsigned row = start;
            unsigned col = 0;
            for (unsigned i = 0; i < m_widgets.size(); i++, row++){
                if (row >= nRows + start){
                    row  = 0;
                    col += 2;
                }
                if (m_labels[i]){
                    lay->addWidget(m_labels[i], row, col);
                    lay->addWidget(m_widgets[i], row, col + 1);
                    m_labels[i]->show();
                }else{
                    lay->addMultiCellWidget(m_widgets[i], row, row, col, col + 1);
                }
                m_widgets[i]->show();
            }
            if (!m_instruction.isEmpty()){
                QLabel *label = new QLabel(m_instruction, this);
                label->setAlignment(WordBreak);
                lay->addMultiCellWidget(label, nRows + start, nRows + start, 0, nCols * 2 + 1);
                m_instruction = "";
            }
        }
        m_widgets.clear();
        m_labels.clear();
        m_bDirty = true;
        QTimer::singleShot(0, this, SLOT(setSize()));
        return;
    }
    if (widget){
        m_bFirst = false;
        if (data->bRequired)
            m_required.push_back(widget);
        QLabel *label = NULL;
        if (!bJoin && data->Label){
            label = new QLabel(i18(data->Label), this);
            label->setAlignment(AlignRight);
        }
        m_labels.push_back(label);
        m_widgets.push_back(widget);
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
        I18N_NOOP("Please enter your UIN and password")
        I18N_NOOP("You need a x:data capable client to register.")
        I18N_NOOP("Enter nick you want to register.")
        I18N_NOOP("Complete the form to submit your searchable attributes in the Jabber User Directory")
        I18N_NOOP("Fill in all of the fields to add yourself to the JUD.")
        I18N_NOOP("Fill in a field to search for any matching Jabber User (POSIX regular expressions allowed)")
        I18N_NOOP("Fill in a field to search for any matching Jabber users.")
        I18N_NOOP("To register, please fill out the following form.  Be as accurate as possible to make it easier for people to search for you.")
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
    bool bRes = true;

    QObjectList *l = queryList("QLineEdit");
    QObjectListIt it( *l );
    QObject *obj;

    while ((obj = it.current()) != 0 ){
        QLineEdit *edit = static_cast<QLineEdit*>(obj);
        if (edit->echoMode() == QLineEdit::Password){
            if (edit->text().isEmpty()){
                bRes = false;
                break;
            }
            ++it;
            continue;
        }
        if (edit->text().isEmpty()){
            list<QWidget*>::iterator itw;
            for (itw = m_required.begin(); itw != m_required.end(); ++itw){
                if (*itw == edit)
                    break;
            }
            if (itw != m_required.end()){
                bRes = false;
                break;
            }
        }
        if (!edit->text().isEmpty())
            bRes = true;
        ++it;
    }
    delete l;
    return bRes;
}

QString JabberSearch::condition(bool &bXSearch)
{
	bXSearch = m_bXData;
    QString res;
    if (m_bXData)
        res += "x:data";

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
        if (box->currentText().isEmpty()){
            ++it1;
            continue;
        }
        if (!res.isEmpty())
            res += ";";
        res += box->name();
        res += "=";
        res += quoteChars(box->value(), ";");
        ++it1;
    }
    delete l;

    l = queryList("QCheckBox");
    QObjectListIt it2( *l );
    while ((obj = it2.current()) != 0 ){
        QCheckBox *box = static_cast<QCheckBox*>(obj);
        if (!box->isChecked()){
            ++it2;
            continue;
        }
        if (!res.isEmpty())
            res += ";";
        res += box->name();
        res += "=1";
        ++it2;
    }
    delete l;

    l = queryList("QMultiLineEdit");
    QObjectListIt it3( *l );
    while ((obj = it3.current()) != 0 ){
        QMultiLineEdit *edit = static_cast<QMultiLineEdit*>(obj);
        if (!edit->text().isEmpty()){
            if (!res.isEmpty())
                res += ";";
            res += edit->name();
            res += "=";
            res += quoteChars(edit->text(), ";");
        }
        ++it3;
    }
    delete l;

    if (!m_key.empty()){
        if (!res.isEmpty())
            res += ";";
        res += "key=";
        res += quoteChars(QString::fromUtf8(m_key.c_str()), ";");
    }

    return res;
}

#ifndef WIN32
#include "jabbersearch.moc"
#endif

