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
    client->get_search(jid);
    m_bDirty = false;
}

void *JabberSearch::processEvent(Event *e)
{
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventSearchInfo){
        JabberSearchInfo *data = (JabberSearchInfo*)(e->param());
        if (m_jid == data->ID){
            QWidget *widget = NULL;
            bool bJoin = false;
            if (data->Type){
                if (!strcmp(data->Type, "text-single")){
                    widget = new QLineEdit(this, data->Field);
                    connect(widget, SIGNAL(returnPressed()), m_receiver, SLOT(search()));
                    connect(widget, SIGNAL(textChanged(const QString&)), m_receiver, SLOT(textChanged(const QString&)));
                }else if (!strcmp(data->Type, "fixed")){
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
                    }
                }
                m_nPos++;
                m_bDirty = true;
                QTimer::singleShot(0, this, SLOT(setSize()));
            }
        }
    }
    return NULL;
}

void JabberSearch::setSize()
{
    if (!m_bDirty || (parent() == NULL))
        return;
    m_bDirty = false;
    QSize s = sizeHint();
    setMinimumSize(s);
    for (QObject *p = parent(); p; p = p->parent()){
        if (p->inherits("QTabWidget")){
            QTabWidget *w = static_cast<QTabWidget*>(p);
            s = w->sizeHint();
            w->setMinimumSize(s);
            s = QSize(w->width(), w->height());
            w->changeTab(this, m_name);
            break;
        }
        if (p->parent() == NULL)
            break;
    }
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

