/***************************************************************************
                          textshow.cpp  -  description
                             -------------------
    begin                : Sat Mar 16 2002
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

#include "textshow.h"
#include "toolbtn.h"
#include "html.h"
#include "simapi.h"

#ifdef USE_KDE
#include <keditcl.h>
#include <kstdaccel.h>
#include <kglobal.h>
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#ifdef HAVE_KROOTPIXMAP_H
#include <krootpixmap.h>
#endif
#else
#include <qfiledialog.h>
#endif

#ifdef USE_KDE
#include <kcolordialog.h>
#include <kfontdialog.h>
#else
#include <qcolordialog.h>
#include <qfontdialog.h>
#endif

#include <qdatetime.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qobjectlist.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <qstringlist.h>
#include <qtextcodec.h>
#include <qtoolbar.h>
#include <qlineedit.h>
#include <qaccel.h>
#include <qdragobject.h>
#include <qtoolbutton.h>
#include <qstatusbar.h>
#include <qtooltip.h>
#include <qlayout.h>

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4786)
#endif
#endif

#define MAX_HISTORY	100

TextEdit::TextEdit(QWidget *p, const char *name)
        : TextShow(p, name)
{
    setReadOnly(false);
    curFG = colorGroup().color(QColorGroup::Text);
    m_bCtrlMode = true;
    setWordWrap(WidgetWidth);
#if QT_VERSION >= 0x030100
    setAutoFormatting(0);
#endif
    viewport()->installEventFilter(this);
}

TextEdit::~TextEdit()
{
}

bool TextEdit::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::FocusOut){
        emit lostFocus();
    }
    return QTextEdit::eventFilter(o, e);
}

void TextEdit::changeText()
{
    emit textChanged();
}

void TextEdit::setCtrlMode(bool mode)
{
    m_bCtrlMode = mode;
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (((e->key() == Key_Enter) || (e->key() == Key_Return))){
        if (!m_bCtrlMode || (e->state() & ControlButton)){
            emit ctrlEnterPressed();
            return;
        }
    }
    if (!isReadOnly()){
        if ((e->state() == ShiftButton) && (e->key() == Key_Insert)){
            paste();
            return;
        }
        if ((e->state() == ControlButton) && (e->key() == Key_Delete)){
            cut();
            return;
        }
    }
#if (QT_VERSION >= 300) && (QT_VERSION < 0x030100)
    // Workaround about autoformat feature in qt 3.0.x
    if ((e->text()[0] == '-') || (e->text()[0] == '*')){
        if (isOverwriteMode() && !hasSelectedText())
            moveCursor(MoveForward, true);
        insert( e->text(), TRUE, FALSE );
        return;
    }
#endif
#if QT_VERSION >= 300
    if ((e->key() == Key_Return) || (e->key() == Key_Enter)){
        QKeyEvent e1(QEvent::KeyPress, e->key(), e->ascii(), e->state() | ControlButton, e->text(), e->count());
        QTextEdit::keyPressEvent(&e1);
        return;
    }
#endif
    TextShow::keyPressEvent(e);
}

void TextEdit::setBackground(const QColor& c)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Base, c);
    pal.setColor(QPalette::Inactive, QColorGroup::Base, c);
    pal.setColor(QPalette::Disabled, QColorGroup::Base, c);
    setPalette(pal);
}

void TextEdit::setForeground(const QColor& c)
{
    curFG = c;
    setColor(c);
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Text, c);
    setPalette(pal);
}

const QColor &TextEdit::background() const
{
    return palette().color(QPalette::Active, QColorGroup::Base);
}

const QColor &TextEdit::foreground() const
{
    return curFG;
}

void TextEdit::setTextFormat(QTextEdit::TextFormat format)
{
    if (format == textFormat())
        return;
    if (format == RichText){
        QTextEdit::setTextFormat(format);
        return;
    }
    QString t = plainText(0, paragraphs(), 0, 0);
    QTextEdit::setTextFormat(format);
    setText(t);
}

TextShow::TextShow(QWidget *p, const char *name)
        : QTextBrowser(p, name)
{
    setTextFormat(RichText);
    setReadOnly(true);
}

TextShow::~TextShow()
{
}

void TextShow::setSource(const QString &href)
{
    QCString s = href.local8Bit();
    Event e(EventGoURL, (void*)(const char*)s);
    e.process();
}

void TextShow::resizeEvent(QResizeEvent *e)
{
    QPoint p = QPoint(0, height());
    p = mapToGlobal(p);
    p = viewport()->mapFromGlobal(p);
    int x, y;
    viewportToContents(p.x(), p.y(), x, y);
    int para;
    int pos = charAt(QPoint(x, y), &para);
    QTextEdit::resizeEvent(e);
    if (pos == -1){
        scrollToBottom();
    }else{
        setCursorPosition(para, pos);
        ensureCursorVisible();
    }
}

void TextShow::keyPressEvent(QKeyEvent *e)
{
    if (((e->state() == Qt::ControlButton) && (e->key() == Qt::Key_C)) ||
            ((e->state() == ControlButton) && (e->key() == Key_Insert))){
        copy();
        return;
    }
    QTextBrowser::keyPressEvent(e);
}

void TextShow::copy()
{
#if QT_VERSION <= 0x030100
    QApplication::clipboard()->setText(selectedText());
#else
    QApplication::clipboard()->setText(selectedText(),QClipboard::Selection);
#endif
}

void TextShow::startDrag()
{
    QDragObject *drag = new QTextDrag(selectedText(), viewport());
    if ( isReadOnly() ) {
        drag->dragCopy();
    } else {
        if ( drag->drag() && QDragObject::target() != this && QDragObject::target() != viewport() )
            removeSelectedText();
    }
}

QString TextShow::selectedText()
{
    QString res;
    int paraFrom, paraTo, indexFrom, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo);
    return plainText(paraFrom, paraTo, indexFrom, indexTo);
}

QString TextShow::plainText(int paraFrom, int paraTo, int indexFrom, int indexTo)
{
    QString res;
    if ((paraFrom > paraTo) || ((paraFrom == paraTo) && (indexFrom >= indexTo)))
        return res;
    for (int i = paraFrom; i <= paraTo; i++){
        if (i >= paragraphs())
            break;
        res += unquoteString(text(i), (i == paraFrom) ? indexFrom : 0, (i == paraTo) ? indexTo : -1);
        if ((i < paraTo) && (i < paragraphs()))
            res += "\n";
    }
    return res;
}

QString TextShow::unquoteString(const QString &s, int from, int to)
{
    unsigned startPos = textPosition(s, from);
    unsigned endPos = textPosition(s, to);
    return SIM::unquoteText(s.mid(startPos, endPos - startPos));
}

unsigned TextShow::textPosition(const QString &text, unsigned pos)
{
    if (pos >= text.length())
        return text.length();
    unsigned i;
    for (i = 0; i < text.length(); i++){
        QChar c = text[(int)i];
        if (c == '<'){
            QString tag = text.mid(i + 1, 3).lower();
            if (tag == "img"){
                if (pos == 0)
                    return i;
                pos--;
            }
            for (; i < text.length(); i++){
                c = text[(int)i];
                if (c == '>')
                    break;
            }
            continue;
        }
        if (pos == 0)
            return i;
        pos--;
    }
    return i;
}

QString TextShow::quoteText(const char *t, const char *charset)
{
    if (t == NULL)
        t = "";
    QString text;
    QTextCodec *codec = NULL;
    if (charset)
        codec = QTextCodec::codecForName(charset);
    if (codec){
        text = codec->makeDecoder()->toUnicode(t, strlen(t));
    }else{
        text = QString::fromLocal8Bit(t);
    }
    return quoteString(text);
}

void TextShow::setText(const QString &text)
{
    QTextBrowser::setText(text, "");
}

class BgColorParser : public HTMLParser
{
public:
    BgColorParser(TextEdit *edit);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    TextEdit *m_edit;
};

BgColorParser::BgColorParser(TextEdit *edit)
{
    m_edit = edit;
}

void BgColorParser::text(const QString&)
{
}

void BgColorParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag != "body")
        return;
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString key = *it;
        ++it;
        QString val = *it;
        if (key == "bgcolor"){
            if (val[0] == '#'){
                bool bOK;
                unsigned rgb = val.mid(1).toUInt(&bOK, 16);
                if (bOK)
                    m_edit->setBackground(QColor(rgb));
            }
        }
    }
}

void BgColorParser::tag_end(const QString&)
{
}

RichTextEdit::RichTextEdit(QWidget *parent, const char *name)
        : QMainWindow(parent, name, 0)
{
    m_edit = new TextEdit(this);
    m_bar  = NULL;
    setCentralWidget(m_edit);
}

void RichTextEdit::setText(const QString &str)
{
    if (m_edit->textFormat() != QTextEdit::RichText)
        m_edit->setText(str);
    BgColorParser p(m_edit);
    p.parse(str);
    m_edit->setText(str);
}

QString RichTextEdit::text()
{
    if (m_edit->textFormat() != QTextEdit::RichText)
        return m_edit->text();
    char bg[20];
    sprintf(bg, "%06X", m_edit->background().rgb());
    QString res;
    res = "<BODY BGCOLOR=\"#";
    res += bg;
    res += "\">";
    res += m_edit->text();
    res += "</BODY>";
    return res;
}

void RichTextEdit::setTextFormat(QTextEdit::TextFormat format)
{
    m_edit->setTextFormat(format);
}

QTextEdit::TextFormat RichTextEdit::textFormat()
{
    return m_edit->textFormat();
}

void RichTextEdit::setReadOnly(bool bState)
{
    m_edit->setReadOnly(bState);
}

static void set_button(QToolButton *btn, const char *icon, const char *label)
{
    btn->setAutoRaise(true);
    btn->setIconSet(*Icon(icon));
    QString text = i18n(label);
    int key = QAccel::shortcutKey(text);
    btn->setAccel(key);
    QString t = text;
    int pos = t.find("<br>");
    if (pos >= 0) t = t.left(pos);
    btn->setTextLabel(t);
    t = text;
    while ((pos = t.find('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    QToolTip::add(btn, t);
}

void RichTextEdit::showBar()
{
    if (m_bar)
        return;
    m_bar = new QToolBar(this);

    QToolButton *btn = new ColorToolButton(m_bar, m_edit->background());
    set_button(btn, "bgcolor", I18N_NOOP("Bac&kground color"));
    connect(btn, SIGNAL(colorChanged(QColor)), this, SLOT(bgColorChanged(QColor)));

    btn = new ColorToolButton(m_bar, m_edit->foreground());
    set_button(btn, "fgcolor", I18N_NOOP("&Text color"));
    connect(btn, SIGNAL(colorChanged(QColor)), this, SLOT(fgColorChanged(QColor)));

    m_bar->addSeparator();

    btnBold = new QToolButton(m_bar);
    set_button(btnBold, "text_bold", I18N_NOOP("&Bold"));
    btnBold->setToggleButton(true);
    connect(btnBold, SIGNAL(toggled(bool)), this, SLOT(toggleBold(bool)));

    btnItalic = new QToolButton(m_bar);
    set_button(btnItalic, "text_italic", I18N_NOOP("&Italic"));
    btnItalic->setToggleButton(true);
    connect(btnItalic, SIGNAL(toggled(bool)), this, SLOT(toggleItalic(bool)));

    btnUnderline = new QToolButton(m_bar);
    set_button(btnUnderline, "text_under", I18N_NOOP("&Underline"));
    btnUnderline->setToggleButton(true);
    connect(btnUnderline, SIGNAL(toggled(bool)), this, SLOT(toggleUnderline(bool)));

    m_bar->addSeparator();

    btn = new QToolButton(m_bar);
    set_button(btn, "text", I18N_NOOP("Text &font"));
    connect(btn, SIGNAL(clicked()), this, SLOT(selectFont()));

    connect(m_edit, SIGNAL(currentFontChanged(const QFont&)), this, SLOT(fontChanged(const QFont&)));
}

void RichTextEdit::toggleBold(bool bState)
{
    m_edit->setBold(bState);
}

void RichTextEdit::toggleItalic(bool bState)
{
    m_edit->setItalic(bState);
}

void RichTextEdit::toggleUnderline(bool bState)
{
    m_edit->setUnderline(bState);
}

void RichTextEdit::fontChanged(const QFont &f)
{
    btnBold->setOn(f.bold());
    btnItalic->setOn(f.italic());
    btnUnderline->setOn(f.underline());
}

void RichTextEdit::bgColorChanged(QColor c)
{
    m_edit->setBackground(c);
}

void RichTextEdit::fgColorChanged(QColor c)
{
    m_edit->setForeground(c);
}

void RichTextEdit::selectFont()
{
#ifdef USE_KDE
    QFont f = m_edit->font();
    if (KFontDialog::getFont(f, false, topLevelWidget()) != KFontDialog::Accepted)
        return;
#else
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, m_edit->font(), topLevelWidget());
    if (!ok)
        return;
#endif
    m_edit->setCurrentFont(f);
}

ColorToolButton::ColorToolButton(QWidget *parent, QColor color)
        : QToolButton(parent)
{
    m_color = color;
    m_popup = NULL;
    connect(this, SIGNAL(clicked()), this, SLOT(btnClicked()));
}

void ColorToolButton::btnClicked()
{
    m_popup = new ColorPopup(this, m_color);
    connect(m_popup, SIGNAL(colorChanged(QColor)), this, SLOT(selectColor(QColor)));
    connect(m_popup, SIGNAL(colorCustom()), this, SLOT(selectCustom()));
    QPoint p = CToolButton::popupPos(this, m_popup);
    m_popup->move(p);
    m_popup->show();
}

void ColorToolButton::selectColor(QColor c)
{
    m_color = c;
    emit colorChanged(c);
    QTimer::singleShot(0, this, SLOT(closePopup()));
}

void ColorToolButton::closePopup()
{
    if (m_popup){
        delete m_popup;
        m_popup = NULL;
    }
}

void ColorToolButton::selectCustom()
{
#ifdef USE_KDE
    QColor c = m_color;
    if (KColorDialog::getColor(c, this) != KColorDialog::Accepted) return;
#else
    QColor c = QColorDialog::getColor(m_color, this);
    if (!c.isValid()) return;
#endif
    m_color = c;
    emit colorChanged(c);
}

static unsigned colors[16] =
    {
        0x000000,
        0xFF0000,
        0x00FF00,
        0x0000FF,
        0xFFFF00,
        0xFF00FF,
        0x00FFFF,
        0xFFFFFF,
        0x404040,
        0x800000,
        0x008000,
        0x000080,
        0x808000,
        0x800080,
        0x008080,
        0x808080
    };

const int CUSTOM_COLOR	= 100;

ColorPopup::ColorPopup(QWidget *popup, QColor color)
        : QFrame(popup, "colors", WType_Popup | WStyle_Customize | WStyle_Tool | WDestructiveClose)
{
    setFrameShape(PopupPanel);
    setFrameShadow(Sunken);
    QGridLayout *lay = new QGridLayout(this, 5, 4);
    lay->setMargin(4);
    lay->setSpacing(2);
    for (unsigned i = 0; i < 4; i++){
        for (unsigned j = 0; j < 4; j++){
            unsigned n = i*4+j;
            QWidget *w = new ColorLabel(this, QColor(colors[n]), n, "");
            connect(w, SIGNAL(selected(int)), this, SLOT(colorSelected(int)));
            lay->addWidget(w, i, j);
        }
    }
    QWidget *w = new ColorLabel(this, color, CUSTOM_COLOR, i18n("Custom"));
    lay->addMultiCellWidget(w, 5, 5, 0, 3);
    connect(w, SIGNAL(selected(int)), this, SLOT(colorSelected(int)));
    resize(minimumSizeHint());
}

void ColorPopup::colorSelected(int id)
{
    if (id == CUSTOM_COLOR){
        emit colorCustom();
    }else{
        emit colorChanged(QColor(colors[id]));
    }
}

ColorLabel::ColorLabel(QWidget *parent, QColor c, int id, const QString &text)
        : QLabel(parent)
{
    m_id = id;
    setText(text);
    setBackgroundColor(c);
    setAlignment(AlignHCenter | AlignVCenter);
    setFrameShape(StyledPanel);
    setFrameShadow(Sunken);
    setLineWidth(2);
}

void ColorLabel::mouseReleaseEvent(QMouseEvent*)
{
    emit selected(m_id);
}

QSize ColorLabel::sizeHint() const
{
    QSize s = QLabel::sizeHint();
    if (s.width() < s.height())
        s.setWidth(s.height());
    return s;
}

QSize ColorLabel::minimumSizeHint() const
{
    QSize s = QLabel::minimumSizeHint();
    if (s.width() < s.height())
        s.setWidth(s.height());
    return s;
}

#ifndef _WINDOWS
#include "textshow.moc"
#endif
