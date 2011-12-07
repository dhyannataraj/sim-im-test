/***************************************************************************
                          msgedit.cpp  -  description
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

#include "msgedit.h"

#include "simapi.h"
#include "contacts/contact.h"
#include "contacts/imcontact.h"
#include "contacts/client.h"
#include "imagestorage/imagestorage.h"

#include "container/userwnd.h"

#include "log.h"
#include "container/container.h"

#include <algorithm>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QFontDialog>
#include <QToolBar>
#include <algorithm>

using namespace std;
using namespace SIM;

const unsigned NO_TYPE = (unsigned)(-1);

MsgTextEdit::MsgTextEdit(MsgEdit *edit, QWidget *parent)
        //: TextEdit(parent)
{
    m_edit = edit;
//    setBackground(CorePlugin::instance()->value("EditBackground").toUInt());
//    setForeground(CorePlugin::instance()->value("EditForeground").toUInt(), true);
#if defined(USE_KDE)
#if KDE_IS_VERSION(3,2,0)
    setCheckSpellingEnabled(CorePlugin::instance()->getEnableSpell());
#endif
#endif
}

void MsgTextEdit::contextMenuEvent(QContextMenuEvent* event)
{
//    if (m_bInClick)
//        return;

//    m_popupPos = event->pos();

//    Command cmd;

//    cmd->popup_id	= MenuTextEdit;
//    cmd->param		= (TextEdit*)this;
//    cmd->flags		= COMMAND_NEW_POPUP;
//    EventMenuGet e(cmd);
//    e.process();
//    if(e.menu())
//        e.menu()->exec(event->globalPos());
}

//Message *MsgTextEdit::createMessage(QMimeSource *src)
//{
//    Message *msg = NULL;
//    CommandDef *cmd;
//    CommandsMapIterator it(CorePlugin::instance()->messageTypes);
//    while ((cmd = ++it) != NULL){
//        MessageDef *def = (MessageDef*)(cmd->param);
//        if (def && def->drag){
//            msg = def->drag(src);
//            if (msg){
//                Command c;
//                c->id      = cmd->id;
//                c->menu_id = MenuMessage;
//                c->param	 = (void*)(m_edit->m_userWnd->id());
//                if (EventCheckCommandState(c).process())
//                    break;
//                delete msg;
//                msg = NULL;
//            }
//        }
//    }
//    return msg;
//}

void MsgTextEdit::contentsDropEvent(QDropEvent *e)
{
//    Message *msg = createMessage(e);
//    if (msg){
//        e->accept();
//        msg->setContact(m_edit->m_userWnd->id());
//        EventOpenMessage(msg).process();
//        delete msg;
//        return;
//    }
    //TextEdit::contentsDropEvent(e); //FIXME
}

void MsgTextEdit::contentsDragEnterEvent(QDragEnterEvent *e)
{
//    Message *msg = createMessage(e);
//    if (msg){
//        delete msg;
//        e->accept();
//        return;
//    }
    //TextEdit::contentsDragEnterEvent(e);  //FIXME
}

void MsgTextEdit::contentsDragMoveEvent(QDragMoveEvent *e)
{
//    Message *msg = createMessage(e);
//    if (msg){
//        delete msg;
//        e->accept();
//        return;
//    }
    //TextEdit::contentsDragMoveEvent(e); //FIXME
}

MsgEdit::MsgEdit(QWidget* parent) 
    : QFrame(parent)
    , m_bTranslationService(true) //#Todo later from config...
    , m_editTrans(NULL)
    , m_editActive(NULL)
{
//    m_msg		= NULL;
//    m_bTyping	= false;
//    m_type		= NO_TYPE;
//    m_flags		= 0;
//    m_retry.msg = NULL;
//    m_bReceived = false;
//    m_processor = NULL;
//    m_recvProcessor = NULL;
//    m_cmd.param = NULL;

//    connect(CorePlugin::instance(), SIGNAL(modeChanged()), this, SLOT(modeChanged()));

    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);

    m_edit = new QTextEdit(this);
    
    
//    m_edit->setBackground(QColor(CorePlugin::instance()->value("EditBackground").toUInt() & 0xFFFFFF));
//    m_edit->setBackground(QColor(255, 255, 255));
//    m_edit->setForeground(QColor(CorePlugin::instance()->value("EditForeground").toUInt() & 0xFFFFFF), true);
//    m_edit->setFont(CorePlugin::instance()->editFont);
//    m_edit->setCtrlMode(!CorePlugin::instance()->value("SendOnEnter").toBool());
//    m_edit->setParam(this);
    setFocusProxy(m_edit);


    connect(m_edit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));
//    connect(m_edit, SIGNAL(lostFocus()), this, SLOT(editLostFocus()));
//    connect(m_edit, SIGNAL(textChanged()), this, SLOT(editTextChanged()));
//    connect(m_edit, SIGNAL(ctrlEnterPressed()), this, SLOT(editEnterPressed()));
//    connect(m_edit, SIGNAL(colorsChanged()), this, SLOT(colorsChanged()));
//    connect(m_edit, SIGNAL(finished()), this, SLOT(editFinished()));
//    connect(m_edit, SIGNAL(fontSelected(const QFont&)), this, SLOT(editFontChanged(const QFont&)));

    QFontMetrics fm(m_edit->font());
    m_edit->setMinimumSize(QSize(fm.maxWidth(), fm.height() + 10));

    m_bar = createToolBar();
    m_layout->addWidget(m_bar);

    
    if (m_bTranslationService)
    {
        QTextEdit * m_editTrans = new QTextEdit(this);
        m_layout->addWidget(m_editTrans);
    }
    m_editActive=&(*m_edit);
    m_layout->addWidget(m_edit);
    connect(m_edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    textChanged();
    
//    if (CorePlugin::instance()->getContainerMode() == 0)
//        showCloseSend(false);
}

MsgEdit::~MsgEdit()
{
//    typingStop();
//    editLostFocus();
//    if (m_retry.msg)
//        delete m_retry.msg;
//    emit finished();
}

void MsgEdit::setUserWnd(UserWnd* wnd)
{
    m_userWnd = wnd;
}

QString MsgEdit::messageHtml() const
{
    return m_edit->toHtml();
}

QToolBar* MsgEdit::createToolBar()
{
    QToolBar* bar = new QToolBar(this); //Memleak
    bar->setFloatable(true);
    bar->setMovable(true);
    
    bar->setAllowedAreas(Qt::TopToolBarArea	& Qt::BottomToolBarArea);
    

    //fixme: the following should be made generic, f.e. for toolbar changes in icon-positioning...

    bar->addSeparator();
    bar->addAction(getImageStorage()->icon("bgcolor"), I18N_NOOP("Back&ground color"), this, SLOT(chooseBackgroundColor()));
    bar->addAction(getImageStorage()->icon("fgcolor"), I18N_NOOP("Fo&reground color"), this, SLOT(chooseForegroundColor()));

    QAction* bold = bar->addAction(getImageStorage()->icon("text_bold"), I18N_NOOP("&Bold"), this, SLOT(setBold(bool)));
    bold->setCheckable(true);

    QAction* italic = bar->addAction(getImageStorage()->icon("text_italic"), I18N_NOOP("&Italic"), this, SLOT(setItalic(bool)));
    italic->setCheckable(true);

    QAction* underline = bar->addAction(getImageStorage()->icon("text_under"), I18N_NOOP("&Underline"), this, SLOT(setUnderline(bool)));
    underline->setCheckable(true);

    bar->addAction(getImageStorage()->icon("text"), I18N_NOOP("Select f&ont"), this, SLOT(chooseFont()));

    bar->addSeparator();

    QAction* emoticons = bar->addAction(getImageStorage()->icon("smile"), I18N_NOOP("I&nsert smile"), this, SLOT(insertSmile()));
    
    QAction* translit = bar->addAction(getImageStorage()->icon("translit"), I18N_NOOP("Send in &translit"), this, SLOT(setTranslit(bool)));
    translit->setCheckable(true);




    
    if (m_bTranslationService) 
    {
        bar->addSeparator();

        QAction* incommingTranslation = bar->addAction(getImageStorage()->icon("translate"), I18N_NOOP("OTRT-Incomming:"), this, SLOT(setTranslateOutgoing(bool))); //Todo create Icon
        incommingTranslation->setCheckable(true);

        m_cmbLanguageIncomming = new QComboBox(m_edit);  //Todo: Implement language selection for the language it should automatically translated...
        //fillLangs(); //Todo Fill cmbBox with languages
        m_cmbLanguageIncomming->setToolTip(i18n("Select translation language for incomming messages"));
        bar->addWidget(m_cmbLanguageIncomming);



        bar->addSeparator();

        QAction* outgoingTranslation = bar->addAction(getImageStorage()->icon("translator"), I18N_NOOP("OTRT-Outgoing:"), this, SLOT(setTranslateIncomming(bool))); //Todo create Icon
        outgoingTranslation->setCheckable(true);

        m_cmbLanguageOutgoing = new QComboBox(m_edit);  //Todo: Implement language selection for the language it should automatically translated...
        //fillLangs(); //Todo Fill cmbBox with languages
        m_cmbLanguageOutgoing->setToolTip(i18n("Select translation language for outgoing messages"));
        bar->addWidget(m_cmbLanguageOutgoing);

        //Translations - How to do:
        //register for an api-key: https://code.google.com/apis/console/

        //Get the translated string:
        //GET https://www.googleapis.com/language/translate/v2?q=%3Ch1%3EDas%20ist%20ein%20Text.%3C%2Fh1%3E&target=en&format=html&pp=1&key={YOUR_API_KEY}
        
        //Doc for implementation and testing: https://code.google.com/apis/explorer/#_s=translate&_v=v2&_m=translations.list&q=%3Ch1%3EDas%20ist%20ein%20Text.%3C/h1%3E&target=en&cid=blub&format=html
    }
    else 
    {
        //trEdit->setVisible(false); //How to do it best?
    }




    bar->addSeparator();

    QAction* closeAfterSend = bar->addAction(getImageStorage()->icon("fileclose"), I18N_NOOP("C&lose after send"), this, SLOT(setCloseOnSend(bool)));
    closeAfterSend->setCheckable(true);
    bar->addSeparator();
    //m_sendAction = bar->addAction(getImageStorage()->icon("mail_generic"), I18N_NOOP("&Send"), this, SLOT(send()));
    

    m_cmdSend = new QToolButton(m_edit);
    connect(m_cmdSend, SIGNAL(clicked()), this, SLOT(send()));

    m_cmdSend->setIcon(getImageStorage()->icon("mail_generic"));
    m_cmdSend->setText(i18n("&Send"));
    m_cmdSend->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_cmdSend->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->addWidget(m_cmdSend);
    
    bar->addSeparator();
    
    m_sendMultiple = bar->addAction(getImageStorage()->icon("1rightarrow"), I18N_NOOP("Send to &multiple"), this, SLOT(sendMultiple(bool)));
    m_sendMultiple->setCheckable(true);

    return bar;
}

QColor MsgEdit::colorFromDialog(QString oldColorName) //reimplement with small ColorPicker...
{
    return QColorDialog::getColor( QColor(oldColorName), m_edit);
}

void MsgEdit::chooseBackgroundColor()
{
    QColor color = colorFromDialog(m_bgColorName);
    if (!color.isValid()) return;
    m_bgColorName=color.name();
    log(L_DEBUG, color.name());
    m_edit->setStyleSheet(getBGStyleSheet(color.name()));

}

void MsgEdit::chooseForegroundColor()
{
    QColor color = colorFromDialog(m_txtColorName);
    if (!color.isValid()) return;
    m_txtColorName=color.name();
    log(L_DEBUG, color.name());
    m_edit->setTextColor(color);
    
    //hub->setValue("msgedit/textcolor", QVariant(color)); //It should be done when window closes, so you should do this from closeEvent of Container

}

QString MsgEdit::getBGStyleSheet(QString bgColorName)
{
    return QString("QTextEdit {background-color: %1; border: 1px solid black; border-radius: 5px; margin-top: 7px; margin-bottom: 7px; padding: 0px;}").arg(bgColorName);
}
void MsgEdit::setBold(bool b)
{
    m_edit->setFontWeight ( b ? QFont::Bold : QFont::Normal );
}

void MsgEdit::setItalic(bool b)
{
    m_edit->setFontItalic (b);
}

void MsgEdit::setUnderline(bool b)
{
    m_edit->setFontUnderline (b);
}

void MsgEdit::insertSmile() //Todo
{

}

void MsgEdit::setTranslit(bool on) //Todo
{
    
}

void MsgEdit::setTranslateIncomming(bool on) //Todo
{
    
}

void MsgEdit::setTranslateOutgoing(bool on) //Todo
{
    
}

void MsgEdit::chooseFont()
{
    bool ok=false;
    QFont f = QFontDialog::getFont(&ok, m_edit->font(),m_edit);
    if (!ok) return;
    m_edit->setFont(f);
}

void MsgEdit::setCloseOnSend(bool b)
{
    
}

void MsgEdit::send()
{
    QString text = m_edit->document()->toPlainText();
    if(text.isEmpty())
        return;

    m_edit->clear();

    emit messageSendRequested(text);
}

void MsgEdit::textChanged()
{
    if(m_edit->toPlainText().isEmpty() &&
            m_cmdSend->isEnabled())
    {
        m_cmdSend->setEnabled(false);
    }
    else if(!m_edit->toPlainText().isEmpty() &&
            !m_cmdSend->isEnabled())
    {
        m_cmdSend->setEnabled(true);
    }
}

void MsgEdit::cursorPositionChanged()
{   
    QTextCharFormat currentFormat = m_edit->textCursor().charFormat();
    Q_FOREACH(QAction * a, m_bar->actions())
    {

        if (a->text()==I18N_NOOP("&Bold"))
            a->setChecked(currentFormat.fontWeight() == QFont::Bold);
        
        if (a->text()==I18N_NOOP("&Italic"))
            a->setChecked(currentFormat.fontItalic());

        if (a->text()==I18N_NOOP("&Underline"))
            a->setChecked(currentFormat.fontUnderline());

    }

}

void MsgEdit::sendMultiple(bool on)
{
    m_sendMultiple->setIcon(getImageStorage()->icon(on ? "1leftarrow" : "1rightarrow"));
}


