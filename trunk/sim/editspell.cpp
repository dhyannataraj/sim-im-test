/***************************************************************************
                          editspell.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "editspell.h"
#include "transparent.h"
#include "mainwin.h"
#include "client.h"
#include "log.h"

#include "locale.h"

#include <qaccel.h>
#include <qapplication.h>
#include <qstringlist.h>

#ifdef USE_SPELL
#include <kspell.h>
#include <kspelldlg.h>
#endif

EditSpell::EditSpell(QWidget *parent) : QTextEdit(parent)
{
#ifdef USE_SPELL
    pSpell = NULL;
#endif
    bgTransparent = new TransparentBg(this);
    setWordWrap(WidgetWidth);
    baseBG = colorGroup().color(QColorGroup::Base);
    baseFG = colorGroup().color(QColorGroup::Text);
    curFG = baseFG;
    QAccel *a = new QAccel(this);
    if (!pMain->SendEnter){
        a->connectItem(a->insertItem(QAccel::stringToKey("Ctrl+Enter")),
                       this, SIGNAL(ctrlEnterPressed()));
        a->connectItem(a->insertItem(QAccel::stringToKey("Ctrl+Return")),
                       this, SIGNAL(ctrlEnterPressed()));
    }
}

QSize EditSpell::sizeHint()
{
    QSize s = QTextEdit::sizeHint();
    if (pMain->SimpleMode || !pMain->SendEnter)
        return s;
    QFontMetrics fm(font());
    s.setHeight(fm.height() + 4);
    return s;
}

QSize EditSpell::minimumSizeHint()
{
    QSize s = QTextEdit::minimumSizeHint();
    if (pMain->SimpleMode || !pMain->SendEnter)
        return QTextEdit::minimumSizeHint();
    QFontMetrics fm(font());
    s.setHeight(fm.height() + 4);
    return s;
}

void EditSpell::keyPressEvent(QKeyEvent *e)
{
    if (pMain->SendEnter && ((e->key() == Key_Enter) || (e->key() == Key_Return)))
        return;
    if (e->state() == ControlButton){
        switch (e->key()){
        case Key_Insert:
            copy();
            return;
        case Key_Delete:
            cut();
            return;
        }
    }
    if ((e->state() == ShiftButton) && (e->key() == Key_Insert)){
        paste();
        return;
    }
    QTextEdit::keyPressEvent(e);
}

void EditSpell::keyReleaseEvent(QKeyEvent *e)
{
    if (pMain->SendEnter && ((e->key() == Key_Enter) || (e->key() == Key_Return))){
        ctrlEnterPressed();
        return;
    }
    QTextEdit::keyReleaseEvent(e);
}

bool EditSpell::colorChanged()
{
    return ((palette().color(QPalette::Active, QColorGroup::Base).rgb() != baseBG.rgb()) ||
            (curFG.rgb() != baseFG.rgb()));
}

bool EditSpell::fontChanged()
{
    return bFontChanged;
}

void EditSpell::setBold(bool bBold)
{
    QTextEdit::setBold(bBold);
    bFontChanged = true;
    fCurrent.setBold(bBold);
}

void EditSpell::setItalic(bool bItalic)
{
    QTextEdit::setItalic(bItalic);
    bFontChanged = true;
    fCurrent.setItalic(bItalic);
}

void EditSpell::setUnderline(bool bUnderline)
{
    QTextEdit::setUnderline(bUnderline);
    bFontChanged = true;
    fCurrent.setUnderline(bUnderline);
}

void EditSpell::setCurrentFont(const QFont &f)
{
    QTextEdit::setCurrentFont(f);
    bFontChanged = true;
    fCurrent = f;
}

void EditSpell::resetColors(bool bCanRich)
{
    curFG = baseFG;
    if (bCanRich){
        bFontChanged = false;
        if (pMain->MessageBgColor != pMain->MessageFgColor){
            setBackground(pMain->MessageBgColor);
            setForeground(pMain->MessageFgColor);
            curFG = pMain->MessageFgColor;
        }else{
            setBackground(baseBG);
            setForeground(baseFG);
        }
        setCurrentFont(pMain->str2font(pMain->UserBoxFont.c_str(), font()));
    }else{
        setBackground(baseBG);
        setForeground(baseFG);
    }
}

void EditSpell::setBackground(const QColor& c)
{
    QPalette pal = palette();
    bool bUpdate = (colorGroup().base() != c);

    pal.setColor(QPalette::Active, QColorGroup::Base, c);
    pal.setColor(QPalette::Inactive, QColorGroup::Base, c);
    pal.setColor(QPalette::Disabled, QColorGroup::Base, c);

    setPalette(pal);
    if (bUpdate) QApplication::postEvent(viewport(), new QEvent(QEvent::User));
}

void EditSpell::setForeground(const QColor& c)
{
    curFG = c;
    setColor(c);
    QPalette pal = palette();
    pal.setColor(QPalette::Active, QColorGroup::Text, c);
    setPalette(pal);
    curFG = c;
}

const QColor &EditSpell::background() const
{
    return palette().color(QPalette::Active, QColorGroup::Base);
}

const QColor &EditSpell::foreground() const
{
    return curFG;
}

#ifdef USE_SPELL
void EditSpell::spell_check(KSpell *spell)
{
#if QT_VERSION < 300
    connect(spell, SIGNAL(misspelling(QString, QStringList*, unsigned)),
            this, SLOT(misspelling(QString, QStringList*, unsigned)));
    connect(spell, SIGNAL(corrected(QString, QString, unsigned)),
            this, SLOT(corrected_old(QString, QString, unsigned)));
#else
    connect(spell, SIGNAL(misspelling(const QString&, const QStringList&, unsigned int)),
            this, SLOT(misspelling(const QString&, const QStringList&, unsigned int)));
    connect(spell, SIGNAL(corrected(const QString&, const QString&, unsigned int)),
            this, SLOT(corrected(const QString&, const QString&, unsigned int)));
#endif
    connect(spell, SIGNAL(done(const QString&)), this, SLOT(spell_done(const QString&)));
#else
void EditSpell::spell_check(KSpell*)
{
#endif
}

void EditSpell::misspelling(QString original, QStringList *suggestion, unsigned pos)
{
    misspelling(original, *suggestion, pos);
}

#ifdef USE_SPELL

void EditSpell::misspelling (const QString& original, const QStringList&, unsigned int)
{
    if (!find(original, true, true, true, &nPara, &nIndex)){
        string oldWord;
        oldWord = original.local8Bit();
        log(L_DEBUG, "%s not found", oldWord.c_str());
        return;
    }
    setCursorPosition(nPara, nIndex);
    for (unsigned i = 0; i < original.length(); i++)
        moveCursor(MoveForward, true);
    ensureCursorVisible();
    QRect rc = paragraphRect(nPara);
    QPoint p1 = QPoint(rc.left(), rc.top());
    QPoint p2 = QPoint(rc.right(), rc.bottom());
    p1 = viewport()->mapToParent(p1);
    p2 = viewport()->mapToParent(p2);
    p1.setX(QMAX(p1.x(), 0));
    p1.setY(QMAX(p1.y(), 0));
    p2.setX(QMIN(p2.x(), width()));
    p2.setY(QMIN(p2.y(), height()));
    p1 = mapToGlobal(p1);
    p2 = mapToGlobal(p2);

    int yPos = p1.y();
    if (p1.y() >= pSpell->heightDlg() - 10){
        yPos -= pSpell->heightDlg() + 25;
    }else{
        yPos = p2.y() + 10;
    }
    p1.setY(yPos);
    p1 = mapFromGlobal(p1);
    pSpell->moveDlg(p1.x(), p1.y());
#else
void EditSpell::misspelling (const QString&, const QStringList&, unsigned int)
{
#endif
}

void EditSpell::corrected_old(QString original, QString newword, unsigned pos)
{
    corrected((const QString&)original, (const QString&)newword, (unsigned int)pos);
}

#ifdef USE_SPELL
void EditSpell::corrected(const QString & original, const QString & newword, unsigned int)
{
    if (!find(original, true, true, true, &nPara, &nIndex)){
        string oldWord;
        string newWord;
        oldWord = original.local8Bit();
        newWord = newword.local8Bit();
        log(L_WARN, "%s -> %s not found", oldWord.c_str(), newWord.c_str());
        return;
    }
    setCursorPosition(nPara, nIndex + original.length());
    setSelection(nPara, nIndex, nPara, nIndex + original.length());
    insert(newword, FALSE, TRUE, TRUE);
#else
void EditSpell::corrected(const QString&, const QString&, unsigned int)
{
#endif
}

void EditSpell::spell_done(const QString&)
{
#ifdef USE_SPELL
    bool res = false;
    if (pSpell){
        if (pSpell->dlgResult() != KS_STOP) res = true;
        delete pSpell;
        pSpell = NULL;
    }
    emit spellDone(res);
#endif
}

void EditSpell::spell()
{
#ifdef USE_SPELL
    nPara  = 0;
    nIndex = 0;
    if (pSpell == NULL)
        pSpell = new KSpell(this, i18n("Spell check"), this, SLOT(spell_check(KSpell *)), NULL, true, true);
    string t;
    if (text().isEmpty()){
        emit spellDone(true);
        return;
    }
    t = text().utf8();
    t = pClient->clearHTML(t.c_str());
    pSpell->check(QString::fromUtf8(t.c_str()));
#else
    emit spellDone(true);
#endif
}

static void parseStyle(QString &tag, QString &head, QString &end)
{
    int pos = tag.find("style=\"");
    if (pos >= 0) tag = tag.mid(pos+7);
    pos = tag.find("\"");
    if (pos >= 0) tag = tag.left(pos);
    QStringList styles = QStringList::split(';', tag);
    for (QStringList::Iterator it = styles.begin(); it != styles.end(); ++it){
        pos = (*it).find(':');
        if (pos < 0) continue;
        QString name = (*it).left(pos);
        QString value = (*it).mid(pos+1);
        if (name == "font-weight"){
            int weight = value.toInt();
            if (weight > 400){
                head += "<b>";
                end = QString("</b>") + end;
            }
        }
        if ((name == "font-style") && (value == "italic")){
            head += "<i>";
            end = QString("</i>") + end;
        }
        if ((name == "text-decoration") && (value == "underline")){
            head += "<u>";
            end = QString("</u>") + end;
        }
        if (name == "color"){
            head += "<font color=\"";
            head += value;
            head += "\">";
            end = QString("</font>") + end;
        }
        if (name == "font-family"){
            head += "<font face=\"";
            head += value;
            head += "\">";
            end = QString("</font>") + end;
        }
    }
}

QString EditSpell::text()
{
    QString res = QTextEdit::text();
    QString tag;
    QString endText;
    int pos = res.find("<body");
    if (pos >= 0){
        res = res.mid(pos + 5);
        pos = res.find(">");
        if (pos >= 0){
            tag = res.left(pos);
            QString head;
            parseStyle(tag, head, endText);
            res = head + res.mid(pos+1);
        }
    }
    pos = res.find("</body");
    if (pos >= 0)
        res = res.left(pos);
    while ((pos = res.find("<span")) >= 0){
        QString head = res.left(pos);
        res = res.mid(pos);
        pos = res.find(">");
        if (pos >= 0){
            tag = res.left(pos);
            res = res.mid(pos+1);
        }
        QString end;
        parseStyle(tag, head, end);
        pos = res.find("</span>");
        if (pos < 0){
            res = head + res + end;
        }else{
            res = head + res.left(pos) + end + res.mid(pos + 7);
        }
    }
    return res + endText;
}

#ifndef _WINDOWS
#include "editspell.moc"
#endif

