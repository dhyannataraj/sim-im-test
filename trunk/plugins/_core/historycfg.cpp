/***************************************************************************
                          historycfg.cpp  -  description
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

#include "simapi.h"

#if QT_VERSION < 300
#include "qt3/qsyntaxhighlighter.h"
#else
#include <qsyntaxhighlighter.h>
#endif

#include "historycfg.h"
#include "core.h"
#include "textshow.h"
#include "msgview.h"
#include "xsl.h"
#include "ballonmsg.h"

#include <time.h>

#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qlabel.h>
#include <qfile.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtabwidget.h>
#include <qspinbox.h>

#ifdef WIN32
static char STYLES[] = "styles\\";
#else
static char STYLES[] = "styles/";
#endif
static char EXT[]    = ".xsl";

#undef QTextEdit

class XmlHighlighter : public QSyntaxHighlighter
{
public:
XmlHighlighter(QTextEdit *textEdit) : QSyntaxHighlighter(textEdit) {}
    virtual int highlightParagraph( const QString &text, int endStateOfLastPara ) ;
};

const int TEXT			= -2;
const int COMMENT		= 1;
const int TAG			= 2;
const int XML_TAG		= 3;
const int XSL_TAG		= 4;
const int STRING		= 5;
const int XML_STRING	= 6;
const int XSL_STRING	= 7;

const unsigned COLOR_COMMENT	= 0x808080;
const unsigned COLOR_TAG		= 0x008000;
const unsigned COLOR_STRING		= 0x000080;
const unsigned COLOR_XSL_TAG	= 0x800000;
const unsigned COLOR_XML_TAG	= 0x808080;

int XmlHighlighter::highlightParagraph(const QString &s, int state)
{
    int pos = 0;
    for (; pos < (int)(s.length());){
        int n = pos;
        int n1;
        QColor c;
        switch (state){
        case TEXT:
            n = s.find("<", pos);
            if (n == -1){
                n = s.length();
            }else{
                state = TAG;
                if (s.mid(n + 1, 2) == "--"){
                    state = COMMENT;
                }else if (s.mid(n + 1, 4) == "?xml"){
                    state = XML_TAG;
                }else if (s.mid(n + 1, 4) == "xsl:"){
                    state = XSL_TAG;
                }else if (s.mid(n + 1, 5) == "/xsl:"){
                    state = XSL_TAG;
                }
            }
            break;
        case COMMENT:
            n = s.find("-->", pos);
            if (n == -1){
                n = s.length();
            }else{
                state = TEXT;
            }
            c = QColor(COLOR_COMMENT);
            break;
        case TAG:
        case XSL_TAG:
        case XML_TAG:
            switch (state){
            case XSL_TAG:
                c = QColor(COLOR_XSL_TAG);
                break;
            case XML_TAG:
                c = QColor(COLOR_XML_TAG);
                break;
            default:
                c = QColor(COLOR_TAG);
            }
            n = s.find(">", pos);
            n1 = s.find("\"", pos);
            if ((n >= 0) && ((n < n1) || (n1 == -1))){
                state = TEXT;
                n++;
            }else if ((n1 >= 0) && ((n1 < n) || (n == -1))){
                switch (state){
                case XSL_TAG:
                    state = XSL_STRING;
                    break;
                case XML_TAG:
                    state = XML_STRING;
                    break;
                default:
                    state = STRING;
                }
                n = n1;
            }else{
                n = s.length();
            }
            break;
        case STRING:
        case XML_STRING:
        case XSL_STRING:
            n = s.find("\"", pos + 1);
            if (n >= 0){
                switch (state){
                case XML_STRING:
                    state = XML_TAG;
                    break;
                case XSL_STRING:
                    state = XSL_TAG;
                    break;
                default:
                    state = TAG;
                }
                n++;
            }else{
                n = s.length();
            }
            c = QColor(COLOR_STRING);
            break;
        }
        if (n - pos > 0)
            setFormat(pos, n - pos, c);
        pos = n;
    }
    return state;
}

HistoryConfig::HistoryConfig(QWidget *parent)
        : HistoryConfigBase(parent)
{
    chkOwn->setChecked(CorePlugin::m_plugin->getOwnColors());
    chkSmile->setChecked(CorePlugin::m_plugin->getUseSmiles());
    cmbPage->setEditable(true);
    cmbPage->insertItem("100");
    cmbPage->insertItem("50");
    cmbPage->insertItem("25");
    m_cur = -1;
    QLineEdit *edit = cmbPage->lineEdit();
    edit->setValidator(new QIntValidator(1, 500, edit));
    edit->setText(QString::number(CorePlugin::m_plugin->getHistoryPage()));
    QString str1 = i18n("Show %1 messages per page");
    QString str2;
    int n = str1.find("%1");
    if (n >= 0){
        str2 = str1.mid(n + 2);
        str1 = str1.left(n);
    }
    lblPage1->setText(str1);
    lblPage2->setText(str2);
    edtStyle->setWordWrap(QTextEdit::NoWrap);
    edtStyle->setTextFormat(QTextEdit::RichText);
    new XmlHighlighter(edtStyle);
    QStringList styles;
    addStyles(user_file(STYLES).c_str(), true);
    addStyles(app_file(STYLES).c_str(), false);
    fillCombo(CorePlugin::m_plugin->getHistoryStyle());
    connect(cmbStyle, SIGNAL(activated(int)), this, SLOT(styleSelected(int)));
    connect(btnCopy, SIGNAL(clicked()), this, SLOT(copy()));
    connect(btnRename, SIGNAL(clicked()), this, SLOT(rename()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(del()));
    connect(tabStyle, SIGNAL(currentChanged(QWidget*)), this, SLOT(viewChanged(QWidget*)));
    connect(edtStyle, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(chkOwn, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    connect(chkSmile, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    connect(chkDays, SIGNAL(toggled(bool)), this, SLOT(toggledDays(bool)));
    connect(chkSize, SIGNAL(toggled(bool)), this, SLOT(toggledSize(bool)));
    HistoryUserData *data = (HistoryUserData*)(getContacts()->getUserData(CorePlugin::m_plugin->history_data_id));
    chkDays->setChecked(data->CutDays != 0);
    chkSize->setChecked(data->CutSize != 0);
    edtDays->setValue(data->Days);
    edtSize->setValue(data->MaxSize);
    toggledDays(chkDays->isChecked());
    toggledSize(chkSize->isChecked());
}

HistoryConfig::~HistoryConfig()
{
}

void HistoryConfig::apply()
{
    bool bChanged = false;
    for (unsigned i = 0; i < m_styles.size(); i++){
        if (m_styles[i].text.isEmpty() || !m_styles[i].bCustom)
            continue;
        if ((int)i == cmbStyle->currentItem())
            bChanged = true;
        string name = STYLES;
        name += QFile::encodeName(m_styles[i].name);
        name += EXT;
        name = user_file(name.c_str());
        QFile f(QFile::decodeName(name.c_str()));
        if (f.open(IO_WriteOnly | IO_Truncate)){
            string s;
            s = m_styles[i].text.utf8();
            f.writeBlock(s.c_str(), s.length());
        }else{
            log(L_WARN, "Can't create %s", name.c_str());
        }
    }
    int cur = cmbStyle->currentItem();
    if ((cur >= 0) && (m_styles[cur].name != QFile::decodeName(CorePlugin::m_plugin->getHistoryStyle()))){
        CorePlugin::m_plugin->setHistoryStyle(QFile::encodeName(m_styles[cur].name));
        bChanged = true;
    }
    delete CorePlugin::m_plugin->historyXSL;
    CorePlugin::m_plugin->historyXSL = new XSL(m_styles[cur].name);

    if (chkOwn->isChecked() != CorePlugin::m_plugin->getOwnColors()){
        bChanged = true;
        CorePlugin::m_plugin->setOwnColors(chkOwn->isChecked());
    }
    if (chkSmile->isChecked() != CorePlugin::m_plugin->getUseSmiles()){
        bChanged = true;
        CorePlugin::m_plugin->setUseSmiles(chkSmile->isChecked());
    }
    CorePlugin::m_plugin->setHistoryPage(atol(cmbPage->lineEdit()->text().latin1()));
    if (bChanged){
        Event e(EventHistoryConfig);
        e.process();
        fillPreview();
    }
    HistoryUserData *data = (HistoryUserData*)(getContacts()->getUserData(CorePlugin::m_plugin->history_data_id));
    data->CutDays = chkDays->isChecked();
    data->CutSize = chkSize->isChecked();
    data->Days    = atol(edtDays->text());
    data->MaxSize = atol(edtSize->text());
}

void HistoryConfig::addStyles(const char *dir, bool bCustom)
{
    QDir d(QFile::decodeName(dir));
    QStringList files = d.entryList("*.xsl", QDir::Files, QDir::Name);
    for (QStringList::Iterator it = files.begin(); it != files.end(); ++it){
        QString name = *it;
        int n = name.findRev(".");
        name = name.left(n);
        vector<StyleDef>::iterator its;
        for (its = m_styles.begin(); its != m_styles.end(); ++its){
            if (name == (*its).name)
                break;
        }
        if (its == m_styles.end()){
            StyleDef s;
            s.name    = name;
            s.bCustom = bCustom;
            m_styles.push_back(s);
        }
    }
}

void HistoryConfig::toggled(bool)
{
    if (tabStyle->currentPage() == preview)
        fillPreview();
}

void HistoryConfig::styleSelected(int n)
{
    if (n == m_cur)
        return;
	if (m_styles.size() == 0) return;
    if (m_bDirty && (m_cur >= 0))
        m_styles[m_cur].text = unquoteText(edtStyle->text());
    m_cur = n;
    bool bCustom = m_styles[n].bCustom;
    btnRename->setEnabled(bCustom);
    btnDelete->setEnabled(bCustom);
    edtStyle->setReadOnly(!bCustom);
    fillPreview();
}

void HistoryConfig::copy()
{
    int cur = cmbStyle->currentItem();
    if (cur < 0)
        return;
    QString name    = m_styles[cur].name;
    QString newName;
    QRegExp re("\\.[0-9]+$");
    unsigned next = 0;
    for (vector<StyleDef>::iterator it = m_styles.begin(); it != m_styles.end(); ++it){
        QString nn = (*it).name;
        int n = nn.find(re);
        if (n < 0)
            continue;
        nn = nn.mid(n + 1);
        next = QMAX(next, nn.toUInt());
    }
    int nn = name.find(re);
    if (nn >= 0){
        newName = name.left(nn);
    }else{
        newName = name;
    }
    newName += ".";
    newName += QString::number(next + 1);
    string n;
    n = STYLES;
    n += QFile::encodeName(name);
    n += EXT;
    if (m_styles[cur].bCustom){
        n = user_file(n.c_str());
    }else{
        n = app_file(n.c_str());
    }
    QFile from(QFile::decodeName(n.c_str()));
    if (!from.open(IO_ReadOnly)){
        log(L_WARN, "Can't open %s", n.c_str());
        return;
    }
    n = STYLES;
    n += QFile::encodeName(newName);
    n += EXT;
    n = user_file(n.c_str());
    QFile to(QFile::decodeName(n.c_str()));
    if (!to.open(IO_WriteOnly | IO_Truncate)){
        log(L_WARN, "Cam't create %s", n.c_str());
        return;
    }
    string s;
    s.append(from.size(), '\x00');
    from.readBlock((char*)(s.c_str()), from.size());
    to.writeBlock(s.c_str(), s.length());
    from.close();
    to.close();
    s = "";
    StyleDef d;
    d.name    = newName;
    d.bCustom = true;
    m_styles.push_back(d);
    fillCombo(QFile::encodeName(newName));
}

void HistoryConfig::fillCombo(const char *current)
{
    sort(m_styles.begin(), m_styles.end());
    unsigned cur = 0;
    cmbStyle->clear();
    for (unsigned i = 0; i < m_styles.size(); i++){
        QString name = m_styles[i].name;
        cmbStyle->insertItem(name);
        if (name == QFile::decodeName(current))
            cur = i;
    }
    cmbStyle->setCurrentItem(cur);
    styleSelected(cur);
}

void HistoryConfig::del()
{
    int cur = cmbStyle->currentItem();
    if (!m_styles[cur].bCustom)
        return;
    BalloonMsg::ask(NULL, i18n("Remove style '%1'?") .arg(m_styles[cur].name),
                    btnDelete, SLOT(realDelete()), NULL, NULL, this);
}

void HistoryConfig::realDelete()
{
    int cur = cmbStyle->currentItem();
    if (cur < 0)
        return;
    if (!m_styles[cur].bCustom)
        return;
    QString name = m_styles[cur].name;
    vector<StyleDef>::iterator it;
    for (it = m_styles.begin(); it != m_styles.end(); ++it)
        if (cur-- == 0)
            break;
    m_styles.erase(it);
    string n;
    n = STYLES;
    n += QFile::encodeName(name);
    n += EXT;
    n = user_file(n.c_str());
    QFile::remove(QFile::decodeName(n.c_str()));
    fillCombo(CorePlugin::m_plugin->getHistoryStyle());
}

void HistoryConfig::rename()
{
    int cur = cmbStyle->currentItem();
    if (!m_styles[cur].bCustom)
        return;
    cmbStyle->setEditable(true);
    cmbStyle->lineEdit()->setText(m_styles[cur].name);
    cmbStyle->lineEdit()->setFocus();
    cmbStyle->lineEdit()->installEventFilter(this);
}

void HistoryConfig::cancelRename()
{
    cmbStyle->lineEdit()->removeEventFilter(this);
    cmbStyle->setEditable(false);
}

void HistoryConfig::realRename()
{
    int cur = cmbStyle->currentItem();
    if (cur < 0)
        return;
    QString newName = cmbStyle->lineEdit()->text();
    cmbStyle->lineEdit()->removeEventFilter(this);
    cmbStyle->setEditable(false);
    if (newName != m_styles[cur].name){
        int n = 0;
        vector<StyleDef>::iterator it;
        for (it = m_styles.begin(); it != m_styles.end(); ++it, n++){
            if ((*it).name == newName){
                if (n < cur)
                    cur--;
                m_styles.erase(it);
                break;
            }
        }

        string nn;
        nn = STYLES;
        nn += QFile::encodeName(m_styles[cur].name);
        nn += EXT;
        nn = user_file(nn.c_str());
        if (m_styles[cur].text.isEmpty()){
            QFile f(QFile::decodeName(nn.c_str()));
            if (f.open(IO_ReadOnly)){
                string s;
                s.append(f.size(), '\x00');
                f.readBlock((char*)(s.c_str()), f.size());
                m_styles[cur].text = QString::fromUtf8(s.c_str());
            }
        }
        QFile::remove(QFile::decodeName(nn.c_str()));
        m_styles[cur].name = newName;
    }
    fillCombo(newName);
}

bool HistoryConfig::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusOut){
        QTimer::singleShot(0, this, SLOT(realRename()));
    }
    if (e->type() == QEvent::KeyPress){
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        switch (ke->key()){
        case Key_Enter:
        case Key_Return:
            QTimer::singleShot(0, this, SLOT(realRename()));
            break;
        case Key_Escape:
            QTimer::singleShot(0, this, SLOT(cancelRename()));
            break;
        }
    }
    return HistoryConfigBase::eventFilter(o, e);
}

void HistoryConfig::viewChanged(QWidget *w)
{
    int cur = cmbStyle->currentItem();
    if (cur < 0)
        return;
    if (w == preview){
        if (!m_styles[cur].bCustom)
            return;
        if (m_bDirty){
            m_styles[cur].text = unquoteText(edtStyle->text());
            fillPreview();
        }
    }else{
        QString xsl;
        if (m_styles[cur].text.isEmpty()){
            string name = STYLES;
            name += QFile::encodeName(m_styles[cur].name);
            name += EXT;
            name = m_styles[cur].bCustom ? user_file(name.c_str()) : app_file(name.c_str());
            QFile f(QFile::decodeName(name.c_str()));
            if (f.open(IO_ReadOnly)){
                name = "";
                name.append(f.size(), '\x00');
                f.readBlock((char*)(name.c_str()), f.size());
                xsl = QString::fromUtf8(name.c_str());
            }else{
                log(L_WARN, "Can't open %s", name.c_str());
            }
        }else{
            xsl = m_styles[cur].text;
        }
        edtStyle->setText(quoteString(xsl));
        QTimer::singleShot(0, this, SLOT(sync()));
    }
}

void HistoryConfig::sync()
{
    edtStyle->sync();
}

void HistoryConfig::textChanged()
{
    m_bDirty = true;
}

void HistoryConfig::fillPreview()
{
    m_bDirty = false;
    int cur = cmbStyle->currentItem();
    if (cur < 0)
        return;
    XSL *xsl = new XSL(m_styles[cur].name);
    if (!m_styles[cur].text.isEmpty())
        xsl->setXSL(m_styles[cur].text);
    Contact *contact = getContacts()->contact(0, true);
    contact->setName("Buddy");
    contact->setTemporary(true);
    edtPreview->clear();
    edtPreview->setXSL(xsl);
    time_t now;
    time(&now);
    bool saveSmiles = CorePlugin::m_plugin->getUseSmiles();
    bool saveOwn    = CorePlugin::m_plugin->getOwnColors();
    CorePlugin::m_plugin->setUseSmiles(chkSmile->isChecked());
    CorePlugin::m_plugin->setOwnColors(chkOwn->isChecked());
    Message m1;
    m1.setId((unsigned)(-1));
    m1.setFlags(MESSAGE_RECEIVED | MESSAGE_LIST);
    m1.setText(i18n("Hi! ;)"));
    m1.setTime(now - 360);
    m1.setContact(contact->id());
    edtPreview->addMessage(&m1);
    Message m2;
    m2.setId((unsigned)(-2));
    m2.setText(i18n("Hi!"));
    m2.setTime(now - 300);
    m2.setContact(contact->id());
    edtPreview->addMessage(&m2);
    Message m3;
    m3.setId((unsigned)(-3));
    m3.setText(i18n("<b><font color=\"#FF0000\">C</font><font color=\"#FFFF00\">olored</font></b> message"));
    m3.setTime(now - 120);
    m3.setFlags(MESSAGE_SECURE | MESSAGE_URGENT | MESSAGE_RICHTEXT);
    m3.setBackground(0xC0C0C0);
    m3.setForeground(0x008000);
    m3.setContact(contact->id());
    edtPreview->addMessage(&m3);
    Message m4;
    m4.setId((unsigned)(-4));
    m4.setText(i18n("New message"));
    m4.setFlags(MESSAGE_RECEIVED);
    m4.setTime(now - 60);
    m4.setContact(contact->id());
    edtPreview->addMessage(&m4, true);
    StatusMessage m5;
    m5.setId((unsigned)(-5));
    m5.setStatus(STATUS_OFFLINE);
    m5.setTime(now);
    m5.setContact(contact->id());
    if (getContacts()->nClients())
        m5.setClient((getContacts()->getClient(0)->name() + ".").c_str());
    edtPreview->addMessage(&m5);
    delete contact;
    CorePlugin::m_plugin->setUseSmiles(saveSmiles);
    CorePlugin::m_plugin->setOwnColors(saveOwn);
}

void HistoryConfig::toggledDays(bool bState)
{
    lblDays->setEnabled(bState);
    lblDays1->setEnabled(bState);
    edtDays->setEnabled(bState);
}

void HistoryConfig::toggledSize(bool bState)
{
    lblSize->setEnabled(bState);
    lblSize1->setEnabled(bState);
    edtSize->setEnabled(bState);
}

#ifndef WIN32
#include "historycfg.moc"
#endif

