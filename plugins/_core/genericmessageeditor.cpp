/*
 * genericmessageeditor.cpp
 *
 *  Created on: Aug 17, 2011
 */

#include "genericmessageeditor.h"

#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QFontDialog>
#include <QToolBar>
#include <algorithm>

#include "core.h"
#include "simapi.h"
#include "log.h"
#include "contacts/contact.h"
#include "contacts/imcontact.h"
#include "contacts/client.h"
#include "imagestorage/imagestorage.h"

using SIM::log;
using SIM::L_DEBUG;

GenericMessageEditor::GenericMessageEditor(QWidget* parent) : SIM::MessageEditor(parent)
{
}

GenericMessageEditor::~GenericMessageEditor()
{
}

QString GenericMessageEditor::messageTypeId() const
{
    return "generic";
}

QColor GenericMessageEditor::colorFromDialog(QString oldColorName) //reimplement with small ColorPicker...
{
    return QColorDialog::getColor(QColor(oldColorName), m_edit);
}

void GenericMessageEditor::chooseBackgroundColor()
{
    QColor color = colorFromDialog(m_bgColorName);
    if(!color.isValid())
        return;
    m_bgColorName = color.name();
    log(L_DEBUG, color.name());
    m_edit->setStyleSheet(getBGStyleSheet(color.name()));

}

void GenericMessageEditor::chooseForegroundColor()
{
    QColor color = colorFromDialog(m_txtColorName);
    if(!color.isValid())
        return;
    m_txtColorName = color.name();
    log(L_DEBUG, color.name());
    m_edit->setTextColor(color);

    //hub->setValue("msgedit/textcolor", QVariant(color)); //It should be done when window closes, so you should do this from closeEvent of Container

}

QString GenericMessageEditor::getBGStyleSheet(QString bgColorName)
{
    return QString("QTextEdit {background-color: %1; border: 1px solid black; border-radius: 5px; margin-top: 7px; margin-bottom: 7px; padding: 0px;}").arg(bgColorName);
}
void GenericMessageEditor::setBold(bool b)
{
    m_edit->setFontWeight(b ? QFont::Bold : QFont::Normal);
}

void GenericMessageEditor::setItalic(bool b)
{
    m_edit->setFontItalic(b);
}

void GenericMessageEditor::setUnderline(bool b)
{
    m_edit->setFontUnderline(b);
}

void GenericMessageEditor::insertSmile() //Todo
{

}

void GenericMessageEditor::setTranslit(bool on) //Todo
{

}

void GenericMessageEditor::setTranslateIncomming(bool on) //Todo
{

}

void GenericMessageEditor::setTranslateOutgoing(bool on) //Todo
{

}

void GenericMessageEditor::chooseFont()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, m_edit->font(), m_edit);
    if(!ok)
        return;
    m_edit->setFont(f);
}

void GenericMessageEditor::setCloseOnSend(bool b)
{

}

void GenericMessageEditor::send()
{
    QString text = m_edit->document()->toPlainText();
    if(text.isEmpty())
        return;

    m_edit->clear();

    //emit messageSendRequested(text);
}

void GenericMessageEditor::textChanged()
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

void GenericMessageEditor::cursorPositionChanged()
{
    QTextCharFormat currentFormat = m_edit->textCursor().charFormat();
    foreach(QAction* a, m_bar->actions())
    {
        // FIXME shouldn't depend on actions text
        if (a->text() == I18N_NOOP("&Bold"))
            a->setChecked(currentFormat.fontWeight() == QFont::Bold);

        if (a->text() == I18N_NOOP("&Italic"))
            a->setChecked(currentFormat.fontItalic());

        if (a->text() == I18N_NOOP("&Underline"))
            a->setChecked(currentFormat.fontUnderline());

    }
}


QToolBar* GenericMessageEditor::createToolBar()
{
    QToolBar* bar = new QToolBar(this);
    bar->setFloatable(true);
    bar->setMovable(true);

    bar->setAllowedAreas(Qt::TopToolBarArea & Qt::BottomToolBarArea);


    //fixme: the following should be made generic, f.e. for toolbar changes in icon-positioning...

    bar->addSeparator();
    bar->addAction(SIM::getImageStorage()->icon("bgcolor"), I18N_NOOP("Back&ground color"), this, SLOT(chooseBackgroundColor()));
    bar->addAction(SIM::getImageStorage()->icon("fgcolor"), I18N_NOOP("Fo&reground color"), this, SLOT(chooseForegroundColor()));

    QAction* bold = bar->addAction(SIM::getImageStorage()->icon("text_bold"), I18N_NOOP("&Bold"), this, SLOT(setBold(bool)));
    bold->setCheckable(true);

    QAction* italic = bar->addAction(SIM::getImageStorage()->icon("text_italic"), I18N_NOOP("&Italic"), this, SLOT(setItalic(bool)));
    italic->setCheckable(true);

    QAction* underline = bar->addAction(SIM::getImageStorage()->icon("text_under"), I18N_NOOP("&Underline"), this, SLOT(setUnderline(bool)));
    underline->setCheckable(true);

    bar->addAction(SIM::getImageStorage()->icon("text"), I18N_NOOP("Select f&ont"), this, SLOT(chooseFont()));

    bar->addSeparator();

    QAction* emoticons = bar->addAction(SIM::getImageStorage()->icon("smile"), I18N_NOOP("I&nsert smile"), this, SLOT(insertSmile()));

    QAction* translit = bar->addAction(SIM::getImageStorage()->icon("translit"), I18N_NOOP("Send in &translit"), this, SLOT(setTranslit(bool)));
    translit->setCheckable(true);

    bar->addSeparator();

    QAction* closeAfterSend = bar->addAction(SIM::getImageStorage()->icon("fileclose"), I18N_NOOP("C&lose after send"), this, SLOT(setCloseOnSend(bool)));
    closeAfterSend->setCheckable(true);
    bar->addSeparator();
    //m_sendAction = bar->addAction(getImageStorage()->icon("mail_generic"), I18N_NOOP("&Send"), this, SLOT(send()));


    m_cmdSend = new QToolButton(m_edit);
    connect(m_cmdSend, SIGNAL(clicked()), this, SLOT(send()));

    m_cmdSend->setIcon(SIM::getImageStorage()->icon("mail_generic"));
    m_cmdSend->setText(i18n("&Send"));
    m_cmdSend->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_cmdSend->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->addWidget(m_cmdSend);

    bar->addSeparator();

    m_sendMultiple = bar->addAction(SIM::getImageStorage()->icon("1rightarrow"), I18N_NOOP("Send to &multiple"), this, SLOT(sendMultiple(bool)));
    m_sendMultiple->setCheckable(true);

    return bar;
}

